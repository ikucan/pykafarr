### Pykafarr is a library for efficientlly reading typed kafka messages in python.

Messages are read using rdkafka client and transformed into an arrow record batch. This is then wrapped as a pandas data frame an returned to the python client.

It should be handy for reading time series data off a Kafka topic into a Pandas frame from python. It is very performant - should read and parse 100 000 small messages in under 250ms.

The c++ codebase is independent of python and can be used directly from c++ (see the src/cpp example). In that case you are working with Apache::Arrow structures. (the Makefile in src/cpp shows how to build)

#### To build
Navigate to the src/py directory and run:

```
python setup.sh build_ext -inplace
```

#### Dependencies:
- apache rdkafka (c/c++)
- apache avro (c/c++)
- apache serdes (c/c++)
- apache arrow (c/c++)
- apache arrow (python)

#### Docker Build:
  - build the development docker container
  - start interactively (or ssh if you prefer)
  - `cd /workstem/pykafarr/src/py`
  - run `python setup.py build_ext --inplace`

The container provides an uncontaminated environment and and example of what is required to build Pykafarr. 

#### To Use
Look at the `tst.py` file in `src/py` or `tst2.cpp` in `src/cpp`

#### Not [yet] supported
- Message sending
- Keyed messages
- Untyped (Schema-less) messages
- a pip/conda/wheel installation (with dependency checking)

I am quite open to adding all of those. Please register an issue.