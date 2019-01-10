[![Build Status](https://travis-ci.org/ikucan/pykafarr.svg?branch=master)](https://travis-ci.org/ikucan/pykafarr)

### Pykafarr is a library for efficientlly streaming Avro-typed Kafka messages in Python

Pykafarr provides a fast, batching Kafka client. Messages are read on Kafka and transformed into an Arrow record batch. This is then wrapped as a Pandas data frame and returned to the Python client. Several messages can be returned as a result of a single _"poll"_ to Kafka. As a result the overhead costs of the Python VM are minimised. Data is also returned in a Python-friendly format, arguably one of the preferred formats.

Returned data frame contains a column for each field of the message as defined in its Avro schema. There is an additional metadata column for the message offset. Frames are guaranteed to be homogenous - only messages from one schema will be included in a frame. If there is a message schema change the _poll_ will return early. If
<a href="https://www.codecogs.com/eqnedit.php?latex=\inline&space;\{A_1,A_2,...,A_m,B_1,...,B_n\}" target="_blank"><img src="https://latex.codecogs.com/gif.latex?\inline&space;\{A_1,A_2,...,A_m,B_1,...,B_n\}" title="\{A_1,A_2,...,A_m,B_1,...,B_n\}" /></a>
arrive the _poll(MaxInt, MaxInt)_ will return early with _m_ rows 
<a href="https://www.codecogs.com/eqnedit.php?latex=\inline&space;\{A_1,A_2,...,A_m\}" target="_blank"><img src="https://latex.codecogs.com/gif.latex?\inline&space;\{A_1,A_2,...,A_m\}" title="\{A_1,A_2,...,A_m\}" /></a>
. Subsequent invocation will return _n_ rows 
<a href="https://www.codecogs.com/eqnedit.php?latex=\inline&space;\{B_1,...,B_n\}" target="_blank"><img src="https://latex.codecogs.com/gif.latex?\inline&space;\{B_1,...,B_n\}" title="\{B_1,...,B_n\}" /></a>
. This behaviour erodes some of the efficiencies if we typically receive messages with mixed schemas, such as
<a href="https://www.codecogs.com/eqnedit.php?latex=\inline&space;\{A_1,&space;B_1,&space;A_2,&space;B_2,&space;A_3,&space;B_3...\}" target="_blank"><img src="https://latex.codecogs.com/gif.latex?\inline&space;\{A_1,&space;B_1,&space;A_2,&space;B_2,&space;A_3,&space;B_3...\}" title="\{A_1, B_1, A_2, B_2, A_3, B_3...\}" /></a>.

Producing is trivial. You simply set up your data frame where each row is assuemd to be a message to be sent on Kafka. The frame is sent accompanied by the desired message type. If the data frame is a superset of the message type it will be sent, provided equally named fields are type-compatible.

In both cases advantages are simplicity and performance. The boilerplate mechanics of deserialisation and serialisation from AVRO to Pandas and the reverse are hidden in the library. Because it is implemented in C++ it has better performance and scalability characteristcs when compared to Python.

The C++ implemenation is fully independent and unaware of Python and can be used directly. In that case you are working with [Apache Arrow](https://arrow.apache.org/docs/cpp/index.html) structures to interface with Kafka.

#### Background:
The original idea is stolen from here: https://github.com/ah-/kafka_arrow. This is a c++ rewrite and an extension of that work.

#### Performance:
While no time has been spent on optimisations Pykafarr is already quite performant. It can read, parse and package into Pandas 100 000 small messages in under 250ms. Note that these numbers are indicative and derived from single-machine tests on an underwhelming old laptop in a single-host setup. Reading from a remote kafka topic would change the numbers depending on your network latency.

#### Status:
Functionality is still underdeveloped, however what is there is thought to work without known issues. Please add to the issue register or raise a pull request if you have anything specific in mind.
-So far only tested with Python 3.7 on Ubuntu 18.10. There is no known reason not to try Pykafarr on other platforms. All feedback welcome. Package genration is now dockerised using Conda so this should be easy to extend.
-CI set up on [_Travis_] (https://travis-ci.org/)
-_Valgrind_ reports no memory leaks.
<hr/>

### Getting and installing:
There are quite a few dependencies and are listed individually below. The installation has so far only been tested with Python 3.7 on Ubuntu 18.10.

#### Option 1: Docker (trivial)
Extend the ```ikucan/pykafarr_test_install:1.0.0``` container. Pykafarr is installed in the conda _base_ environment which is fully set up with all the dependencies.

#### Option 2: Using Conda (painless) 
Prerequisites:
 ```
 apt-get install -yq libjansson-dev
 apt-get install -yq libcurl4-gnutls-dev
 ```
Recommeded:
Most of the third party depencencies are bundled with the conda package, so these could overwrite your existing environment. Unlikely to cause problems but to be sure, create a separate virtual environement.
```
conda create -n new_pykafarr_env python=3.7
```
Installation:
```
conda install -c iztok pykafarr
```
The conda package includes all the dependencies apart form jansson and curl. It is however conceivable you could get a version conflict if some of those already exist on your system. Let me know if this happens and I will try to fix. Better yet, submit a pull request if you figure it out.

#### Option 3: Using Pip (advanced :)
Pykafarr module without any dependencies is available via Pip. Take care of the dependencies listed below and then run:
```
pip install -i https://test.pypi.org/simple/ pykafarr
```
<hr/>

### Using pykafarr (example):

Message receipt. Wait for a maximum number of messages ro maximum amount of time to receive. Once either is reached, return with a Pandas frame, each row a message, each column a field as defined by the Avro schema for the message type. This is all fairly intuitive.

```python
import pykafarr

lstnr = pykafarr.listener('kafka_server:9092',
                          'client_group_id',
                          ['GBPUSD_PRICE_TOPIC', 'GBPUSD_TRADE_TOPIC'],
                          'http://schema_reg:8081/')

while -2 < -1:
    message_type, frame = lstnr.poll(num_messages = 1000, max_time = 30000)
    # 1. message_type: name of the [fully-qualified avro] message schema
    # 2. frame: pandas data frame, one row per mesage, columns as defined in the message Avro schema
    print(message_type)
    print(frame)
```
To send a Pandas data frame serialized with an avro schema, ensure it has all the columns required by the avro schema (<a href="https://www.codecogs.com/eqnedit.php?latex=\inline&space;Pandas\&space;Columns&space;\supseteq&space;Avro\&space;Columns" target="_blank"><img src="https://latex.codecogs.com/gif.latex?\inline&space;Pandas\&space;Columns&space;\supseteq&space;Avro\&space;Columns" title="Pandas\ Columns \supseteq Avro\ Columns" /></a>). All the names in the Avro schema for the message must be present in the Pandas data frame. All the types must be coerce-able. More on that below.
```python
# get the pandas data frame
new_orders = generate_orders()

prod = pykafarr.producer('kfk1:9092 kfk2:9092', 'http://kfk1:8081')

# 1. schema name. this will be looked up from the schema registry
# 2. the data
# 3. target topic
prod.send('avros.broker.Order', new_orders, 'order_topic')
```

#### A note on type conversion:
Upon message receipt, explicit type conversion is not needed default type conversion seems sufficient. Native types are generated. (In the future, it may be desirable to set a "numpy" flag to generate numpy types into nuympy arrays instead.)

When sending however, seemingly compatible types, such as Python int and AVRO int are actually incompatible. Sending a Python ```int``` column as an to an AVRO ```int``` field actually resluts in an```int64``` to ```int32``` type conversion. Currentlly this generates an error. In order to avoid risky coercion  we need to fix types in the pandas data frame more explicitly where necessary. Numpy type conversion functions such as numpy.int32 etc work really well for this purpose.

In order to send a frame with an Avr schema:
```JSON
{"subject":"avros.pricing.Tick","version":1,"id":1,"schema":"{\"type\":\"record\",\"name\":\"Tick\",\"namespace\":\"avros.pricing.ig\",\"fields\":[{\"name\":\"inst\",\"type\":\"string\"},{\"name\":\"t\",\"type\":\"long\"},{\"name\":\"dt\",\"type\":\"int\"},{\"name\":\"bid\",\"type\":\"float\"},{\"name\":\"ask\",\"type\":\"float\"}]}"}
```
you can enforce column types to be more compatible with C/C++ primitive types like this:
```python
def gen_ticks(n):
  instr = ['GBPUSD'] * n
  tms   = np.array(list(np.int64(time()*1000) for x in range(n)))
  dt    = np.array(list(np.int32(r.randint(0,150)) for x in range(n)))
  mid   = np.array(list(np.float32((125000 + r.randint(-100, 100))/100000) for x in range(n)))
  sprd  = np.array(list(np.float32(r.randint(1, 10)/100000) for x in range(n)))
  bid   = mid - sprd
  ask   = mid + sprd
  return pd.DataFrame({'inst':instr, 't':tms, 'dt':dt, 'bid':bid, 'ask':ask})
```
(well, as appropriate for your data source...).

At some point there will be an option to allow risky type conversion (e.g. from int64 to int 32) at users' discretion. This would then allow a Pandas dataframe containing a Python ```int``` column (```int64``` by the time it is in Arrow) to serialise to the Avro ```int``` (```int32```) field.

### Dependencies:
There are a few:
- apache rdkafka (c & c++)
- apache avro (c & c++)
- apache serdes (c & c++)
- apache arrow/pyarrow (c & c++)
- apache pyarrow (python)

Other things you will need: g++, boost, jansson (JSON parser), snappy and curl dev libs.

A farily isolated developmnet environment is described in the _dev_env_ docker container. If you have any issues installing dependencies I would recommend the Dockerfile as a guide on how to get it built and installed.

The only way this was tested is by building the above rather than installing distros' official versions. While the latter should be ok there are more variables in play.

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

##### Build image
<tbd>
  
##### Runtime image
<tbd>
Runtime image can be used as a basis for creating python applications whcih use pykafarr. The idea is that your docker containers containing apps could simply use the pykafarr image as the base.

#### To Use
Look at the python and c++ examples in the _test_ directory.

#### Not [yet] supported
- Keyed messages
- All Avro primitive types. Only bool, int, long, float, double and string currentlly supported.
- Complex schemas. Schemas where children are not primitive types.
- Untyped (schema-less) messages

Those will be added some time in the near future, the first priority has been to get something working sensibly released. Please register an issue if you have a preference.

#### Issues/Limitations
- More testing needs to be done around reading from multiple topics and multiple partitions.
- Come up with a configurable model of how much kafka metadata to return (offset, partition, topic, etc...). In an idealised model none of this would be needed but in practice it is often desirable. 
- OS. This has only been tested on Ubuntu 18.xx. There is no reason any other Linux versions and MacOS should be an issue. Windows howver might be a different story.

#### References
- [Apache Arrow C++](https://arrow.apache.org/docs/cpp/index.html "C++ API docs")
- [Cython & Apache Arrow](https://arrow.apache.org/docs/python/extending.html "Apache Arrow API for usage with Cython")
- [Serdes](https://github.com/confluentinc/libserdes/blob/master/src-cpp/Serdes.cpp "[De]Serialisation API for Kafka")
- [Apache Avro](https://avro.apache.org/docs/1.6.1/api/cpp/html/index.html "C++ API docs")
