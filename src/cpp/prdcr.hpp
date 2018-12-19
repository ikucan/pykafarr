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
    const std::unique_ptr<RdKafka::KafkaConsumer> _prdcr;
    bool first_call = true;
    
  public:
    prdcr() = delete;
    prdcr(const std::string& srvr_lst,
	  const std::string& reg_url) : kfk_bse(reg_url)
    {      
    }
    
  public:
    /**
     * destructor
     */
    ~prdcr(){}

  public:
    void send(const std::string& msg_typ, std::shared_ptr<arrow::Table> tbl) {

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

      std::vector<std::shared_ptr<avro::GenericDatum> > avro_data = arr_tbl_dcdr::arr2avr(tbl, schm);
      
      /**
       * take the arrow table and serialise it to a vector of generic avro objects
       */
      {
	{
	  
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
	  
	  //for(auto i = 0; i < 10; ++i){
	  for(auto i = 0; i < avro_data.size(); ++i){
	    std::vector<char> out;

	    // debug
	    std::shared_ptr<avro::GenericDatum>  dtm = avro_data[i];
	    avro::GenericRecord& rcrd = dtm->value<avro::GenericRecord>();
	    /* Serialize Avro */
	    if(_srds_avro->serialize(schm.get(), avro_data[i].get(), out, err) == -1)
	      throw kafarr::err("failed to serialise message: ", err);
	    
	    RdKafka::ErrorCode kerr = producer->produce(ktopic, -1, &out, NULL, NULL);	  
	    if (kerr != RdKafka::ERR_NO_ERROR) 
	      std::cerr << "% Failed to produce message: " << RdKafka::err2str(kerr) << std::endl;	  
	  }
	  
	  producer->flush(3000);
	
	}
      }      
    }
  };  
}

#endif //  __INCLUDE_PRDCR_HPP__
