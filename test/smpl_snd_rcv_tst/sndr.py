import pykafarr
import sys
import random as r
import numpy  as np
import pandas as pd
import time

def cstr(s):
    return s.encode('utf-8')

##
## helper method. generates a data frame with some stuff in it
##
def gen_ticks(n):
  instr = ['GBPUSD'] * n
  tms   = np.array(list(np.int64(time.time()*1000) for x in range(n)))
  dt    = np.array(list(np.int32(r.randint(0,150)) for x in range(n)))
  mid   = np.array(list(np.float32((125000 + r.randint(-100, 100))/100000) for x in range(n)))
  sprd  = np.array(list(np.float32(r.randint(1, 10)/100000) for x in range(n)))
  bid   = mid - sprd
  ask   = mid + sprd
  return pd.DataFrame({'inst':instr, 't':tms, 'dt':dt, 'bid':bid, 'ask':ask})

#
##
#
def main():
  print('---- DATA GENERATOR STARTING --------------')
  srvrs   = cstr('kfk:9092')
  grp_id  = cstr('test_grp_' + str(r.randint(10000, 20000)))
  tpc_1   = cstr('test_topic_1_1')
  tpc_2   = cstr('test_topic_1_2')
  reg_url = cstr('http://kfk:8081')

  prdcr = pykafarr.producer(srvrs, reg_url)
  lstnr = pykafarr.listener(srvrs, grp_id, [tpc_2], reg_url)

  # generate a block of 2000 ticks
  n_ticks = 10
  data = gen_ticks(n_ticks)

  try:
      print ('- SENDING ---------------------')
      print(data)
      prdcr.send(cstr('avros.pricing.ig.Tick'), data, tpc_1)
      nme, frm = lstnr.poll(n_ticks, 20)
      print ('- RECEIVED --------------------')
      print(frm)
  except RuntimeError: 
      type, value, traceback = sys.exc_info()
      print("RuntimeError exception caught in python")

  #for i in range(0, 1000):
  #  print ('run number:>> ', i)
  #  if frm is not None:
  #      print(nme
  #      print(frm.shape)
  #      #print(type(frm))
  #      print(frm)

#
##
#
if __name__ == "__main__":
    main()
