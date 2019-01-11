import pykafarr
import sys
import time
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
  instr = ['GBPUSD'] * n
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
srvrs   = 'kfk:9092'.encode('utf-8')
reg_url = 'http://kfk:8081'.encode('utf-8')

p = pykafarr.producer(srvrs, reg_url)


try:
  for i in range(0, 10):
    print("------- Run %. Sending 2000. ----" % i)
    data = gen_ticks(2000)
    p.send(cstr('avros.pricing.ig.Tick'), data, cstr('test_topic_1'))
    time.sleep(2)

except RuntimeError: 
  type, value, traceback = sys.exc_info()
  print("RuntimeError exception caught in python")
