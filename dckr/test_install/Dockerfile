# -------------------------------------------------------
# -------------------------------------------------------

FROM ubuntu:18.10

# use bash instead of sh
SHELL ["/bin/bash", "-c"]

RUN apt-get update --fix-missing
#RUN apt-get install -yq apt-utils
RUN   apt-get install -yq wget


WORKDIR  /workstem
RUN      wget https://repo.continuum.io/miniconda/Miniconda3-latest-Linux-x86_64.sh
RUN      /bin/bash /workstem/Miniconda3-latest-Linux-x86_64.sh -b -p /opt/miniconda
RUN      rm /workstem/Miniconda3-latest-Linux-x86_64.sh
ENV      PATH=/opt/miniconda/bin/:$PATH
