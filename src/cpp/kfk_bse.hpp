#ifndef __INCLUDE_KFK_BSE_HPP__
#define __INCLUDE_KFK_BSE_HPP__

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
#include "kfk_hlpr.hpp"
#include "avr_hlpr.hpp"

namespace kafarr {
  /**
   *
   */  
  class kfk_bse {
  protected:
    std::shared_ptr<Serdes::Conf>   _srds_conf;
    std::shared_ptr<Serdes::Handle> _srds_hndl;
    std::unique_ptr<Serdes::Avro>   _srds_avro;
    
  public:
    kfk_bse() = delete;

    kfk_bse(const std::string& reg_url) :
      _srds_conf(kfk_hlpr::mk_srds_conf(reg_url)),
      _srds_hndl(kfk_hlpr::mk_srds_hndl(_srds_conf)),
      _srds_avro(kfk_hlpr::mk_avro(_srds_conf))
    {}    
    
    virtual ~kfk_bse(){}
    
  protected:
    auto now_ms() {
      return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    }
    
  };
}

#endif //  __INCLUDE_KFK_BSE_HPP__
