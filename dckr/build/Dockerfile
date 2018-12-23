# -------------------------------------------------------
# NOTE: when run, this container starts an ssh deamon,
#       use with caution
# -------------------------------------------------------

FROM iztokkucan/pykafarr_dev2:1.0.0

# use bash instead of sh
SHELL ["/bin/bash", "-c"]

#
## check out the work (the SSH keys should be sufficient for authentication)
#
WORKDIR  /workstem
RUN      git clone https://github.com/ikucan/pykafarr.git
WORKDIR  /workstem/pykafarr/src/
RUN      python setup.py build_ext --inplace

