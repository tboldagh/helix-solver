# helix-solver

# Old project structure and README
All the files of helix-solver present before pl-new-project-structure have been moved to the `old` directory. Project's components will be gradually moved/copied to the root directory with changes neccessary to make them compliant with the new structure, build system, and conventions.

# Documentation
Documetnation is placed in `docs` directory.

# Setup dev environment
* Build docker image with tag helix-solver-docker: `docker build -t helix-solver-docker .`.
* Run the image in interactive mode: `docker run --gpus all --rm -it -v.:/helix/repo -v /usr/local/cuda-12.1:/cuda helix-solver-docker`.
* At this point you should have root console like looking like: `root@ec333231c56e:/helix/repo# `.
* Prepare environment: `source prepare_environment.sh`.
* Build the app: `./build.sh`.
* Obtain `spacepoints.root` file (e.g. scp it).
* Edit `/code/config.json` to point to it and run the code: `./build/application/HelixSolver/HelixSolver config.json`.

## Troubleshooting
### SYCL_FEATURE_TEST_EXTRACT Function invoked with incorrect arguments
```
SYCL feature test compile failed!
compile output is: 
CMake Error at /opt/intel/oneapi/compiler/2023.2.1/linux/IntelSYCL/IntelSYCLConfig.cmake:282 (SYCL_FEATURE_TEST_EXTRACT):
  SYCL_FEATURE_TEST_EXTRACT Function invoked with incorrect arguments for
  function named: SYCL_FEATURE_TEST_EXTRACT
Call Stack (most recent call first):
  CMakeLists.txt:25 (find_package)
```
If you encounter this error make sure that:
* Cmake uses appropriate compilers (icx for C and icpx for C++), see [Unable to use cmake + intel compiler + intel/oneapi-basekit:devel-ubuntu22.04](https://community.intel.com/t5/Intel-C-Compiler/intel-oneapi-basekit-devel-ubuntu22-04-cmake-intel-compiler-does/m-p/1477916).
* Cuda directory is mounted into the container.
* CUDA is installed on the host machine and accessible from the container. Run `nvcc --version`.
* `$PATH` includes path to CUDA.
* `$LD_LIBRARY_PATH` includes `[path to cuda]/lib64`