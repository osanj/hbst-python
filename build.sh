#!/bin/bash

PLAT_X86_64="x86_64"
PYTHON_3_6="3.6"
PYTHON_3_7="3.7"
PYTHON_3_8="3.8"
PYTHON_3_9="3.9"

PLAT=$1
PYTHON=$2

if [ -z "$PLAT" ] || [ -z "$PYTHON" ]; then
    echo "usage: $0 platform python-version"
    echo "example: $0 $PLAT_X86_64 $PYTHON_3_6"
    exit 1
fi

if [ "$PLAT" == "$PLAT_X86_64" ]; then
    if [ "$PYTHON" == "$PYTHON_3_6" ]; then
        PYTHON_EXEC="/opt/python/cp36-cp36m/bin/python3"
    elif [ "$PYTHON" == "$PYTHON_3_7" ]; then
        PYTHON_EXEC="/opt/python/cp37-cp37m/bin/python3"
    elif [ "$PYTHON" == "$PYTHON_3_8" ]; then
        PYTHON_EXEC="/opt/python/cp38-cp38/bin/python3"
    elif [ "$PYTHON" == "$PYTHON_3_9" ]; then
        PYTHON_EXEC="/opt/python/cp39-cp39/bin/python3"
    else
        echo "python $PYTHON is not supported for platform $PLAT"
        exit 1
    fi

    PYTHON_EXEC=$PYTHON_EXEC docker-compose -f build-compose.yml up


else
    echo "platform $PLAT is not supported"
    exit 1
fi