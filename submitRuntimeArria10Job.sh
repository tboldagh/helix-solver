#!/bin/bash

qsub -l nodes=1:fpga_runtime:arria10:ppn=2 -d . run_fpga_hw.sh
