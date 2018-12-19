#ifndef __INCLUDE_ARROW_TABLE_DECODER_HPP__
#define __INCLUDE_ARROW_TABLE_DECODER_HPP__

#include <memory>
#include <libserdes/serdescpp.h>
#include <libserdes/serdescpp-avro.h>
#include "arrow/table.h"

#include <avro/Generic.hh>

#include "err.hpp"

namespace kafarr {

  /**
   * an arrow table decoder.
   * equivalent to avro::jsonDecoder except does not inherit from avro::Decoder
   */
  class arr_tbl_dcdr {
  public:
    /**
     * A Shallow descent (only first level)
     */
    static std::vector<std::shared_ptr<avro::GenericDatum> > arr2avr(const std::shared_ptr<arrow::Table> tbl, const std::shared_ptr<Serdes::Schema> schm){
      /**
       * get the root node object
       */
      const avro::NodePtr avr_root_nd = schm->object()->root();

      /**
       * extract the arrow schema from the table. 
       * this is the schema of the data we want to send
       */
      std::shared_ptr<arrow::Schema> arr_schm = tbl->schema();

      /**
       * make sure that the arrow table schema can satisfy the requirements of the avro schema
       * 1. the table arrow schema must contain all the fields in the message avro schema
       * 2. we must check that the data can be type coerced into a target typ
       * ??3. grab data columns and store in order of fields in the schema for quicker access later. 
       *       ?? std::vector<const std::shared_ptr<arrow::Field> > tbl_flds;
       *        tbl_flds.push_back(arr_fld);
       */
      for(int i = 0; i < avr_root_nd->leaves(); ++i){
	//root->nameAt(i));
        const avro::NodePtr lf = avr_root_nd->leafAt(i);
        if (!lf) throw kafarr::err("ERROR/BUG?. leaf node is unexpectedly null:", avr_root_nd->nameAt(i));
        //std::cerr << " AVRO schema field @" << i << " :: " << avr_root_nd->nameAt(i) << " : " << lf->type() << std::endl;
	
        const std::string avro_fld_nme = avr_root_nd->nameAt(i);
        const std::shared_ptr<arrow::Field> arr_fld = arr_schm->GetFieldByName(avro_fld_nme);

        /**
         * check that the avro schema-required field exists in the table
         */
        if(!arr_fld){
          std::cerr << "ERROR. Field required byt the Avro schema is missing from the data table: " << avro_fld_nme << std::endl;
          throw kafarr::err("ERROR. Field required byt the Avro schema is missing from the data table: ", avr_root_nd->nameAt(i));
        }
      }
      
      /**
       * while at it!, check that the field data is not chunkedd. we cant handle cunked arrays atm
       */
      for(auto i = 0; i < tbl->num_columns(); ++i) {
	const std::shared_ptr<arrow::Column> col = tbl->column(i);
	if(col->data()->num_chunks() != 1){
	  std::cerr << "ERROR. Column " << col->name() << " is composed of more or less than 1 chunk: " << col->data()->num_chunks() << std::endl;
	  throw kafarr::err("ERROR. Column " + col->name() + " is composed of more or less than 1 chunk: " + std::to_string(col->data()->num_chunks()));
	}
      }
      
      /**
       * convert each row in the table (shallow for now)
       */
      std::vector<std::shared_ptr<avro::GenericDatum> > avro_data;
      for (auto i = 0; i < tbl->num_rows(); ++i){
	std::shared_ptr<avro::GenericDatum> dtm(new avro::GenericDatum(avr_root_nd));
	avro::GenericRecord& rcrd = dtm->value<avro::GenericRecord>();
	
	for(auto j = 0; j < rcrd.fieldCount(); ++j) 
	  bld_leaf_val(avr_root_nd->leafAt(j)->type(), tbl->column(j), rcrd.fieldAt(j), i);
	
	avro_data.push_back(dtm);
      }
      return avro_data;
    }
    
    /**
     * check if the source schema type (Arrow) can be coerced into the target schema type (Avro)
     * eg if target is int32, then source int8, int16 and int 32 are ok but int64 is not
     */
    //static void is_coercable(const avro::Type avr_typ, const std::shared_ptr<arrow::Column> col, const avro::GenericDatum& dtm) {
    //}

