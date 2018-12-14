#ifndef __INCLUDE_PRDCR_HPP__
#define __INCLUDE_PRDCR_HPP__

#include <memory>
#include <algorithm>
#include <chrono>
#include <sstream>

#include <librdkafka/rdkafkacpp.h>
#include <libserdes/serdescpp.h>
#include <libserdes/serdescpp-avro.h>

#include "arrow/array.h"
#include "arrow/record_batch.h"
#include "arrow/status.h"
#include "arrow/table.h"
#include "arrow/table_builder.h"
#include "arrow/array.h"
#include "arrow/buffer.h"
#include "arrow/builder.h"
#include "arrow/memory_pool.h"


#include "err.hpp"
#include "kfk_bse.hpp"
#include "kfk_hlpr.hpp"
#include "avr_hlpr.hpp"

namespace kafarr {
  /**
   *
   */
  class prdcr : protected kfk_bse {
  private :
    const int RD_KFK_POLL_MS = 5;
    const std::unique_ptr<RdKafka::KafkaConsumer> _cnsmr;
    const std::unique_ptr<RdKafka::KafkaConsumer> _prdcr;
    bool first_call = true;
    
  public:
    prdcr() = delete;
    prdcr(const std::string& srvr_lst,
	  const std::string& grp,
	  const std::vector<std::string>& tpcs,
	  const std::string& reg_url) : kfk_bse(reg_url),_cnsmr(kfk_hlpr::mk_kfk_cnsmr(grp, srvr_lst))
    {      
      auto err = _cnsmr->subscribe(tpcs);       
      
      if (err != RdKafka::ErrorCode::ERR_NO_ERROR){
	std::string msg = "failed to subscribe to topics: ";
	std::for_each(tpcs.begin(), tpcs.end(), [&](auto tpc){msg.append(tpc).append(", ");});	
      	msg.append(RdKafka::err2str(err));
      	throw kafarr::err(msg);
      }
    }
    
  public:
    /**
     * destructor
     */
    ~prdcr(){
      _cnsmr->close();
    }

