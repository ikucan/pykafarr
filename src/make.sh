rm -Rf build dist pykafarr.egg-info *.so *.cpp
python setup.py build_ext --inplace
