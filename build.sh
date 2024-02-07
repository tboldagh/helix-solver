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
    echo "Environment not prepared correctly. \$SETVARS_COMPLETED not set. Ensure you have run prepare_environment.sh"
    exit 1
fi

if [[ -z "${ROOTSYS}" ]]; then
    echo "Environment not prepared correctly. \$ROOTSYS not set. Ensure you have run prepare_environment.sh"
    exit 1
fi

cd build
cmake ..
make
