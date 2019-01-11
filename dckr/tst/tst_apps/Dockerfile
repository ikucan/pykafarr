# -------------------------------------------------------
#
# -------------------------------------------------------
FROM ikucan/pykafarr_runtime:1.0.0

# use bash instead of sh
SHELL ["/bin/bash", "-c"]

RUN   apt-get install -yq wget

WORKDIR  /opt
WORKDIR  /workstem
RUN      wget https://raw.githubusercontent.com/vishnubob/wait-for-it/master/wait-for-it.sh
RUN      chmod u+x wait-for-it.sh      
COPY     app1.py /workstem/app1.py


