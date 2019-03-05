import pykafarr
import sys
import random as r
import numpy  as np
import pandas as pd
import time 

def cstr(s):
    return s.encode('utf-8')

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
  lstnr = pykafarr.listener(srvrs, grp_id, [tpc_1], reg_url)

  n_ticks = 10

  try:
    nme, frm = lstnr.poll(n_ticks, 20)
    print(frm)
    for i in range(1):
      time.sleep(1)
      print('.',end='')
    print
    #prdcr.send(cstr(nme), frm, tpc_2)
    prdcr.send(cstr('avros.pricing.ig.Tick'), frm.drop(columns=['offst']), tpc_2)
  except RuntimeError: 
    type, value, traceback = sys.exc_info()
    print("RuntimeError exception caught in python")

#
##
#
if __name__ == "__main__":
    main()
