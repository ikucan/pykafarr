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

    def _run_cmake(self):
        print("RUN CMAKE")
        # The directory containing this setup.py
        source = os.path.dirname(os.path.abspath(__file__))

        # The staging directory for the module being built
        build_temp = pjoin(os.getcwd(), self.build_temp)
        build_lib = os.path.join(os.getcwd(), self.build_lib)
        saved_cwd = os.getcwd()

        if not os.path.isdir(self.build_temp):
            self.mkpath(self.build_temp)
            
        print("build_temp:>> " + build_temp)
        print("self.build_tmp:>> " + self.build_temp)
        print("build_lib:>> " + build_lib)
        print("saved_cwd:>> " + saved_cwd)

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


