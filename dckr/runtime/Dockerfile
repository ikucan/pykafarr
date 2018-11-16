FROM ubuntu:18.10
SHELL ["/bin/bash", "-c"]

# configure apt-get repos
RUN  apt-get update --fix-missing
RUN  apt-get install -yq vim tree htop tmux net-tools iputils-ping 
RUN  apt-get install -yq wget curl
RUN  apt-get install -yq gcc g++
RUN  apt-get install -yq libboost-all-dev libjansson-dev libsnappy-dev
RUN  apt-get install -yq git

# --------------------------------------------------------------------------------------------
# --------------------------------------------------------------------------------------------
# copy all the 3rdf party binary dependencies from the pykafarr dev container
# todo:>> in theory, they could be standard packages but not sure this can be done in all cases
# --------------------------------------------------------------------------------------------
# --------------------------------------------------------------------------------------------
RUN     mkdir -p /usr/local/lib
WORKDIR /usr/local/lib
COPY   --from=ikucan/pykafarr_dev:1.0.0 /usr/local/lib/*.a /usr/local/lib/
COPY   --from=ikucan/pykafarr_dev:1.0.0 /usr/local/lib/libarrow.so.12.0.0 /usr/local/lib/
RUN    ln -s libarrow.so.12.0.0 libarrow.so.12
RUN    ln -s libarrow.so.12 libarrow.so
COPY   --from=ikucan/pykafarr_dev:1.0.0 /usr/local/lib/libavro.so.23.0.0 /usr/local/lib/
COPY   --from=ikucan/pykafarr_dev:1.0.0 /usr/local/lib/libavrocpp.so.1.9.0-SNAPSHOT.0 /usr/local/lib/
RUN    ln -s libavro.so.23.0.0 libavro.so 
RUN    ln -s libavrocpp.so.1.9.0-SNAPSHOT.0 libavrocpp.so
COPY   --from=ikucan/pykafarr_dev:1.0.0 /usr/local/lib/librdkafka.so.1 /usr/local/lib/
COPY   --from=ikucan/pykafarr_dev:1.0.0 /usr/local/lib/librdkafka++.so.1 /usr/local/lib/
RUN    ln -s librdkafka.so.1 librdkafka.so
RUN    ln -s librdkafka++.so.1 librdkafka++.so
COPY   --from=ikucan/pykafarr_dev:1.0.0 /usr/local/lib/libserdes.so.1 /usr/local/lib/
COPY   --from=ikucan/pykafarr_dev:1.0.0 /usr/local/lib/libserdes++.so.1 /usr/local/lib/
RUN    ln -s libserdes.so.1 libserdes.so
RUN    ln -s libserdes++.so.1 libserdes++.so

ENV     LD_LIBRARY_PATH=/usr/local/lib

RUN     mkdir -p /usr/local/include
WORKDIR /usr/local/include
COPY   --from=ikucan/pykafarr_dev:1.0.0 /usr/local/include/ /usr/local/include/


# --------------------------------------------------------------------------------------------
# --------------------------------------------------------------------------------------------
# set up python
# --------------------------------------------------------------------------------------------
# --------------------------------------------------------------------------------------------

RUN      mkdir -p /workstem/
RUN      mkdir -p /opt/
WORKDIR  /workstem
RUN      wget https://repo.continuum.io/miniconda/Miniconda3-latest-Linux-x86_64.sh
RUN      /bin/bash /workstem/Miniconda3-latest-Linux-x86_64.sh -b -p /opt/miniconda
RUN      rm /workstem/Miniconda3-latest-Linux-x86_64.sh
ENV      PATH=/opt/miniconda/bin/:$PATH
RUN      conda install -y numpy
RUN      conda install -y pandas
RUN      conda install -y pyarrow
RUN      conda install -y cython
RUN      pip install --upgrade pip
RUN      pip install -i https://test.pypi.org/simple/ pykafarr --no-clean --no-cache-dir

CMD      ["echo", "--- this is a virtual container. provides a base for pykafarr python apps. not to be run without overriding ----"]
