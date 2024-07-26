#!/bin/bash

ln -s ../lib lib

echo y | sh 3pp/codeplay/oneapi-for-nvidia-gpus-2023.2.1-cuda-12.0-linux.sh

source /opt/intel/oneapi/setvars.sh --include-intel-llvm --force

source lib/root/bin/thisroot.sh

export TEST_SANDBOX_DIR="/helix/repo/build/tmp"