#!/bin/bash

qsub -l nodes=1:fpga_compile:ppn=2 -d . reportJob.sh
