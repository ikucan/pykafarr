#!/bin/bash

if [ "X${DEBUG}X" == "XtrueX" ]; then
    echo ---- DEBUG MODE : starting SSH service -----
    /usr/sbin/sshd -D &
fi

echo "---- kafka intallation directory: ${CONFLUENT_HOME} ----"
#${CONFLUENT_HOME}/bin/zookeeper-server-start ${CONFLUENT_HOME}/etc/kafka/zookeeper.properties > zk.console.out 2>&1 &
${CONFLUENT_HOME}/bin/zookeeper-server-start ${CONFLUENT_HOME}/etc/kafka/zookeeper.properties &
echo "wait for zk to start up on port 2181"
/opt/wait-for-it.sh -t 60 0.0.0.0:2181 || exit -1

${CONFLUENT_HOME}/bin/kafka-server-start ${CONFLUENT_HOME}/etc/kafka/server.properties &
echo "wait for kafka to start up on port 9092"
/opt/wait-for-it.sh -t 60 0.0.0.0:9092 || exit -1

${CONFLUENT_HOME}/bin/schema-registry-start ${CONFLUENT_HOME}/etc/schema-registry/schema-registry.properties &
echo "wait for schema registry to start up on port 8081"
/opt/wait-for-it.sh -t 60 0.0.0.0:8081 || exit -1

if [ "X${DEBUG}X" == "XtrueX" ]; then
    echo "-----------------------------------------------"
    echo "-----------------------------------------------"
    echo "---- WARNING ----"
    echo "running in DEBUG mode. SSH service is on. "
    echo "-----------------------------------------------"
    echo "-----------------------------------------------"
fi
#
#
# don't exit
while [ 1 -lt 2 ]; do sleep 1; done
