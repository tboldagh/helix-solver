#!/bin/bash

clean=false
if [ "$#" -gt 0 ]; then
    clean_string="clean"
    if [ "$1" = "$clean_string" ]; then
        clean=true
    fi
fi

if [ "$clean" = true ]; then
    rm -rf build/*
fi

if [ "${SETVARS_COMPLETED:-}" != "1" ]; then
    source /opt/intel/oneapi/setvars.sh
fi

cd build
cmake ../CMakeLists.txt
make