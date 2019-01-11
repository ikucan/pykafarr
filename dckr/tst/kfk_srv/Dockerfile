FROM ubuntu:18.10
SHELL ["/bin/bash", "-c"]

# configure apt-get repos
RUN apt-get update --fix-missing

# install required env
RUN apt-get install -yq wget
RUN apt-get install -yq vim tree htop curl
RUN  apt-get install -yq ssh

#
##  java, the WGET might be fragile but have a look here for more info:
##  https://stackoverflow.com/questions/10268583/downloading-java-jdk-on-linux-via-wget-is-shown-license-page-instead
#
RUN      mkdir -p /opt
WORKDIR  /opt
RUN      wget --no-cookies --no-check-certificate --header "Cookie: gpw_e24=http%3a%2F%2Fwww.oracle.com%2Ftechnetwork%2Fjava%2Fjavase%2Fdownloads%2Fjdk8-downloads-2133151.html; oraclelicense=accept-securebackup-cookie;" "https://download.oracle.com/otn-pub/java/jdk/8u191-b12/2787e4a523244c269598db4e85c51e0c/jdk-8u191-linux-x64.tar.gz"
RUN      tar -xzvf jdk-8u191-linux-x64.tar.gz
RUN      rm jdk-8u191-linux-x64.tar.gz
ENV      JAVA_HOME=/opt/jdk1.8.0_191

#
## kafka++ : https://docs.confluent.io/current/installation/installing_cp/zip-tar.html
#
WORKDIR  /opt
RUN      wget http://packages.confluent.io/archive/5.1/confluent-5.1.0-2.11.tar.gz
RUN      tar -xzvf confluent-5.1.0-2.11.tar.gz
RUN      rm confluent-5.1.0-2.11.tar.gz
ENV      CONFLUENT_HOME=/opt/confluent-5.1.0
COPY     start_kfk.sh /opt/start_kfk.sh

#
## the wait script
#
WORKDIR  /opt
RUN      wget https://raw.githubusercontent.com/vishnubob/wait-for-it/master/wait-for-it.sh
RUN      chmod u+x wait-for-it.sh      

# fix up the path
ENV PATH=${JAVA_HOME}/bin:${CONFLUENT_HOME}/bin:${PATH}

#
## config the sshd
#
# RUN  mkdir /var/run/sshd
# RUN      mkdir /root/.ssh
# COPY     id_rsa.pub /root/.ssh/authorized_keys
# RUN      chmod 600 /root/.ssh/*

#
## custom script to start zookeeper, kafka and registry services
#
CMD ["/opt/start_kfk.sh"]