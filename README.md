### Pykafarr is a library for efficientlly streaming typed Kafka messages in Python

Pykafarr provides a fast, batching Kafka client. Messages are read on Kafka and transformed into an Arrow record batch. This is then wrapped as a Pandas data frame and returned to the Python client.

The data frame contains a column for each field of the message as defined in its Avro schema. There is an additional metadata column for the message offset. Frames are guaranteed to be homogenous - only messages from one schema will be included in a frame. If there is a message schema change the _poll_ will return early. If A,A,A,A,A,B,B,B,B,B arrive the _poll(MaxInt, MaxInt)_ will return early with 5 rows of A. The subsequent invocation will return 5 rows of B. This behaviour erodes some of the efficiencies if we typically receive messages with mixed schemas, such as A,B,A,B,A,B....

The c++ part of codebase is fully independent of Python and can be used directly from c++. In that case you are working with Apache::Arrow structures to interface with Kafka.

#### Performance:
While no time has been spent on optimisations it is already reasonably performant. Pykafarr can read, parse and package into Pandas 100 000 small messages in under 250ms. Note that these numbers are indicative and derived from single-machine tests on an underwhelming old laptop in a single-host setup. Reading from a remote kafka topic would change the numbers depending on your network latency.

#### Status:
Functionality is still underdeveloped, however what is there is thought to work without known issues. Please add to the issue register or raise a pull request if you have anything specific in mind.
<br/><br/>So far only tested with Python3 on Ubuntu but no known reason not to try other platforms.
<br/><br/>_Valgrind_ reports no memory leaks.

#### Example:

```python
import pykafarr

p = pykafarr.listener('kafka_server:9092',
                      'client_group_id',
                      ['GBPUSD_PRICE_TOPIC', 'GBPUSD_TRADE_TOPIC'],
                      'http://schema_reg:8081/')

while -2 < -1:
    message_type, frame = p.poll(num_messages = 1000, max_time = 30000)
    # 1. message_type: name of the [fully-qualified avro] message schema
    # 2. frame: pandas data frame, one row per mesage, columns as defined in the message Avro schema
    print(message_type)
    print(frame)
```

#### Dependencies:
There are a few:
- apache rdkafka (c & c++)
- apache avro (c & c++)
- apache serdes (c & c++)
- apache arrow (c & c++)
- apache pyarrow (python)

Other things you will need: g++, boost, jansson (JSON parser), snappy and curl dev libs.

A farily isolated developmnet environment is described in the _dev_env_ docker container. If you have any issues installing dependencies I would recommend the Dockerfile as a guide on how to get it built and installed.

The only way this was tested is by building the above rather than installing distros' official versions. While the latter should be ok there are more variables in play.

#### To install
Ensure you have the dependencies as above and then run:

```
pip install -i https://test.pypi.org/simple/ pykafarr
```

#### To build
Navigate to the src/py directory and run:

```
git clone https://github.com/ikucan/pykafarr.git
cd pykafarr/src
python setup.sh build_ext -inplace
```

#### Docker Containers
Two docker containers are provied. The development and the runtime. To build the runtime image the developmnet image must be built first.

##### Development image
Provides a complete development environment with all dependencies built and installed in order to build and run *pykafarr*. It is fairly minimal but takes some time to build (~10mins) due to building all the dependencies.

##### Runtime image
Runtime image can be used as a basis for creating python applications whcih use pykafarr. The idea is that your docker containers containing apps could simply use the pykafarr image as the base.

#### To Use
Look at the `tst.py` file in `src/py` or `tst2.cpp` in `src/cpp`

#### Not [yet] supported
- Message sending
- Keyed messages
- All Avro primitive types. Only bool, int, long, float, double and string currentlly supported.
- Complex schemas. Schemas where children are not primitive types.
- Untyped (schema-less) messages

Those will be added some time in the near future, the first priority has been to get something working sensibly released. Please register an issue if you have a preference.

#### Issues/Limitations
- More testing needs to be done around reading from multiple topics and multiple partitions.
- Come up with a configurable model of hos much kafka metadata to return (offset, partition, topic, etc...). In an idealised model none of this would be needed but in practice it is often desirable. 
- OS. This has only been tested on Ubuntu 18.xx. There is no reason any other Linux versions and MacOS should be an issue. Windows howver might be a different story.
- ~~The polling timeout is currentlly not working correctly. The logic needs to be clearer. The issue is slightly complicated by the fact that there is also the 'client catcup time' which presumably should not be included as polling time or perhaps needs to be mandated separtely (andother parameter?). Suggestions welcome but the next change will be to this:~~
  This has been fixed. ~~Still testing but almost ready to be checked in.~~ The algorithm is as was suggested initially.

```
  poll(n_msgs, timeout):
    wait until kafka client caught up
    set message_count, polling_time to 0
    while message_count < n_msgs && polling_time < timeout
       receive and process a message
       update message_count, polling_time
       
```
- ~~More testing needs to be done around messages with mixed schemas.~~ This has been done. Works as expected. No messages are lost upon schema change (a poped message with a new schema is pushed back and succesfully popped on next _poll_).


#### References
- [Apache Arrow C++](https://arrow.apache.org/docs/cpp/index.html "C++ API docs")
- [Cython & Apache Arrow](https://arrow.apache.org/docs/python/extending.html "Apache Arrow API for usage with Cython")
- [Serdes](https://github.com/confluentinc/libserdes/blob/master/src-cpp/Serdes.cpp "[De]Serialisation API for Kafka")
- [Apache Avro](https://avro.apache.org/docs/1.6.1/api/cpp/html/index.html "C++ API docs")
