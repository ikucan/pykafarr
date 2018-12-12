# distutils: language = c++

from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp.memory cimport shared_ptr

import ctypes
from pyarrow.includes.libarrow cimport CRecordBatch, CTable

cdef extern from "kafarr.hpp" namespace "kafarr":
    cdef cppclass lstnr:
        lstnr(string&, string&, vector[string]&, string&) except +
        string poll(int, shared_ptr[CRecordBatch]*, int) except +
        void   send(string&, shared_ptr[CTable]) except +
        void   ex_tst(string&) except +
