# helix-solver

# Compilation instructions
The project is build with cmake.
To compile: create build dir next to helix-solver and change to it.
```
cmake -DCMAKE_INSTALL_PREFIX=$PWD ../helix-solver  ; make install
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

# Debugging plots

