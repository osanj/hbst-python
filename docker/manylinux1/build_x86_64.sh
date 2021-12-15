#!/bin/bash

REPO=$1
PY=$2

if [ "$PY" == "36" ]; then
    PYTHON_EXEC="/opt/python/cp36-cp36m/bin/python3"
elif [ "$PY" == "37" ]; then
    PYTHON_EXEC="/opt/python/cp37-cp37m/bin/python3"
elif [ "$PY" == "38" ]; then
    PYTHON_EXEC="/opt/python/cp38-cp38/bin/python3"
elif [ "$PY" == "39" ]; then
    PYTHON_EXEC="/opt/python/cp39-cp39/bin/python3"
else
    echo "python $PYTHON is not supported"
    exit 1
fi

cd $REPO
$PYTHON_EXEC setup.py sdist bdist_wheel
auditwheel repair dist/hbst_python-*cp$PY-*-linux_x86_64.whl
