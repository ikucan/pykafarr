#include <iostream>
#include "kafarr.hpp"

#include <unistd.h>
#include <limits.h>


/**
 * main
 */
int main(int argc, char** argv) {
  /**
   * get a hostname where the kafka server and schema registry server live...
   */
  char hostname[HOST_NAME_MAX];
  gethostname(hostname, HOST_NAME_MAX);
  std::string kfk_hst = hostname;
  if(argc == 2) kfk_hst = argv[1];
  
  std::cerr << "kafka host: " << kfk_hst << std::endl;  

  try{
    //kafarr::lstnr l(kfk_hst, "cpp_tst_grp" , {"CS.D.GBPUSD.MINI.IP_TOPICX"}, "http://" + kfk_hst + ":8081");
    kafarr::lstnr l(kfk_hst, "cpp_tst_grp" , {"test_topic_1"}, "http://" + kfk_hst + ":8081");

    auto go = true;
    auto n = 0;
    while(go) {
      std::shared_ptr<arrow::RecordBatch> rcrds;
      auto name = l.poll(200, &rcrds, 5000);
      
      if(rcrds){
	std::cerr << "--------------------------------------------\n";
	std::cerr << "Batch #: " << n << "\n";
	std::cerr << "--------------------------------------------\n";
	std::cerr << "message name: " << name << std::endl;
	std::cerr << "#cols: " << rcrds->num_columns() << std::endl;
	std::cerr << "#rows: " << rcrds->num_rows() << std::endl;
	
	for(int i = 0; i < rcrds->num_columns(); ++i) {
	  auto col =  rcrds->column(i);
	  //std::cerr << rcrds->column_name(i) << "[" << col->type()->ToString() << "] : ";
	  if(rcrds->column_name(i) == "offst" ){
	    std::cerr << rcrds->column_name(i) << "[" << col->type()->ToString() << "] : ";
	    std::cerr << col->ToString();
	    std::cerr << std::endl;
	  }
	}
	std::cerr << "--------------------------------------------\n";
	std::cerr << "--------------------------------------------\n";
	n += 1;
	if (n > 10) go = false;
      }
    }
  }
  catch (const kafarr::err& ke) {
    std::cerr << "ERROR caught:>> " << ke.msg() << "\n" ;
  }
  catch(...) {
    std::cerr << "ERROR caught:>> UNKNOWN exception!!!\n" ;
  }
  std::cerr << "\n" ;
}

