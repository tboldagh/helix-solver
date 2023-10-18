# Installation

# Environment

## SYCL

### setvars.sh
Make sure `setvars.sh` is present in ` /opt/intel/oneapi/setvars.sh`.

# Build
To build the project run `build.sh` in the root directory. Build result will be placed in the `build` directory. SYCL-related environment variables will automatically be sourced from `/opt/intel/oneapi/setvars.sh`.

## Options

### clean
```bash
build.sh clean
```
Makes a clean build by removing whole `build` directory before build.