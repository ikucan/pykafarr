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
RUN   apt-get install -yq ssh


# RUN   apt-get update --fix-missing  &&\
#     apt-get install -yq    \
#       apt-utils \
#       htop tree vim \
#       ssh \
#        make cmake git g++ valgrind \
#        libjansson-dev curl wget libboost-all-dev libsnappy-dev \
#        libcurl4-openssl-dev libcurl4-nss-dev libcurl4-gnutls-dev && \
#      rm -rf /var/lib/apt/lists/*

RUN      mkdir -p /workstem/
WORKDIR  /workstem

#
# NOTE: rdkafka, avro, serdes and arrow chechouts build a specific release
# as the head can occasionally be broken (e.g. avro)
# This really should be a specific [major] release!
#

# install librdkafka
RUN      git clone https://github.com/edenhill/librdkafka.git
WORKDIR  librdkafka
RUN      git checkout eb51812ace00fc3ef3999f8ec041482b1147b0de
RUN      ./configure 
RUN      make -j 10
RUN      make install

# install avro c
WORKDIR  /workstem
RUN      git clone https://github.com/apache/avro.git
WORKDIR  /workstem/avro
RUN      git checkout 787be93d3cb1ef8f90e9e136cff6b0d2b66f4bcb
#RUN      git checkout release-1.8.2
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
RUN      git checkout 2ed6fa318413df781b5fde8e74b504a9b42c4f62
RUN      ./configure
RUN      make -j 10
RUN      make install

#install arrow
WORKDIR  /workstem
RUN      git clone https://github.com/apache/arrow.git
WORKDIR  /workstem/arrow/
RUN      git checkout a8bd1c9ce4273d53eee2c67074b86410d1db4ce1
WORKDIR  /workstem/arrow/cpp
RUN      mkdir release
COPY     Arrow_CMakeLists.txt /workstem/arrow/cpp/CMakeLists.txt
WORKDIR  release
RUN      cmake .. -DCMAKE_BUILD_TYPE=Release
RUN      make -j 10
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
RUN      conda create -y -n pykafarr python=3.7
RUN      conda install -y numpy   -n pykafarr
RUN      conda install -y pandas  -n pykafarr
RUN      conda install -y pyarrow -n pykafarr
RUN      conda install -y cython  -n pykafarr
RUN      python -V

#
## set up the ssh client
#
RUN      mkdir /root/.ssh
#COPY     authorized_keys /root/.ssh/
#RUN      chmod 600 /root/.ssh/*

#
## check out the work (the SSH keys should be sufficient for authentication)
#
WORKDIR  /workstem
RUN      git clone https://github.com/ikucan/pykafarr.git
WORKDIR  /workstem/pykafarr/src/
#RUN      source activate pykafarr
#RUN      python setup.py build_ext --inplace

#
## all third party sources are built into default prefix, so needs to be in the path
#
ENV LD_LIBRARY_PATH=/usr/local/lib

#
##install and expose the ssh service
#
RUN      mkdir /var/run/sshd
EXPOSE   22

#
## run ssh as main command.
#
CMD ["/usr/sbin/sshd", "-D"]
