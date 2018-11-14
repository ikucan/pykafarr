#ifndef __INCLUDE_ERR_HPP__
#define __INCLUDE_ERR_HPP__

namespace kafarr {

  /**
   * kafarr exception class. the only exception class thrown here
   */
  class err : std::exception {
  private:
    const std::string _msg;
  public:
    err(const std::string& m1, const std::string& m2):_msg(std::string().append(m1).append(m2)) {}
    
    err(const std::string& m):_msg(m) {}
    
    virtual const char* what() {return _msg.c_str();}

    const std::string msg() const {return _msg;}
    
  } ;
}

#endif
