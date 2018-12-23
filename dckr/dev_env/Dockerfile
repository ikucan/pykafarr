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

RUN      mkdir -p /workstem/

#
# NOTE: rdkafka, avro, serdes and arrow chechouts build a specific release
# as the head can occasionally be broken (e.g. avro)
# This really should be a specific [major] release!
#

# install librdkafka
WORKDIR  /workstem
RUN      git clone https://github.com/edenhill/librdkafka.git
WORKDIR  librdkafka
RUN      git checkout v1.0.0-RC5
#RUN      git checkout eb51812ace00fc3ef3999f8ec041482b1147b0de
RUN      ./configure 
RUN      make -j 4
RUN      make install

# install avro c
WORKDIR  /workstem
RUN      git clone https://github.com/apache/avro.git
WORKDIR  /workstem/avro
# release 1.8.2 is from May 2017
RUN      git checkout release-1.8.2
#RUN      git checkout 787be93d3cb1ef8f90e9e136cff6b0d2b66f4bcb
WORKDIR  /workstem/avro/lang/c
RUN      mkdir build
WORKDIR  build
RUN      cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local/ -DCMAKE_BUILD_TYPE=RelWithDebInfo
RUN      make -j 4
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
RUN      make -j 4
RUN      make install

#install arrow
WORKDIR  /workstem
RUN      git clone https://github.com/apache/arrow.git
WORKDIR  /workstem/arrow/
RUN      git checkout apache-arrow-0.11.1
WORKDIR  /workstem/arrow/cpp
RUN      mkdir release
WORKDIR  release
RUN      cmake .. -DCMAKE_BUILD_TYPE=Release
RUN      make -j 4
RUN      make install

# set up python
RUN      mkdir -p /opt/

#
## set up python
#
WORKDIR  /workstem
RUN      wget https://repo.continuum.io/miniconda/Miniconda3-latest-Linux-x86_64.sh
RUN      /bin/bash /workstem/Miniconda3-latest-Linux-x86_64.sh -b -p /opt/miniconda
RUN      rm /workstem/Miniconda3-latest-Linux-x86_64.sh
ENV      PATH=/opt/miniconda/bin/:$PATH
RUN      conda install -y numpy
RUN      conda install -y pandas
RUN      conda install -y pyarrow
RUN      conda install -y cython
RUN      python -V

#
## all third party sources are built into default prefix, so needs to be in the path
#
ENV LD_LIBRARY_PATH=/usr/local/lib

