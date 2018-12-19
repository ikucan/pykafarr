#ifndef __INCLUDE_PRDCR_HPP__
#define __INCLUDE_PRDCR_HPP__

#include "arr_tbl_dcdr.hpp"

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
    const std::unique_ptr<RdKafka::Producer> _prdcr;
    bool first_call = true;
    
  public:
    prdcr() = delete;
    prdcr(const std::string& srvr_lst,
	  const std::string& reg_url) : kfk_bse(reg_url), _prdcr(kfk_hlpr::mk_kfk_prdcr(srvr_lst))
    {
      std::cerr << "prdcr::prdcr(....)" << std::endl;
    }
    
  public:
    /**
     * destructor
     */
    ~prdcr(){}

  public:
    void send(const std::string& msg_typ, std::shared_ptr<arrow::Table> tbl, const std::string topic, const int partition = RdKafka::Topic::PARTITION_UA) {
      try{
	std::cerr << "send(...) - START" << std::endl;
	auto t0 = now_ms();
	// general purpose error message string used in many calls below
	std::string err;
	
	/**
	 * get the schema for the message first
	 */
	std::shared_ptr<Serdes::Schema> schm = std::shared_ptr<Serdes::Schema>(Serdes::Schema::get(_srds_hndl.get(), msg_typ, err));
	if (!schm) {
	  std::stringstream ss ;
	  ss << "ERROR retrieving the schema for message type: " << msg_typ << ". error: " << err << std::endl;
	  ss << "Make sure it exists or supply it directly";
	  std::cerr << ss.str() << std::endl;
	  throw kafarr::err(ss.str());
	}
	
	std::cerr << "send(...) - 2. got schema" << std::endl;
	/**
	 * convert the ARROW table to a list of AVRO datums
	 */
	std::vector<std::shared_ptr<avro::GenericDatum> > avro_data = arr_tbl_dcdr::arr2avr(tbl, schm);
	
	std::cerr << "send(...) - 3. generated data" << std::endl;
	/**
	 * create the topic object
	 */
	//std::shared_ptr<RdKafka::Topic> ktopic(RdKafka::Topic::create(_prdcr.get(), "test_topic_1", NULL, err));
	std::shared_ptr<RdKafka::Topic> ktopic(RdKafka::Topic::create(_prdcr.get(), topic, NULL, err));
	if (!ktopic){
	  std::cerr << "failed to create kafka topic: " << err << std::endl;
	  throw kafarr::err("failed to create kafka topic: ", err);
	}
	
	//for(auto i = 0; i < 10; ++i){
	for(auto i = 0; i < avro_data.size(); ++i){
	  std::vector<char> out;
	  
	  /**
	   * serialize the datum 
	   */
	  if(_srds_avro->serialize(schm.get(), avro_data[i].get(), out, err) == -1)
	    throw kafarr::err("failed to serialise message: ", err);
	  
	  /**
	   * and send...
	   */
	  const RdKafka::ErrorCode kerr = _prdcr->produce(ktopic.get(), partition, &out, NULL, NULL);	  
	  if (kerr != RdKafka::ERR_NO_ERROR) {
	    if (kerr == RdKafka::ERR__QUEUE_FULL) {
	      _prdcr->flush(100000);
	      const RdKafka::ErrorCode kerr = _prdcr->produce(ktopic.get(), partition, &out, NULL, NULL);	  
	      if (kerr != RdKafka::ERR_NO_ERROR)
		throw kafarr::err("Failed sending message for the second time!. Error:" + RdKafka::err2str(kerr));;
	    }
	    else
	      throw kafarr::err("Error sending message:" + RdKafka::err2str(kerr));;
	  }
	}
	
	//auto t1 = now_ms();
	_prdcr->flush(100000);
	//std::cerr << tbl->num_rows() << " Sent. Total sending time    : " << (now_ms() - t0) << "ms" << std::endl;
	//std::cerr << "          Message creation time : " << (t1 - t0) << "ms" << std::endl;
	//std::cerr << "          Message flushing time : " << (now_ms() - t1) << "ms" << std::endl;
      }
      catch (const kafarr::err& ke){
	std::cerr << "ERROR caught:>> " << ke.msg() << "\n" ;
	throw ke;
      }   
    }
  };  
  }

#endif //  __INCLUDE_PRDCR_HPP__
