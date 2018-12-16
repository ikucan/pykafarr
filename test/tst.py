import sys
import os
from os.path import dirname, realpath


script_dir = dirname(realpath(__file__))
src_dir   =  script_dir + '/../src'
sys.path.append(src_dir)

import pykafarr
