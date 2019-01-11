#!/bin/bash

echo "DEBUG:>> ${DEBUG}"
sleep 2

if [ "X${DEBUG}X" == "XtrueX" ]; then
    echo ---- DEBUG MODE : starting SSH service -----
    /usr/sbin/sshd -D &
fi

echo "---- kafka intallation directory: ${CONFLUENT_HOME} ----"
#${CONFLUENT_HOME}/bin/zookeeper-server-start ${CONFLUENT_HOME}/etc/kafka/zookeeper.properties > zk.console.out 2>&1 &
${CONFLUENT_HOME}/bin/zookeeper-server-start ${CONFLUENT_HOME}/etc/kafka/zookeeper.properties &
sleep 10
${CONFLUENT_HOME}/bin/kafka-server-start ${CONFLUENT_HOME}/etc/kafka/server.properties &
sleep 30
${CONFLUENT_HOME}/bin/schema-registry-start ${CONFLUENT_HOME}/etc/schema-registry/schema-registry.properties &
sleep 10

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
