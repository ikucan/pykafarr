#ifndef __INCLUDE_AVR_HLPR_HPP__
#define __INCLUDE_AVR_HLPR_HPP__

#include <iostream>
#include <sstream>
#include <algorithm>

#include <avro/Decoder.hh>
#include <avro/Generic.hh>
#include <avro/Specific.hh>
#include <avro/Exception.hh>
#include <avro/NodeImpl.hh>

#include <arrow/type.h>

namespace kafarr {
  
  class avr_hlpr{

  public:
    /**
     * convert the data from avro format to json
     */
    static int avro2jsn(Serdes::Schema *schm, const avro::GenericDatum *dtm, std::string &str, std::string &err) {
      const avro::ValidSchema* avr_schm = schm->object();
      const avro::EncoderPtr encdr = avro::jsonEncoder(*avr_schm);
      // json output stream
      std::ostringstream oss;
      const std::unique_ptr<avro::OutputStream> jsn_os = avro::ostreamOutputStream(oss);
      try{
	encdr->init(*jsn_os.get());
	avro::encode(*encdr, *dtm);
	encdr->flush();
	std::cerr << "AVRO 2 JSON transform done. data: " << oss.str() << std::endl;    
      } catch(const avro::Exception &e) {
	std::cerr << "AVRO 2 JSON transform failed. error: " << e.what() << std::endl;
      }
    }


    /**
     * make an arrow schema from an avro schema
     */
    static std::tuple<std::string, std::shared_ptr<arrow::Schema>> mk_arrw_schm(Serdes::Schema* schm) {
      const avro::ValidSchema* avr_schm = schm->object();
      return avr2arr(avr_schm->root());      
    }

  private:
    static std::tuple<std::string, std::shared_ptr<arrow::Schema>> avr2arr(const avro::NodePtr& root){

      if(root->type() != avro::AVRO_RECORD)
	throw kafarr::err("ERROR. Expecting root node to be record type but is ", toString(root->type()));

      if (root->name().fullname().length() < 1)
	throw kafarr::err("ERROR. Expecting root node to have a non-empty name.");      
      
      auto arrw_flds = std::vector<std::shared_ptr<arrow::Field>>();

      for(int i = 0; i < root->leaves(); ++i){
	auto lf = root->leafAt(i);
	if (!lf) throw kafarr::err("leaf node is null", root->nameAt(i));
	//std::cerr << "  field @" << i << " :: " << root->nameAt(i) << " : " << lf->type() << std::endl;
	arrw_flds.push_back(arrow::field(root->nameAt(i), arrw_typ(lf)));
      }
      // insert a message offset field
      // TODO:>>this is someahat hard-coded
            arrw_flds.push_back(arrow::field("offst", arrow::int64()));
      return {root->name().fullname(), std::make_shared<arrow::Schema>(arrw_flds, nullptr)};
    }

    /**
     * map an instance of 
     */
    static std::shared_ptr<arrow::DataType> arrw_typ(const boost::shared_ptr<avro::Node>& nd){
      switch(nd->type()) {
      case avro::Type::AVRO_STRING:
	return arrow::utf8();
      case avro::Type::AVRO_BYTES:
	throw kafarr::err("BUG!! unhandled avro type:", toString(nd->type()));
      case avro::Type::AVRO_INT:
	return arrow::int32();
      case avro::Type::AVRO_LONG:
	return arrow::int64();
      case avro::Type::AVRO_FLOAT:
	return arrow::float32();
      case avro::Type::AVRO_DOUBLE:
	return arrow::float64();
      case avro::Type::AVRO_BOOL:
	return arrow::float32();
      case avro::Type::AVRO_NULL:
	throw kafarr::err("BUG!! unhandled avro type:", toString(nd->type()));
      case avro::Type::AVRO_RECORD:
	throw kafarr::err("BUG!! unhandled avro type:", toString(nd->type()));
      case avro::Type::AVRO_ENUM:
	throw kafarr::err("BUG!! unhandled avro type:", toString(nd->type()));
      case avro::Type::AVRO_ARRAY:
	throw kafarr::err("BUG!! unhandled avro type:", toString(nd->type()));
      case avro::Type::AVRO_MAP:
	throw kafarr::err("BUG!! unhandled avro type:", toString(nd->type()));
      case avro::Type::AVRO_UNION:
	throw kafarr::err("BUG!! unhandled avro type:", toString(nd->type()));
      case avro::Type::AVRO_FIXED:
	{
	  // check if the fixed node is valid
	  const avro::NodeFixed* nf = static_cast<avro::NodeFixed*>(nd.get());
	  if (!nf->isValid()) throw kafarr::err("BUG!! fixed node not valid.");
	}
	return arrow::fixed_size_binary(nd->fixedSize());
      case avro::Type::AVRO_SYMBOLIC:
	throw kafarr::err("BUG!! unhandled avro type:", toString(nd->type()));
	// NUM_TYPES is a synonym for SYMBOLIC
      case avro::Type::AVRO_UNKNOWN:
	throw kafarr::err("BUG!! unhandled avro type:", toString(nd->type()));
      default:
	throw kafarr::err("BUG!! unhandled avro type:", toString(nd->type()));
      }
    }
    
