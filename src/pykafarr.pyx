# distutils: language = c++
# cython: language_level=3

from libcpp.string cimport string
from libcpp.vector cimport vector
from pykafarr cimport lstnr, prdcr
from libcpp.memory cimport shared_ptr

import ctypes
import pyarrow as pa
from pyarrow.includes.libarrow cimport CRecordBatch, CTable
from pyarrow.lib cimport RecordBatch, pyarrow_wrap_batch, pyarrow_unwrap_table, check_status

#
#
#
cdef class producer:
  '''wrap the C++ Kafka producer class in a python object'''
  cdef prdcr* c_obj

  def __cinit__(self, string server_list, schema_registry_url):
    self.c_obj = new prdcr(server_list, schema_registry_url)

  def __dealloc__(self):
    del self.c_obj
    
  def send(self, string& msg_typ, frm, string& topic, int partition = -1):
    pa_tbl = pa.Table.from_pandas(frm)
    cdef shared_ptr[CTable] pptr = pyarrow_unwrap_table(pa_tbl)
    if pptr:
      self.c_obj.send(msg_typ, pptr, topic, partition)

#
#
#
cdef class listener:
  '''wrap the C++ Kafka listener class in a python object'''
  cdef lstnr* c_obj

  def __cinit__(self, string server_list, string group_id, vector[string] topics, schema_registry_url):
    self.c_obj = new lstnr(server_list, group_id, topics, schema_registry_url)

  def __dealloc__(self):
    del self.c_obj

  def poll(self, int num_messages, int max_time):
      cdef shared_ptr[CRecordBatch] ptr;
      msg_name = self.c_obj.poll(num_messages, &ptr, max_time)
      return msg_name.decode('utf-8'), pyarrow_wrap_batch(ptr).to_pandas() if ptr else None
