#!/bin/bash

clean=false
if [ "$#" -gt 0 ]; then
    clean_string="clean"
    if [ "$1" = "$clean_string" ]; then
        clean=true
    fi
fi

# Create build directory if it doesn't exist
if [ ! -d "build" ]; then
    mkdir "build"
fi

if [ "$clean" = true ]; then
    rm -rf build/*
fi

if [ "${SETVARS_COMPLETED:-}" != "1" ]; then
    source ./source_setvars.sh
fi

if [[ -z "${ROOTSYS}" ]]; then
    source lib/root/bin/thisroot.sh
fi

cd build
cmake ..
make