  public:    
    /**
     * add an avro datum (a deserialised kafka message) to the arrow builder
     */
    static void rd_dta(const long offset, avro::GenericDatum* row, std::unique_ptr<arrow::RecordBatchBuilder>& bldr) {
      if (row->type() != avro::Type::AVRO_RECORD) {
	std::stringstream msg("BUG!. Top-level Avro datum needst to be record type but is ");
	msg << row->type();
	throw kafarr::err(msg.str());
      }
      // extract the record value from the row datum
      auto rcrd = row->value<avro::GenericRecord>();
      // extract and compare number of avro and arrow fields. they should match
      auto n_avr_flds = rcrd.fieldCount();
      auto n_arr_flds = bldr->num_fields();
      if((n_avr_flds + 1) != n_arr_flds){ // additional message offset in the arrow schema
	std::stringstream msg;
	msg << "BUG!. Avro and Arrow record field counts do not match. Avro record has" << n_avr_flds << " fields and the Arrow RecordBatchBuilder has " << n_arr_flds << ".";
	throw kafarr::err(msg.str());
      }

      // copy over row data
      for(auto i = 0; i < n_avr_flds; ++i)
	rd_fld(rcrd.fieldAt(i), bldr->GetField(i));

      // and message offset
      static_cast<arrow::Int64Builder *>(bldr->GetField(n_avr_flds))->Append(offset);
    }

    /**
     * add an avro datum (a deserialised kafka message) to the arrow builder
     */
    static void rd_dta(const long offset, const std::shared_ptr<avro::GenericDatum>& row, std::unique_ptr<arrow::RecordBatchBuilder>& bldr) {
      if (row->type() != avro::Type::AVRO_RECORD) {
	std::stringstream msg("BUG!. Top-level Avro datum needst to be record type but is ");
	msg << row->type();
	throw kafarr::err(msg.str());
      }
      // extract the record value from the row datum
      auto rcrd = row->value<avro::GenericRecord>();
      // extract and compare number of avro and arrow fields. they should match
      auto n_avr_flds = rcrd.fieldCount();
      auto n_arr_flds = bldr->num_fields();
      if((n_avr_flds + 1) != n_arr_flds){ // additional message offset in the arrow schema
	std::stringstream msg;
	msg << "BUG!. Avro and Arrow record field counts do not match. Avro record has" << n_avr_flds << " fields and the Arrow RecordBatchBuilder has " << n_arr_flds << ".";
	throw kafarr::err(msg.str());
      }

      // copy over row data
      for(auto i = 0; i < n_avr_flds; ++i)
	rd_fld(rcrd.fieldAt(i), bldr->GetField(i));

      // and message offset
      static_cast<arrow::Int64Builder *>(bldr->GetField(n_avr_flds))->Append(offset);
    }

  private:
    /**
     * forward to the right decoder for given type
     */
    static void rd_fld(const avro::GenericDatum& dtm, arrow::ArrayBuilder* bldr) {
      switch (dtm.type()) {
      case avro::Type::AVRO_STRING:
	static_cast<arrow::StringBuilder *>(bldr)->Append(dtm.value<std::string>());
	break;
      case avro::Type::AVRO_INT:
	static_cast<arrow::Int32Builder *>(bldr)->Append(dtm.value<int>());
	break;
      case avro::Type::AVRO_LONG:
	static_cast<arrow::Int64Builder *>(bldr)->Append(dtm.value<long>());
	break;
      case avro::Type::AVRO_FLOAT: 
	static_cast<arrow::FloatBuilder *>(bldr)->Append(dtm.value<float>());
	break;
      case avro::Type::AVRO_DOUBLE:
	static_cast<arrow::DoubleBuilder *>(bldr)->Append(dtm.value<double>());
	break;
      default:{
	std::stringstream msg("BUG!. Avro type not handled:");
	msg << dtm.type();
	throw kafarr::err(msg.str());
      }
      }
    }
    /**
     *
     */
  };
}

#endif
