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
      
    }
  };
}

#endif //  __INCLUDE_ARROW_TABLE_DECODER_HPP__
