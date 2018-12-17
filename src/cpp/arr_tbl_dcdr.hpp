#ifndef __INCLUDE_ARROW_TABLE_DECODER_HPP__
#define __INCLUDE_ARROW_TABLE_DECODER_HPP__

#include <memory>
#include <libserdes/serdescpp.h>
#include <libserdes/serdescpp-avro.h>
#include "arrow/table.h"


namespace kafarr {

  /**
   * an arrow table decoder
   */
  class arr_tbl_dcdr {
  public:
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
       */
      for(int i = 0; i < avr_root_nd->leaves(); ++i){
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
      }
      
      
    }
  };
}

#endif //  __INCLUDE_ARROW_TABLE_DECODER_HPP__
