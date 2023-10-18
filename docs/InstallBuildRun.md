# Installation

# Environment

## SYCL

### setvars.sh
The environemnt must be prepared to build and run applications using SYCL by sourcing `setvars.sh`. To do so run `source_setvars.sh` in the root directory. Make sure `setvars.sh` is present in ` /opt/intel/oneapi/setvars.sh`. Alternatively, modify path in `source_setvars.sh`.

# Build
To build the project run `build.sh` in the root directory. Build result will be placed in the `build` directory. SYCL-related environment variables will be automatically sourced using `source_setvars.sh`.

## Options

### clean
```bash
build.sh clean
```
Makes a clean build by removing whole `build` directory before build.