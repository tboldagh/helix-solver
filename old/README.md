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
First the log needs to be processed (again assuming you are in `build` dir). 
Parameters included in the log files can be choosen using bool values in the Debug.h file.
```
make install && . ./scripts/make_tbox_plots.sh log

```

Files include in the scripts directory:
* Accumulator_Plots.C - the file enables plotting cells area (different colors indicate the division level), lines considered by the algorithm,
calculated solutions, and solution cell limits. Bool parameters allow to choose which objects are to be plotted. Output histograms are saved to ROOT and PDF format. The file takes as arguments limits of the analyzed section of the accumulator. Compilation requires files produced by make_tbox_plots.sh. To draw plots with selected objects:
```
make install > /dev/null && root -l "scripts/Accumulator_Plots.C(2.1, 2.4, -1., 1.)"
```
Above arguments constrain accumulator to `phi = [2.1, 2.4]` and `q/pt = [-1, 1]`.

* Histogram_pt_phi.C - file plotting a histogram of pt and phi of truth particles and solutions. Output is saved in PDF format. Arguments specify lower
and upper limits of pt and phi. Required inputs are file detected-circles and particle_initial.
```
make install > /dev/null && root -l "scripts/Histogram_pt_phi.C(1, 12, -3.15, 3.15)"
```
Above arguments translates into range of histograms: `q/pt = [1, 12]`, `phi = [-3.15, 3.15]`.

* Histogram_R_Phi.C - plots histogram of r and phi of lines considered by the algorithm. It takes as input file hough_tree_R_Phi.root. No arguments are required.
```
make install > /dev/null && root -l scripts/Histogram_R_Phi.C
```

* Reconstruction_Efficiency_Single.C - file provides a summary of algorithm performance in terms of efficiency. Created histograms: scatterplot of Hough vs truth pt and phi, a scatterplot of Delta pt and phi (for phi also histogram), reconstruction efficiency as a function of pt, phi, eta, a scatterplot of number of hits by eta value. The file requires no arguments. Necessary input files: particle_initial, detected-circles, spacepoints.
```
make install > /dev/null && root -l scripts/Reconstruction_Efficiency_Single.C 
```

* Filtering analysis - comparison of efficiency and number of solutions per event for filtered and unfiltered solutions. No arguments are required. Necessary input files: 
particles_initial, detected-circles for filtered and unfiltered events, hough_events.root (file containing even_id of all events that were not excluded 
after checking the position of spacepoints - excludeEventsWithHitsInRZ, generated just like e.g. hough_tree_BoxPosition.root). Output saved to ROOT and PDF formats.
```
make install > /dev/null && root -l scripts/Filtering_Analysis.C 
```

* Precision_Optimization_Scatterplot.C - file generating a colorful histogram of efficiency and the numer of solutions found in a given selection of phi_precision and
pt_precision. No arguments are required. Each point in the histogram corresponds to a separate file. Two arguments are required - minimal and maximal division levels.
Input file: detected-circles for each set of pt and phi precision and spacepoints. 
```
make install > /dev/null && root -l "scripts/Precision_Optimization_Scatterplot.C(2, 14)"
```
Above arguments translates into analyzed division levels range: `divisionLevel = [2, 14]`.


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