    /**
     * build a leaf value in the target AVRO datum
     */ 
    static void bld_leaf_val(const avro::Type avr_typ, const std::shared_ptr<arrow::Column> col, avro::GenericDatum& col_dtm, const int row_idx) {
      /**
       * compare each AVRO target type with the source ARROW type
       * determine if the source type can be coerced to the target type
       * TODO:>> currentlly typing is strict - only perfectly matching types are accepted all others rejected. 
       *         in the future it would be nice to coerce types where possible (say ARROW int8 to AVRO int32 etc...)
       * NOTE:>> this is the deepest loop. explicit cast for performance
       */
      switch (avr_typ) {
      case avro::Type::AVRO_INT    :
	switch(col->type()->id()){
	  //case arrow::Type::type::INT64 :
	  //col_dtm.value<int32_t>() = ((arrow::NumericArray<arrow::Int64Type>*)(col->data()->chunk(0).get()))->Value(row_idx);
	  //break;
	case arrow::Type::type::INT32 :
	  col_dtm.value<int32_t>() = ((arrow::NumericArray<arrow::Int32Type>*)(col->data()->chunk(0).get()))->Value(row_idx);
	  break;
	case arrow::Type::type::INT16 :
	  col_dtm.value<int32_t>() = ((arrow::NumericArray<arrow::Int16Type>*)(col->data()->chunk(0).get()))->Value(row_idx);
	  break;
	case arrow::Type::type::INT8 :
	  col_dtm.value<int32_t>() = ((arrow::NumericArray<arrow::Int8Type>*)(col->data()->chunk(0).get()))->Value(row_idx);
	  break;
	default:
	  throw kafarr::err("ERROR. inompatible types. Cannot convert field " + col->name() + " to AVRO '" + avro::toString(avr_typ) + "' from ARROW '" +  col->type()->name() + "'");
	}
	break;
      case avro::Type::AVRO_LONG   :
	switch(col->type()->id()){
	case arrow::Type::type::INT64 : 
	  col_dtm.value<int64_t>() = ((arrow::NumericArray<arrow::Int64Type>*)(col->data()->chunk(0).get()))->Value(row_idx);
	  break;
	default:
	  throw kafarr::err("ERROR. inompatible types. Cannot convert to AVRO '" + avro::toString(avr_typ) + "' from ARROW '" +  col->type()->name() + "'");
	}
	break;
      case avro::Type::AVRO_FLOAT  :
	switch(col->type()->id()){
	case arrow::Type::type::FLOAT : 
	  col_dtm.value<float>() = ((arrow::NumericArray<arrow::FloatType>*)(col->data()->chunk(0).get()))->Value(row_idx);
	  break;
	default:
	  throw kafarr::err("ERROR. inompatible types. Cannot convert to AVRO '" + avro::toString(avr_typ) + "' from ARROW '" +  col->type()->name() + "'");
	}
	break;
      case avro::Type::AVRO_DOUBLE :
	switch(col->type()->id()){
	case arrow::Type::type::DOUBLE : 
	  col_dtm.value<double>() = ((arrow::NumericArray<arrow::DoubleType>*)(col->data()->chunk(0).get()))->Value(row_idx);
	  break;
	default:
	  throw kafarr::err("ERROR. inompatible types. Cannot convert to AVRO '" + avro::toString(avr_typ) + "' from ARROW '" +  col->type()->name() + "'");
	}
	break;
      case avro::Type::AVRO_STRING :
	switch(col->type()->id()){
	case arrow::Type::type::STRING : 
	  col_dtm.value<std::string>() = ((arrow::StringArray*)(col->data()->chunk(0).get()))->GetString(row_idx);
	  break;
	case arrow::Type::type::BINARY : 
	  col_dtm.value<std::string>() = ((arrow::BinaryArray*)(col->data()->chunk(0).get()))->GetString(row_idx);
	  break;
	default:
	  throw kafarr::err("ERROR. inompatible types. Cannot convert to AVRO '" + avro::toString(avr_typ) + "' from ARROW '" +  col->type()->name() + "'");
	}
	break;
      case avro::Type::AVRO_BOOL   :
	switch(col->type()->id()){
	case arrow::Type::type::BOOL : 
	  col_dtm.value<bool>() = ((arrow::BooleanArray*)(col->data()->chunk(0).get()))->Value(row_idx);
	  break;
	default:
	  throw kafarr::err("ERROR. inompatible types. Cannot convert to AVRO '" + avro::toString(avr_typ) + "' from ARROW '" +  col->type()->name() + "'");
	}
	break;
      case avro::Type::AVRO_NULL   : 
      default :
	throw kafarr::err("ERROR. Leaf column '" + col->name() + "' cannot be handled. Column type: " + avro::toString(avr_typ));
      }
    }

    /**
     * check if the arrow type can be coerced to the avro type
     * static bool is_coercable(const avro::Type avr_typ, const std::shared_ptr<arrow::DataType> arr_typ) {return false;}
     */
  };
}

#endif //  __INCLUDE_ARROW_TABLE_DECODER_HPP__
