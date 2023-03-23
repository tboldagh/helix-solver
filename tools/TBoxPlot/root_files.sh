#!/bin/bash
cd input/
rm data_BoxPosition.csv
rm data_SolutionPair.csv
rm data_R_Phi.csv
rm data_LinePosition.csv

cd ../../../
make

./bin/ht_no_sycl config.json | grep ':BoxPosition' > tools/TBoxPlot/input/temp_BoxPosition.txt
echo "BoxPosition saved to txt file."
./bin/ht_no_sycl config.json | grep ':SolutionPair' > tools/TBoxPlot/input/temp_SolutionPair.txt
echo "SolutionPair saved to txt file."
./bin/ht_no_sycl config.json | grep ':PHI' > tools/TBoxPlot/input/temp_R_Phi.txt
echo "PHI saved to txt file."
./bin/ht_no_sycl config.json | grep ':LinePosition' > tools/TBoxPlot/input/temp_LinePosition.txt
echo "LinePosition saved to txt file."

cd tools/TBoxPlot
sed 's/:BoxPosition//g' input/temp_BoxPosition.txt > input/data_BoxPosition.csv
sed 's/:SolutionPair//g' input/temp_SolutionPair.txt > input/data_SolutionPair.csv
sed 's/:PHI//g' input/temp_R_Phi.txt > input/data_R_Phi.csv
sed 's/:LinePosition//g' input/temp_LinePosition.txt > input/data_LinePosition.csv

rm input/temp_BoxPosition.txt
rm input/temp_SolutionPair.txt
rm input/temp_R_Phi.txt
rm input/temp_LinePosition.txt

root -l -q  Create_Root_File.C