#!/bin/bash

export ARROW_LIBS_PATH=/usr/local/lib
export SERDES_LIBS_PATH=/usr/local/lib
export KAFKA_LIBS_PATH=/usr/local/lib
export AVRO_LIBS_PATH=/usr/local/lib
#export JANSSON_LIBS_PATH=/usr/lib/x86_64-linux-gnu/
#export LIBCURL_LIBS_PATH=/usr/lib/x86_64-linux-gnu/

# ok, this is a bit mucky... but! copy all the libs used to build into

echo "+++++++++++++++++++++++++++++++++++++++++"
echo  "copying dependency libs to ${PREFIX}"
cp -d $SERDES_LIBS_PATH/libserdes*    $PREFIX/lib
cp -d $KAFKA_LIBS_PATH/librdkafka*    $PREFIX/lib
cp -d $AVRO_LIBS_PATH/libavro*        $PREFIX/lib
#cp -d $JANSSON_LIBS_PATH/libjansson*  $PREFIX/lib
#cp -d $LIBCURL_LIBS_PATH/libcurl*     $PREFIX/lib
echo "+++++++++++++++++++++++++++++++++++++++++"

#exit 1

${PYTHON} setup.py install || exit 1

#exit 1

