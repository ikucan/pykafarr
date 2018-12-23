# -------------------------------------------------------
# NOTE: when run, this container starts an ssh deamon,
#       use with caution
# -------------------------------------------------------

FROM ubuntu:18.10

# use bash instead of sh
SHELL ["/bin/bash", "-c"]

## configure apt-get repos
RUN apt-get update --fix-missing
RUN apt-get install -yq apt-utils
## install required env
RUN   apt-get install -yq g++ git vim tree htop make cmake emacs
RUN   apt-get install -yq valgrind
RUN   apt-get install -yq libjansson-dev curl wget libboost-all-dev libsnappy-dev
#
RUN   apt-get install -yq libcurl4-openssl-dev
RUN   apt-get install -yq libcurl4-nss-dev
RUN   apt-get install -yq libcurl4-gnutls-dev

RUN   apt-get install -yV librdkafka1
RUN   apt-get install -yV librdkafka++1
RUN   apt-get install -yV librdkafka-dev

# install avro c
WORKDIR  /workstem
RUN      git clone https://github.com/apache/avro.git
WORKDIR  /workstem/avro
# release 1.8.2 is from May 2017
RUN      git checkout release-1.8.2
WORKDIR  /workstem/avro/lang/c
RUN      mkdir build
WORKDIR  build
RUN      cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local/ -DCMAKE_BUILD_TYPE=RelWithDebInfo
RUN      make -j 10
RUN      make test
RUN      make install
# and avro c++
WORKDIR  /workstem/avro/lang/c++
RUN      pwd
RUN      ./build.sh clean
RUN      ./build.sh install

#install serdes
WORKDIR  /workstem
RUN      git clone https://github.com/confluentinc/libserdes.git
WORKDIR  /workstem/libserdes
RUN      git checkout v5.1.0
RUN      ./configure
RUN      make -j 10
RUN      make install

ENV LD_LIBRARY_PATH=/usr/local/lib

# set up python
RUN      mkdir -p /opt/
RUN      mkdir -p /workstem/

#
## set up python
#
WORKDIR  /workstem
ENV      CONDA_VER=4.5.11
RUN      wget https://repo.continuum.io/miniconda/Miniconda3-$CONDA_VER-Linux-x86_64.sh
RUN      /bin/bash /workstem/Miniconda3-$CONDA_VER-Linux-x86_64.sh -b -p /opt/miniconda
RUN      rm /workstem/Miniconda3-$CONDA_VER-Linux-x86_64.sh
ENV      PATH=/opt/miniconda/bin/:$PATH
RUN      conda install -y numpy
RUN      conda install -y pandas
RUN      conda install -y pyarrow
RUN      conda install -y cython
RUN      python -V


##################################################
#
## check out the work (the SSH keys should be sufficient for authentication)
#
#WORKDIR  /workstem
#RUN      git clone https://github.com/ikucan/pykafarr.git
#WORKDIR  /workstem/pykafarr/src/
#RUN      python setup.py build_ext --inplace

