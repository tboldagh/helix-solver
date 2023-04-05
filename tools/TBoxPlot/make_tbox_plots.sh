#!/bin/bash

LOG=$1

function filter() {
    echo "... filtering pattern $1 "
    cat ${LOG} | grep ":$1" | cut -b6- | sed "s/:$1//g" > data_$1.csv
    echo "... done"
}

filter BoxPosition
filter SolutionPair
filter RPhi
filter LinePosition
echo "Producing plots"
root -l -q  scripts/Create_Root_File.C
echo "... done"
