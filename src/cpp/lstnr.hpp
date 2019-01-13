#ifndef __INCLUDE_LSTNR_HPP__
#define __INCLUDE_LSTNR_HPP__

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
  class lstnr : protected kfk_bse {
  private :
    const int RD_KFK_POLL_MS = 5;
    const std::unique_ptr<RdKafka::KafkaConsumer> _cnsmr;
    bool first_call = true;
    
  public:
    lstnr() = delete;
    lstnr(const std::string& srvr_lst,
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
    ~lstnr(){
      _cnsmr->close();
    }
    
  public:
    std::string poll(int n_msgs, std::shared_ptr<arrow::RecordBatch>* out, int max_tme_ms = 1000, int max_catchup_tme = 30000) {
      // no known schema id yet...
      int _val_blck_sch_id = -1;

      Serdes::Schema* schema = NULL;
      avro::GenericDatum *dtm = NULL;
      std::string err;
      std::string schema_name;
      std::unique_ptr<arrow::RecordBatchBuilder> bldr;
            
      auto [n, go]   = std::tuple(0, true);
      auto [t0, t00] = std::tuple(now_ms(), now_ms());
      
      while(go) {
	// poll kafka
	auto foo = now_ms();
	const std::shared_ptr<RdKafka::Message> msg(_cnsmr->consume(max_tme_ms));
	//std::cerr << (now_ms() - foo) << "ms" << std::endl;
	//std::cerr << msg->err() << std::endl;
	
	// check if result is an actual messgage
	if(kfk_hlpr::is_msg(msg)){
	  first_call = false;
	  // we have a message
	  // make sure there is no key, we currentlly don't process keys
	  if(msg->key()) throw kafarr::err("keyed messages currentlly not handled! ");

	  // if there is a payload
	  if(msg->payload()){
	    int msg_val_sch_id = -1;
	    
	    if(kfk_hlpr::val_cp1(msg)) {
	      // so message value is schema encoded - get schema id
	      const int msg_val_sch_id = kfk_hlpr::schm_id(msg->payload());

	      // first message processed by this poll
	      if(_val_blck_sch_id == -1) {
		// first message in a block. set schema
		// t00 = now_ms();  //DBG
		// std::cerr << "DEBUG:>> time waiting for the first message: " <<  (t00 - t0) << "ms" <<  std::endl;
		_val_blck_sch_id = msg_val_sch_id;

		if(_srds_avro->deserialize(&schema, &dtm, msg->payload(), msg->len(), err) == -1)
		  throw kafarr::err(" failed to deserialise messge: ", err);

		std::tuple<std::string, std::shared_ptr<arrow::Schema>> res = avr_hlpr::mk_arrw_schm(schema);
		schema_name = std::get<0>(res);
		auto pool = arrow::default_memory_pool();
		arrow::RecordBatchBuilder::Make(std::get<1>(res), pool, &bldr);
		avr_hlpr::rd_dta(msg->offset(), dtm, bldr);
		delete dtm;
	      }
	      else if(_val_blck_sch_id == msg_val_sch_id) {
		if(_srds_avro->deserialize(&schema, &dtm, msg->payload(), msg->len(), err) == -1)
		  throw kafarr::err(" failed to deserialise messge: ", err);
		avr_hlpr::rd_dta(msg->offset(), dtm, bldr);
		delete dtm;
	      }
	      else {
		//std::cerr<< "BREAK in schema. Previous was " << _val_blck_sch_id << ", current is : " << msg_val_sch_id << std::endl;	   
		//std::cerr << "rejecting message @ offset " << msg->offset() << " on topic::" << msg->topic_name() << " [" << msg->partition() << "]" << std::endl;	
		//read the state of client on partition message arrived on
		std::vector<RdKafka::TopicPartition*> prtns{RdKafka::TopicPartition::create(msg->topic_name(), msg->partition())};
		_cnsmr->position(prtns);
		//std::cerr << "rejecting message @ offset " << prtns[0]->offset() << " on topic::" << prtns[0]->topic() << " [" << prtns[0]->partition() << "]" << std::endl;
		
		//set a modified offset
		prtns[0]->set_offset(msg->offset());
		//std::cerr << "Resseting offset to:: [" << prtns[0]->offset() <<  "]@" << prtns[0]->partition() << std::endl;
		//
		
		//_cnsmr->offsets_store(prtns);
		_cnsmr->seek(*prtns[0], 1000);
		_cnsmr->commitSync(prtns);

		go = false;
		
		// TODO:>> remove once the above code is tested
		//throw kafarr::err("BUG. Break in schema is not handled.");
	      }
	    }
	    else
	      throw kafarr::err("only schema encoded messages currentlly handled");
	  }
	  // if message max or timed out
	  if (++n >= n_msgs) go = false;
        }
	else if (first_call && msg->err() == -185){
	  //std::cerr << " FIRST CALL. WAITING to catch up." << std::endl;
	  t00 = now_ms();
	}
	else {
	  if(msg->err() == -185){
	    // end of topic/partition (i.e. all messages read)
	    //std::cerr << " Not FIRST CALL. No MESSAGES." << std::endl;
	    go = false;
	  }
	  else if(msg->err() == -191){	  
	    //std::cerr << " No MORE MESSAGES." << std::endl;
	    // end of topic/partition (i.e. all messages read)
	    go = false;
	  }
	  first_call = false;
	}
	
	if (now_ms() - t00 > max_tme_ms) go = false;
      }

      if(schema) delete(schema);

      //std::cerr << "DEBUG:>> time waiting to catch up : " <<  (t0 - t00) << "ms" <<   std::endl;
      //std::cerr << "DEBUG:>> time popping messages    : " <<  (now_ms() - t00) << "ms" <<   std::endl;
      
      _cnsmr->commitSync();
      
      if(n > 0) bldr->Flush(out);

      return schema_name;
    }
  };  
}

#endif //  __INCLUDE_LSTNR_HPP__
