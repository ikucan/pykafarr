import rel_pth

import pykafarr
import sys
import random as r
import numpy  as np
import pandas as pd
from   time import time

def cstr(s):
    return s.encode('utf-8')

##
## helper method. generates a data frame with some stuff in it
##
def gen_ticks(n):
  instr = ['GBPUSD'.encode('utf-8')] * n
  tms   = np.array(list(np.int64(time()*1000) for x in range(n)))
  dt    = np.array(list(np.int32(r.randint(0,150)) for x in range(n)))
  mid   = np.array(list(np.float32((125000 + r.randint(-100, 100))/100000) for x in range(n)))
  sprd  = np.array(list(np.float32(r.randint(1, 10)/100000) for x in range(n)))
  bid   = mid - sprd
  ask   = mid + sprd
  return pd.DataFrame({'inst':instr, 't':tms, 'dt':dt, 'bid':bid, 'ask':ask})

##
## main 
##
print('------------------')
#lst     = [str(i*10 + 2).encode('utf-8') for i in range(1, 4)]
srvrs   = 'kfk:9092'.encode('utf-8')
grp_id  = ('test_grp_' + str(r.randint(10000, 20000))).encode('utf-8')
tpcs    = ['test_topic_2'.encode('utf-8')]
reg_url = 'http://kfk:8081'.encode('utf-8')

p = pykafarr.producer(srvrs, reg_url)

data = gen_ticks(20)


try:
  p.send(cstr('avros.pricing.ig.Tick'), data, cstr('test_topic_1'))
except RuntimeError: 
  type, value, traceback = sys.exc_info()
  print("RuntimeError exception caught in python")
