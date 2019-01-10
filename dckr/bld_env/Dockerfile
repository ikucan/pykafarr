# -------------------------------------------------------
# NOTE: when run, this container starts an ssh deamon,
#       use with caution
# -------------------------------------------------------

FROM ikucan/pykafarr_dev:1.0.0

# use bash instead of sh
SHELL ["/bin/bash", "-c"]

#
## check out the work (the SSH keys should be sufficient for authentication)
#
#WORKDIR  /workstem
#RUN      git clone https://github.com/ikucan/pykafarr.git
#WORKDIR  /workstem/pykafarr/src/
#
## 
#
#RUN      python setup.py build_ext --inplace

#
## add conda build package(s)
#
RUN conda install -y conda-build
RUN conda install anaconda-client

COPY  build.sh /workstem/build.sh
RUN   chmod u+x /workstem/build.sh
CMD   ["/workstem/build.sh"]
