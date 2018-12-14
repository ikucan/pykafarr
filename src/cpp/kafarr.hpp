#ifndef __INCLUDE_KAFARR_HPP__
#define __INCLUDE_KAFARR_HPP__

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
  class bse{
  protected:
    std::shared_ptr<Serdes::Conf>   _srds_conf;
    std::shared_ptr<Serdes::Handle> _srds_hndl;
    std::unique_ptr<Serdes::Avro>   _srds_avro;
    
  public:
    bse() = delete;
    bse(const std::string& reg_url) :
      _srds_conf(kfk_hlpr::mk_srds_conf(reg_url)),
      _srds_hndl(kfk_hlpr::mk_srds_hndl(_srds_conf)),
      _srds_avro(kfk_hlpr::mk_avro(_srds_conf))
    {}    
    virtual ~bse(){}
  };

  /**
   *
   */
  class lstnr : protected bse {
  private :
    const int RD_KFK_POLL_MS = 5;
    const std::unique_ptr<RdKafka::KafkaConsumer> _cnsmr;
    const std::unique_ptr<RdKafka::KafkaConsumer> _prdcr;
    bool first_call = true;
    
  public:
    lstnr() = delete;
    lstnr(const std::string& srvr_lst,
	  const std::string& grp,
	  const std::vector<std::string>& tpcs,
	  const std::string& reg_url) : bse(reg_url),_cnsmr(kfk_hlpr::mk_kfk_cnsmr(grp, srvr_lst))
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

  private:
    auto now_ms() {
      return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
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

    // TODO:>>  
    void ex_tst(const std::string& msg) {      
      std::cerr << "about to throw TEST exception: " << msg << "\n";
      throw kafarr::err("thrown TEST exception. msg: " + msg);
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

#endif //  __INCLUDE_KAFARR_HPP__
