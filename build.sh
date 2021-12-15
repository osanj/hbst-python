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
        PY="36"
    elif [ "$PYTHON" == "$PYTHON_3_7" ]; then
        PY="37"
    elif [ "$PYTHON" == "$PYTHON_3_8" ]; then
        PY="38"
    elif [ "$PYTHON" == "$PYTHON_3_9" ]; then
        PY="39"
    else
        echo "python $PYTHON is not supported for platform $PLAT"
        exit 1
    fi

    PY=$PY docker-compose -f build-compose.yml run linux_x86_64

else
    echo "platform $PLAT is not supported"
    exit 1
fi
