#!/bin/bash

LOG=$1

function filter() {
    echo "... filtering pattern $1 "
    rm -f data_$1.csv
    rm -f hough_tree_$1.root
    cat ${LOG} | grep ":$1" | cut -b6- | sed "s/:$1//g" > data_$1.csv
    echo "... done"
}

filter BoxPosition
filter SolutionPair
filter RPhi
filter Events
filter MeanStdev
filter WedgeCounts
echo "Producing plots"
root -l -q  scripts/Create_Root_File.C
echo "... done"

rm data_BoxPosition.csv
rm data_SolutionPair.csv
rm data_RPhi.csv
rm data_Events.csv
rm data_MeanStdev.csv
rm data_WedgeCounts.csv