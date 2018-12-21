import os
from   setuptools     import setup, find_packages
from   distutils.core import Extension
import sys
import platform

import numpy
import pyarrow

srcs=['pykafarr.pyx']
libs=['m', 'dl', 'pthread', 'rt', 'stdc++', 'serdes++', 'serdes', 'rdkafka++', 'rdkafka', 'arrow']

compile_args=['-std=c++17', '-fPIC']

#suppress all compiler warnings
compile_args.append('-w')

with open("README.md", "r") as fh:
    long_description = fh.read()
    
module = Extension(
    name       = 'pykafarr', 
    sources    = srcs,
    libraries  = libs,
    language   = 'c++',
    extra_compile_args = compile_args,
    include_dirs = [numpy.get_include(), pyarrow.get_include(), 'cpp/']
)

setup(
    name             = 'pykafarr', 
    version          = '0.6.0.3',
    author           = 'iztok kucan',
    author_email     = 'iztok.kucan@gmail.com',
    url              = 'https://github.com/ikucan/pykafarr',
    long_description = long_description,
    long_description_content_type ="text/markdown",
    ext_modules      = [module],
    python_requires  = '>=3.5.',
    install_requires = ['numpy', 'pyarrow', 'pandas'],
    setup_requires   = ['numpy', 'pyarrow', 'pandas'],
    #extras_require   = ['numpy'],
    classifiers      = [
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: Apache Software License",
        "Operating System :: OS Independent",
        'Programming Language :: Python :: 3.5',
        'Programming Language :: Python :: 3.6',
        'Programming Language :: Python :: 3.7',
    ],
)

