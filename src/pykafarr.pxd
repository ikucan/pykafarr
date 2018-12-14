# distutils: language = c++

from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp.memory cimport shared_ptr

import ctypes
from pyarrow.includes.libarrow cimport CRecordBatch, CTable

cdef extern from "lstnr.hpp" namespace "kafarr":
  cdef cppclass lstnr:
    lstnr(string&, string&, vector[string]&, string&) except +
    string poll(int, shared_ptr[CRecordBatch]*, int) except +

cdef extern from "prdcr.hpp" namespace "kafarr":
  cdef cppclass prdcr:
    prdcr(string&, string&, vector[string]&, string&) except +
    void send(string&, shared_ptr[CTable]) except +
