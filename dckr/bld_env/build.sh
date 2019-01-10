#!/bin/bash
cd /workstem
pwd
rm -Rf pykafarr
ls -Fila
git clone https://github.com/ikucan/pykafarr.git
ls -Fila
cd pykafarr/conda
pwd
ls -Fila
conda-build --no-remove-work-dir .
anaconda upload --force /opt/miniconda/conda-bld/linux-64/pykafarr-0.7.0-py37_0.tar.bz2
