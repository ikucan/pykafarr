### Pykafarr is a library for efficientlly reading typed kafka messages in python.

Messages are read using rdkafka client and transformed into an arrow record batch. This is then wrapped as a pandas data frame an returned to the python client.

The pandas data frame has the colums as defined in the Avro schema for the message. There is a meta-data column for the message offset. One row per message. Frames are guaranteed to be homogenous - only messages from one schema will be included in a frame. If there is a message schema change the _poll_ will return early. Eg: if messages A,A,A,A,A,B,B,B,B,B arrive the _poll_ will return early with 5 rows of A and 5 rows of B upon the subsequent all. Clearly, this behaviour erodes some of the efficiencies if we typically receive messages with mixed schemas, such as A,B,A,B,A,B....

It should be useful for reading time series data off a Kafka topic into a Pandas frame from python. It is very performant - should read and parse 100 000 small messages in under 250ms.

The c++ codebase is independent of python and can be used directly from c++ (see example in the cpp directory). In that case you are working with Apache::Arrow structures to interface with Kafka. (the Makefile in src/cpp shows how to build).

#### Example:

```
import pykafarr

p = pykafarr.listener('kafka_server:9092',
                      'client_group_id',
                      ['GBPUSD_PRICE_TOPIC', 'GBPUSD_TRADE_TOPIC'],
                      'http://schema_reg:8081/')

while -2 < -1:
    frame = p.poll(num_messages = 1000, max_time = 30000)
    # pandas data frame, one row per mesage, columns as defined in the message Avro schema
    print(frame)
```

#### Dependencies:
- apache rdkafka (c & c++)
- apache avro (c & c++)
- apache serdes (c & c++)
- apache arrow (c & c++)
- apache pyarrow (python)

Other things you will need: g++, boost, jansson (JSON parser), snappy and curl dev libs.

A farily isolated developmnet environment is described in the _dev_env_ docker container.

The only way this was tested is by building the above rather than installing distros' official versions. While the latter should be ok there are more variables in play.

#### To install
Ensure you have the dependencies as above and then run:

```
pip install -i https://test.pypi.org/simple/ pykafarr
```

#### To build
Navigate to the src/py directory and run:

```
python setup.sh build_ext -inplace
```

#### Docker Containers
Two docker containers are provied. The development and the runtime. To build the runtime image the developmnet image must be built first.

##### Development image
Provides a complete development environment with all dependencies built and installed in order to build and run *pykafarr*. It is fairly minimal but takes some time to build (~10mins) due to building all the dependencies.

##### Development image
Runtime image can be used as a basis for creating python applications whcih use pykafarr. The idea is that your docker containers would simply use the pykafarr as an example ut 

#### To Use
Look at the `tst.py` file in `src/py` or `tst2.cpp` in `src/cpp`

#### Not [yet] supported
- Message sending
- Keyed messages
- Untyped (Schema-less) messages

Those will be added some time in the near future. Please register an issue if you have a preference.

#### Issues/Limitations
- The polling timeout is currentlly not working correctly. The logic needs to be clearer. The issue is slightly complicated by the fact that there is also the 'client catcup time' which presumably should not be included as polling time or perhaps needs to be mandated separtely (andother parameter?). Suggestions welcome but the next change will be to this:

```
  poll(n_msgs, timeout):
    wait until kafka client caught up
    set message_count, polling_time to 0
    while message_count < n_msgs && polling_time < timeout
       receive and process a message
       update message_count, polling_time
       
```
- More testing needs to be done around messages with mixed schemas.
- OS. This has only been tested on Ubuntu 18.xx. There is no reason any other Linux versions and MacOS should be an issue. Windows howver might be a different story.
