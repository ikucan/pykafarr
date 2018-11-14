rm -Rf build dist pykafarr.egg-info *.so *.cpp
python setup.py build_ext --inplace
python setup.py sdist
twine upload --repository-url https://test.pypi.org/legacy/ dist/*
