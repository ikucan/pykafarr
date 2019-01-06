#!/usr/bin/env python

import sys
import os

from Cython.Distutils import build_ext as _build_ext
import Cython

import pkg_resources
from setuptools import setup, find_packages, Extension, Distribution

from os.path import join as pjoin

from distutils.command.clean import clean as _clean
from distutils.util import strtobool
from distutils import sysconfig

# Check if we're running 64-bit Python
is_64_bit = sys.maxsize > 2**32
if Cython.__version__ < '0.27':
    raise Exception('Please upgrade to Cython 0.27 or newer')

setup_dir = os.path.abspath(os.path.dirname(__file__))
#print("Setup DIR:>> " + setup_dir)

class BinaryDistribution(Distribution):
    def has_ext_modules(foo):
        return True
    

class clean(_clean):
    def run(self):
        print("running CLEAN: ", _clean)
        _clean.run(self)
        for x in []:
            try:
                #os.remove(x)
                print("CLEANING:>> " + str(x))
            except OSError:
                pass


class build_ext(_build_ext):
    _found_names = ()

    def build_extensions(self):
        print("BUILD EXTENSIONS")
        numpy_incl = pkg_resources.resource_filename('numpy', 'core/include')
        
        #print(dir(pkg_resources))
        self.extensions = [ext for ext in self.extensions if ext.name != '__dummy__']
        print(self.extensions)

    #def run(self):
        #self._run_cmake()
        #_build_ext.run(self)

    user_options = ([('XXX', None, 'CMake generator'),
                     ('bundle-arrow-cpp', None, 'bundle the Arrow C++ libraries')] +
                    _build_ext.user_options)

    #print(">>" + str(_build_ext.user_options))
    def initialize_options(self):
        print("INIT OPTS")
        _build_ext.initialize_options(self)

    def mk_incldpth(self):
        ##py_incl_pth = sys.prefix
        paths=[]
        # ptyhon headers
        paths.append(sysconfig.get_python_inc())

        # pykafarr c++ impl
        paths.append(os.getcwd() + "/cpp")
        
        return ['-I' + pth for pth in paths]
        
    def _run_cmake(self):
        print("RUN CMAKE")
        # The directory containing this setup.py
        source = os.path.dirname(os.path.abspath(__file__))
        build_temp = pjoin(os.getcwd(), self.build_temp)
        build_lib = os.path.join(os.getcwd(), self.build_lib)
        saved_cwd = os.getcwd()

        if not os.path.isdir(self.build_temp):
            self.mkpath(self.build_temp)
        if not os.path.isdir(self.build_lib):
            self.mkpath(self.build_lib)
            
        print("---------------------------------------------")
        print("build_temp     :>>  %s" % build_temp)
        print("self.build_tmp :>>  %s" % self.build_temp)
        print("build_lib      :>>  %s" % build_lib)
        print("saved_cwd      :>>  %s" % saved_cwd)
        print("python exe     :>>  %s" % sys.executable)
        print("python env     :>>  %s" % sys.prefix)
        print("---------------------------------------------")        

        ## ###################################################

        # run the cython compiler to generate the cpp interface
        self.spawn(['python', '-m',  'cython', '--cplus', '-X', 'language_level=3', 'pykafarr.pyx'])

        # compile the pykafarr source into an object 
        inclds = self.mk_incldpth()
        compat_args = ['-B', pjoin(sys.prefix, 'compiler_compat')]
        cmplr_args = ['-pthread', '-Wl,--sysroot=/', '-std=c++17', '-DNDEBUG', '-fPIC', '-fwrapv', '-O3', '-g']
# -pthread -B /opt/python/miniconda3/envs/pykafarr/compiler_compat -Wl,--sysroot=/ -Wsign-compare -DNDEBUG -g -fwrapv -O3 -Wall -Wstrict-prototypes -fPIC -I/opt/python/miniconda3/envs/pykafarr/lib/python3.7/site-packages/numpy/core/include -I/opt/python/miniconda3/envs/pykafarr/lib/python3.7/site-packages/pyarrow/include -Icpp/ -I/opt/python/miniconda3/envs/pykafarr/include/python3.7m -c pykafarr.cpp -o build/temp.linux-x86_64-3.7/pykafarr.o -std=c++17 -fPIC -w
        obj_file = pjoin(build_temp , 'pykafarr.o')
        cmpl_cmd = ['gcc'] + compat_args + cmplr_args + inclds + [ '-c', 'pykafarr.cpp', '-o', obj_file]
        print(cmpl_cmd)        
        self.spawn(cmpl_cmd)

        # link...
        lnkr_args = ['-pthread', '-shared']
#g++
# -pthread
# -shared
# -B /opt/python/miniconda3/envs/pykafarr/compiler_compat
# -L/opt/python/miniconda3/envs/pykafarr/lib
# -Wl,-rpath=/opt/python/miniconda3/envs/pykafarr/lib
# -Wl,--no-as-needed
# -Wl,--sysroot=/
# build/temp.linux-x86_64-3.7/pykafarr.o
#-lm -ldl -lpthread -lrt -lstdc++ -lserdes++ -lserdes -lrdkafka++ -lrdkafka -larrow
#-o build/lib.linux-x86_64-3.7/pykafarr.cpython-37m-x86_64-linux-gnu.so
        extr=['-Wl,-rpath=/opt/python/miniconda3/envs/pykafarr/lib', '-Wl,--no-as-needed', '-Wl,--sysroot=/']

        so_name='pykafarr.so'
        so_file = pjoin(build_lib, so_name)
        libs=['-l'+lib for lib in ['m', 'dl', 'pthread', 'rt', 'stdc++', 'serdes++', 'serdes', 'rdkafka++', 'rdkafka', 'arrow']]
        lnk_cmd = ['g++'] + compat_args + extr + lnkr_args + libs + [obj_file, '-o', so_file]
        self.spawn(lnk_cmd)
        
    def run(self):
        print("RUN")
        self._run_cmake()
        _build_ext.run(self)

setup(
    name="pykafarr",
    packages=find_packages(),
    zip_safe=False,
    package_data={'pykafarr': ['*.pxd', '*.pyx']},
    include_package_data=True,
    distclass=BinaryDistribution,
        # Dummy extension to trigger build_ext
    ext_modules=[Extension('__dummy__', sources=[])],
    cmdclass={
        'clean': clean,
        'build_ext': build_ext
    },
    
)


