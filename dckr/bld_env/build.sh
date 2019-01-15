#!/bin/bash
cd /workstem
rm -Rf pykafarr
git clone https://github.com/ikucan/pykafarr.git
cd pykafarr/conda
conda-build --no-remove-work-dir .
#anaconda upload --force /opt/miniconda/conda-bld/linux-64/pykafarr-0.7.0-py37_0.tar.bz2
