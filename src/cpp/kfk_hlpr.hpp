#ifndef __INCLUDE_KFK_HLPR_HPP__
#define __INCLUDE_KFK_HLPR_HPP__

#include <arpa/inet.h>
#include <string.h>

#include <memory>
#include <sstream>

#include <librdkafka/rdkafkacpp.h>


namespace kafarr {
  class kfk_hlpr {
  public:
    kfk_hlpr() = delete;
  public:
        
    /**
     * create a kafka consumer handle
     * TODO:>> add exception for failed creation
     */
    static std::unique_ptr<RdKafka::KafkaConsumer> mk_kfk_cnsmr(const std::string& grp, const std::string& brkr_lst) {
      std::string _err;
      
      const std::unique_ptr<RdKafka::Conf> kconf(RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL));
      
      if (kconf->set("metadata.broker.list", brkr_lst, _err) != RdKafka::Conf::ConfResult::CONF_OK)
	throw kafarr::err("failed to set broker list property . ", _err);
      
      if (kconf->set("group.id", grp, _err) != RdKafka::Conf::ConfResult::CONF_OK)
	throw kafarr::err("failed to set group id property . ", _err);
      
      if (kconf->set("enable.auto.commit", "false", _err) != RdKafka::Conf::ConfResult::CONF_OK)
	throw kafarr::err("failed to set auto commit property . ", _err);
      
      
      /** 
       * create kafka consumer
       */ 
      std::unique_ptr<RdKafka::KafkaConsumer> cnsmr(RdKafka::KafkaConsumer::create(kconf.get(), _err));
      if (!cnsmr) throw kafarr::err("failed to crate kafka consumer. ", _err);
      
      return cnsmr;
    }


    /**
     * create a serdes handle
     * TODO::> combine errors with exceptions
     */
    static std::unique_ptr<Serdes::Avro> mk_srds(const std::string& reg_url) {
      std::string _err;
      const std::unique_ptr <Serdes::Conf> sconf(Serdes::Conf::create());
      
      if (sconf->set("schema.registry.url", reg_url, _err)) throw kafarr::err("failed to set schema url. ", _err);
      if (sconf->set("deserializer.framing", "cp1", _err)) throw kafarr::err("faled to set framing. ", _err);
      
      std::unique_ptr<Serdes::Avro> srds(Serdes::Avro::create(sconf.get(), _err));      
      if (!srds) throw kafarr::err("failed to create serdes. ", _err);
      
      return srds;
    }


    /**
     * check if there is a valid message
     */
    static bool is_msg(const std::shared_ptr<RdKafka::Message> msg, const bool strct = false) {
      if(msg->err() == RdKafka::ERR_NO_ERROR) return true;
      else if(msg->err() == RdKafka::ERR__TIMED_OUT) return false;
      else if(strct) {
	std::stringstream ss ;
	ss << "BUG!!!. Unhandled message status code (msg->err()): " << msg->err();
	std::cerr << "--------------------------------------------------" << std::endl;
	std::cerr << ss.str() << std::endl;
	std::cerr << "--------------------------------------------------" << std::endl;
	throw kafarr::err(ss.str());
      }
      else return false;
    }


    /**
     * check if the message key/value is cp1 framed
     */
    static bool key_cp1(const std::shared_ptr<RdKafka::Message> msg) {
      // must be 5 bytes long (at least...)
      if(msg->key_len() < 5) return false;
      // magic byte 
      else if(((const char*)msg->key_pointer())[0] != 0) return false;
      else return true;
    }
    static bool val_cp1(const std::shared_ptr<RdKafka::Message> msg) {
      // must be 5 bytes long (at least...)
      if(msg->len() < 5) return false;
      // magic byte 
      else if(((const char*)msg->payload())[0] != 0) return false;
      else return true;
    }

    /**
     * extract the schema id from a CP1 framed buffer
     * if cp1 framed, the schema is in bytes 1,2,3,4 of the buffer in netowrk significant order
     */
    static int schm_id(const void *pyld){
      // must be 5 bytes long (at least...)
      int sch_id;
      memcpy(&sch_id, (const char*)pyld + 1, 4); // copy bytes 1,2,3,4 from the buffer 
      sch_id = ntohl(sch_id); // reverse bytes
      return sch_id;
    }
        

  };
}

#endif // __INCLUDE_KFK_HLPR_HPP__
