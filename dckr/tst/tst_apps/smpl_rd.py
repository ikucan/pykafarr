import pykafarr
import sys
import random

def cstr(s):
    return s.encode('utf-8')

srvrs   = 'kfk:9092'.encode('utf-8')
grp_id  = ('test_grp_' + str(random.randint(10000, 20000))).encode('utf-8')
tpcs    = ['test_topic_1'.encode('utf-8')]
reg_url = 'http://kfk:8081'.encode('utf-8')

p = pykafarr.listener(srvrs, grp_id, tpcs, reg_url)

for i in range(0, 1000):
    print ('------------------------------')
    print ('poll # :>> %d' % i)
    nme, frm = p.poll(50, 5000)
    if frm is not None:
        print(nme)
        print(frm.shape)
        #print(type(frm))
        print(frm)