  public:
    void send(const std::string& msg_typ, std::shared_ptr<arrow::Table> tbl) {
      std::cerr << "Writing message type : " << msg_typ << std::endl;
      std::cerr << "data table # columns : " << tbl->num_columns() << std::endl;
      
      for (auto i = 0; i < tbl->num_columns(); ++i) {
	auto col = tbl->column(i);
	std::cerr << "col #" << i << " :: name: " << col->name() << " :: length: " << col->length() << std::endl;	
      }
      
      // get the serdes schema for the message type
      std::string err;
      ///auto schm = Serdes::Schema::get(hndl, msg_typ, err);
      Serdes::Schema* schm = Serdes::Schema::get(_srds_hndl.get(), msg_typ, err);
      if (!schm) {
	std::stringstream ss ;
	ss << "ERROR retrieving the schema for message type: " << msg_typ << ". error: " << err << std::endl;
	ss << "Make sure it exists or supply it directly";
	std::cerr << ss.str() << std::endl;	  
	
	throw kafarr::err(ss.str());
      }
      
      //avro::ValidSchema* avro_schm = schm->object();
      auto avr_schm = schm->object();
      if (!avr_schm) throw kafarr::err("ERROR. Invalid avro schema object");
      
      auto root = avr_schm->root();
      
      if(root->name().fullname() != msg_typ){
	std::stringstream ss ;
	ss << "ERROR. type name mismatch.\n";
	ss << "  requested type: " << msg_typ << std::endl;
	ss << "  retrieved type: " << root->name().fullname();
	std::cerr << ss.str() << std::endl;
	
	throw kafarr::err(ss.str());
      }
      
      /**
       * get the schema from the table and check that it contains all the fields required 
       */
      auto tbl_schm = tbl->schema();
      for(int i = 0; i < root->leaves(); ++i){
	//root->nameAt(i));
	auto lf = root->leafAt(i);
	if (!lf) throw kafarr::err("BUG. leaf node is null", root->nameAt(i));
	std::cerr << " AVRO schema field @" << i << " :: " << root->nameAt(i) << " : " << lf->type() << std::endl;
	
	if(tbl_schm->GetFieldByName(root->nameAt(i)))
	  std::cerr << " table contains schema field: " << root->nameAt(i) << std::endl;
	else{
	  std::cerr << " ERROR. schema field is missing: " << root->nameAt(i) << std::endl;
	  throw kafarr::err("ERROR. schema field is missing: ", root->nameAt(i));
	}
	
	
	if (false){
	  avro::GenericDatum* dtm = new avro::GenericDatum(*avr_schm);	  
	  if (dtm->type() != avro::Type::AVRO_RECORD) {
	    std::stringstream msg("BUG!. Top-level Avro datum needst to be record type but is ");
	    msg << dtm->type();
	    std::cerr << msg.str() << std::endl;
	    throw kafarr::err(msg.str());
	  }
	  
	  // extract the record value from the row datum
	  auto rcrd = dtm->value<avro::GenericRecord>();
	  // extract and compare number of avro and arrow fields. they should match
	  auto n_avr_flds = rcrd.fieldCount();
	  for (auto i = 0; i < n_avr_flds; ++i) {
	    auto fld = rcrd.fieldAt(i);
	    std::cerr << "XX " << fld.type() << std::endl;

	    switch (fld.type()) {
	    case avro::Type::AVRO_STRING:
	      break;
	    case avro::Type::AVRO_INT: {
	      int& v = fld.value<int>();
	      v = -11;
	    }
	      break;
	    case avro::Type::AVRO_LONG:
	      break;
	    case avro::Type::AVRO_FLOAT: 
	      break;
	    case avro::Type::AVRO_DOUBLE:
	      break;
	    default:{
	      std::stringstream msg("BUG!. Avro type not handled:");
	      msg << fld.type();
	      throw kafarr::err(msg.str());
	    }
	    }
	  }
	}
      }
	
      if(true){
	std::string jsn = "{\"inst\": \"abcd\", \"t\":12345, \"dt\":3, \"bid\":1.2345, \"ask\":1.2345}";
	avro::GenericDatum *datum = NULL;
	std::vector<char> out;

	avr_hlpr::json2avro(schm, jsn, &datum);

	/* Serialize Avro */
	if(_srds_avro->serialize(schm, datum, out, err) == -1)
	  throw kafarr::err("failed to serialise message: ", err);
	
	//if (serdes->serialize(schema, datum, out, errstr) == -1) {
	delete datum;


	/* Create Kafka producer */
	RdKafka::Conf *kconf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
	if (kconf->set("bootstrap.servers", "kfk:9092", err) != RdKafka::Conf::ConfResult::CONF_OK)
	  throw kafarr::err("failed to set broker list property . ", err);

	RdKafka::Producer *producer = RdKafka::Producer::create(kconf, err);
	if (!producer){
	  std::cerr << "failed to create kafka producer: " << err << std::endl;
	  throw kafarr::err("failed to create kafka producer: ", err);
	}
	delete kconf;

	/* Create topic object */
	//RdKafka::Topic *ktopic = RdKafka::Topic::create(producer, topic, tconf, errstr);
	RdKafka::Topic *ktopic = RdKafka::Topic::create(producer, "test_topic_1", NULL, err);
	if (!ktopic){
	  std::cerr << "failed to create kafka topic: " << err << std::endl;
	  throw kafarr::err("failed to create kafka topic: ", err);
	}

	for(auto i = 0; i < 10; ++i){
	  RdKafka::ErrorCode kerr = producer->produce(ktopic, -1, &out, NULL, NULL);	  
	  if (kerr != RdKafka::ERR_NO_ERROR) 
	    std::cerr << "% Failed to produce message: " << RdKafka::err2str(kerr) << std::endl;
	  
	}
      }
	
    }
  };  
}

#endif //  __INCLUDE_PRDCR_HPP__
