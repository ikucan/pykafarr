# distutils: language = c++

from libcpp.string cimport string
from libcpp.vector cimport vector
from pykafarr cimport lstnr
from libcpp.memory cimport shared_ptr

import ctypes
from pyarrow.includes.libarrow cimport CRecordBatch
from pyarrow.lib cimport RecordBatch, pyarrow_wrap_batch, check_status

cdef class listener:
  cdef lstnr* c_obj

  def __cinit__(self, string server_list, string group_id, vector[string] topics, schema_registry_url):
    self.c_obj = new lstnr(server_list, group_id, topics, schema_registry_url)

  def __dealloc__(self):
    del self.c_obj

  def poll(self, int num_messages, int max_time):
      cdef shared_ptr[CRecordBatch] ptr;
      msg_name = self.c_obj.poll(num_messages, &ptr, max_time)
      return msg_name.decode('utf-8'), pyarrow_wrap_batch(ptr).to_pandas() if ptr else None

  def ex_tst(self, string msg):
      self.c_obj.ex_tst(msg)
