# helix-solver

# Compilation instructions
The project is build with cmake.
To compile: create `build` dir next to helix-solver and change to it.
```
cmake -DCMAKE_INSTALL_PREFIX=$PWD ../helix-solver ; make install
```
programs are installed in bin subdirectory of the build in this case.
By default a lot of debugging is enabled.
It can be switched off by editing editing CMakeFile.txt file.

Together with the main code unit tests are compiled.
It is recommended to run them after compilation to check if local env is not somehow faulty.
```
./test/*
```

By default oneAPI/SYCL independent version is only compiled.
To compile SYCL variant run `make ht_sycl`

# Running
```
./ht_no_sycl config.json | tee log
```
The JSON file defines input files.

When developing it is hand to combine running with the compilation & install:
```
cd build
make install && ./bin/ht_no_sycl config.json > log
```

# Debugging plots
The plots for drawing solutions are also installed by default.
First the log needs to be processed like this (again assuming you are in `build` dir):
```
make install && . ./scripts/make_tbox_plots.sh log
```
To draw lines considered by the algorithm (additional install ):
```
make install > /dev/null && root -l "scripts/Accumulator_Lines_R_Phi.C(1.82, 1.92, -0.5, 0.2)"
```
Above the range accumulator are is constrained to `phi = [2.1, 2.4]` and `q/pt = [-1, 1]`.


# Compiling with oneAPI
First, the oneAPI installation has to be available on the system.
The project provides docker image with oneAPI + ROOT.
To use it: copy this file to some empty dir and run command listed in the end of the file (in the comment). The image is sizeable (about 18 GB).
There is also an example command to launch this image `docker run ...`.

The image mounts current directory as `/helix` while in container. You need to launch it from place where all sources codes are visible. Below I assume it is just above the `helix-solver` dir.

To setup compilation in `build` dir that is next to `helix-solver` dir:
```
cmake -DCMAKE_C_COMPILER=icx -DCMAKE_CXX_COMPILER=icpx ../helix-solver/
```
Hint. The `build` dir needs to be cleaned up before. Without this the standard compliler



