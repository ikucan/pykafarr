#include <iostream>
#include <prdcr.hpp>

#include <unistd.h>
#include <limits.h>

std::shared_ptr< arrow::Table > mk_tck_tbl(const int n) {
  const std::string msg_typ = "avros.pricing.ig.Tick";
  
  std::shared_ptr<Serdes::Conf>   srds_conf = kafarr::kfk_hlpr::mk_srds_conf("kfk:8081");
  std::shared_ptr<Serdes::Handle> srds_hndl = kafarr::kfk_hlpr::mk_srds_hndl(srds_conf);
  
  std::string err;
  Serdes::Schema* schm = Serdes::Schema::get(srds_hndl.get(), msg_typ, err);
  std::tuple<std::string, std::shared_ptr<arrow::Schema>> res = kafarr::avr_hlpr::mk_arrw_schm(schm);
  std::cerr << "arrow schema name:: " << std::get<0>(res) << std::endl;
  if(msg_typ != std::get<0>(res)){
    std::stringstream ss ;
    ss << "ERROR:: message type mismatch. Expected: " << msg_typ << " but received: " << std::get<0>(res);
    std::cerr << ss.str() << std::endl;
    throw kafarr::err(ss.str());
  }

  /**
   * remove the offset field when sending
   */
  std::shared_ptr<arrow::Schema> schm2;
  std::get<1>(res)->RemoveField(5, &schm2);
  
  std::unique_ptr<arrow::RecordBatchBuilder> bldr;
  auto pool = arrow::default_memory_pool();
  arrow::RecordBatchBuilder::Make(schm2, pool, &bldr);

  std::cerr << bldr->num_fields() << std::endl;
  for(auto i = 0; i < 100; ++i){
    for (auto j = 0; j < bldr->num_fields(); ++j) {
      auto fld = bldr->GetField(j);
      //std::cerr << "fld(" << i<< "): " << fld->type()->name() << std::endl;
      if(j == 0)      static_cast<arrow::StringBuilder *>(fld)->Append("xxxx_" + std::to_string(i));
      else if(j == 1) static_cast<arrow::Int64Builder  *>(fld)->Append(1234554321l + i);
      else if(j == 2) static_cast<arrow::Int32Builder  *>(fld)->Append(234 + i);
      else if(j == 3) static_cast<arrow::FloatBuilder  *>(fld)->Append(1.3f - (i*1000.0f/10000.0f));
      else if(j == 4) static_cast<arrow::FloatBuilder  *>(fld)->Append(1.3f + (i*1000.0f/10000.0f));
    }
  }

  std::shared_ptr<arrow::RecordBatch> batch;
  bldr->Flush(&batch);
  
  std::shared_ptr< arrow::Table > table;
  std::vector< std::shared_ptr< arrow::RecordBatch > >  batches = {batch};
  auto status =  arrow::Table::FromRecordBatches(batches, &table);

  return table;
}

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

  const std::string tck_msg_typ = "avros.pricing.ig.Tick";

  std::cerr << "kafka host: " << kfk_hst << std::endl;  

  try{
    auto tck_tbl = mk_tck_tbl(10);
    kafarr::prdcr p(kfk_hst, "cpp_tst_grp" , {"test_topic_1"}, "http://" + kfk_hst + ":8081");
    p.send2(tck_msg_typ, tck_tbl);
  }
  catch (const kafarr::err& ke){
    std::cerr << "ERROR caught:>> " << ke.msg() << "\n" ;
  }
  catch(...) {
    std::cerr << "ERROR caught:>> UNKNOWN exception!!!\n" ;
  }
  std::cerr << "\n" ;
}
