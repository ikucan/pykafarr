# -------------------------------------------------------
# NOTE: when run, this container starts an ssh deamon,
#       use with caution
# -------------------------------------------------------

FROM ikucan/pykafarr_dev:1.0.0

# use bash instead of sh
SHELL ["/bin/bash", "-c"]

#
## add conda build package(s)
#
RUN conda install -y conda-build
RUN conda install anaconda-client

COPY  build.sh /workstem/build.sh
RUN   chmod u+x /workstem/build.sh
CMD   ["/workstem/build.sh"]
