#ifndef __INCLUDE_ARROW_TABLE_DECODER_HPP__
#define __INCLUDE_ARROW_TABLE_DECODER_HPP__

#include <memory>
#include <libserdes/serdescpp.h>
#include <libserdes/serdescpp-avro.h>
#include "arrow/table.h"

//#include <avro/Decoder.hh>
#include <avro/Generic.hh>
//#include <avro/Specific.hh>
//#include <avro/Exception.hh>
//#include <avro/NodeImpl.hh>


#include "err.hpp"

namespace kafarr {

  /**
   * an arrow table decoder
   */
  class arr_tbl_dcdr {
  public:
    /**
     * A Shallow descent (only first level)
     */
    static void arr2avr(const std::shared_ptr<arrow::Table> tbl, const std::shared_ptr<Serdes::Schema> schm){
      /**
       * get the avro schema from the Serdes schema wrapper
       * this is the schema sent messages need to conform to
       */
      const std::shared_ptr<avro::ValidSchema> avr_schm(schm->object());
      const avro::NodePtr avr_root_nd = avr_schm->root();

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
        std::cerr << " AVRO schema field @" << i << " :: " << avr_root_nd->nameAt(i) << " : " << lf->type() << std::endl;
	
        const std::string avro_fld_nme = avr_root_nd->nameAt(i);
        const std::shared_ptr<arrow::Field> arr_fld = arr_schm->GetFieldByName(avro_fld_nme);

        /**
         * check that the avro schema-required field exists in the table
         */
        if(!arr_fld){
          std::cerr << "ERROR. Field required byt the Avro schema is missing from the data table: " << avro_fld_nme << std::endl;
          throw kafarr::err("ERROR. Field required byt the Avro schema is missing from the data table: ", avr_root_nd->nameAt(i));
        }

        std::cerr << "AVRO  field name: " << avro_fld_nme << ", type: " << lf->type() << std::endl;
        std::cerr << "ARROW field name: " << arr_fld->name() << ", type: " << arr_fld->type()->name() << std::endl;
      }

      //convert each row in the table
      for (auto i = 0; i < tbl->num_rows(); ++i){
      //for (auto i = 0; i < 2; ++i){
	std::cerr << "row #: " << i << std::endl;
	std::shared_ptr<avro::GenericDatum> dtm(new avro::GenericDatum(avr_root_nd));
	avro::GenericRecord& rcrd = dtm->value<avro::GenericRecord>();

	for(auto j = 0; j < rcrd.fieldCount(); ++j) {
	  const std::string avro_fld_nme = avr_root_nd->nameAt(j);
	  const avro::NodePtr lf         = avr_root_nd->leafAt(j);
	  const std::shared_ptr<arrow::Field> arr_fld = arr_schm->GetFieldByName(avro_fld_nme);
	  const std::shared_ptr<arrow::Column> col    = tbl->column(j);

	  std::cerr << " AVRO schema field @" << j << " :: " << avr_root_nd->nameAt(j) << " : " << lf->type() << std::endl;
	  std::cerr << " ARROW column. name: " << col->name() << ". type: " << col->type()->name() << ". length:  " << col->length() << std::endl;
	  std::cerr << "     #chunks :  " << col->data()->num_chunks() << std::endl;

	  if (j == 1){
	    //auto l_arr = (arrow::NumericArray<arrow::Int64Type>*)(col->data()->chunk(0).get());
	    //std::cerr << "value[" << i << ", 1] = " << l_arr->Value(i) << std::endl;


	    auto l_arr = std::dynamic_pointer_cast<arrow::NumericArray<arrow::Int64Type>  >(col->data()->chunk(0));
	    std::cerr << "value[" << i << ", 1] = " << l_arr->Value(i) << std::endl;

	    //auto l_arr = (arrow::NumericArray<long>*)(col->data()->chunk(0).get());
	  }
	  //const avro::GenericDatum& di = rcrd.fieldAt(0);

	}
	//avro::GenericDatum* dtm = new avro::GenericDatum(avr_root_nd);
	
	
      }
      
    }
    
    static void coerce(const avro::Type avr_typ, const std::shared_ptr<arrow::DataType> arr_typ, const avro::GenericDatum& di) {
    }

    /**
     * check if the arrow type can be coerced to the avro type
     * static bool is_coercable(const avro::Type avr_typ, const std::shared_ptr<arrow::DataType> arr_typ) {return false;}
     */
  };
}

#endif //  __INCLUDE_ARROW_TABLE_DECODER_HPP__
