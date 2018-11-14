### Pykafarr is a library for efficientlly reading typed kafka messages in python.

Messages are read using rdkafka client and transformed into an arrow record batch. This is then wrapped as a pandas data frame an returned to the python client.

It should be useful for reading time series data off a Kafka topic into a Pandas frame from python. It is very performant - should read and parse 100 000 small messages in under 250ms.

The c++ codebase is independent of python and can be used directly from c++ (see example in the cpp directory). In that case you are working with Apache::Arrow structures to interface with Kafka. (the Makefile in src/cpp shows how to build).

#### Dependencies:
- apache rdkafka (c & c++)
- apache avro (c & c++)
- apache serdes (c & c++)
- apache arrow (c & c++)
- apache arrow (python)

The only way this was tested is by building the above rather than installing distros' official versions. In theory the latter should be ok.

#### To install

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

I am quite open to adding all of those. Please register an issue.

#### Issues/Limitations
- OS. This has only been tested on Ubuntu 18.xx. There is no reason any other Linux versions and MacOS should be an issue. Windows howver might be a different story.
- the PIP installation is rudimentary and requires a local build which in terms has a few dependencies (see above...)
