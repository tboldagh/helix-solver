# Installation

# Environment

## SYCL

### setvars.sh
The environemnt must be prepared to build and run applications using SYCL by sourcing `setvars.sh`. To do so run `source_setvars.sh` in the root directory. Make sure `setvars.sh` is present in ` /opt/intel/oneapi/setvars.sh`. Alternatively, modify path in `source_setvars.sh`.

## Available devices
You can check the list of available devices by running `show_devices.sh` in the root directory. The script requires correct application build. The scripts lists names of available devices. If the list is empty, SYCL environment is probably not set up properly.

# Build
To build the project run `build.sh` in the root directory. Build result will be placed in the `build` directory. SYCL-related environment variables will be automatically sourced using `source_setvars.sh`.

## Options

### clean
```bash
build.sh clean
```
Makes a clean build by removing whole `build` directory before build.

# Run unit tests
Build process generates `TestSuitesList.txt` file which contains a list of all unit test suites in the framework and application. You can run them manually or using `run_test.sh`. The script runs all suites, saves log for each suite next to its binary, and generates a summary of the test results. For more info see `run_test.md`.