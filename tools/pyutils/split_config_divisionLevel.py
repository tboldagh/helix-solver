import json
import math

inconfig = None

with open("config.json", "r") as infile:
    inconfig = json.load(infile)

print(".. loaded config.json")

outputFile_prefix_json = "config_"
outputFile_prefix_root = "detected-circles/detected-circles_single_1k_"

phi_range = 6.28
pt_range = 2
correction_factor = 1.05

min_divisionLevel = 2
max_divisionLevel = 22

phi_precision_array = [correction_factor * phi_range / math.pow(2, i) for i in range(min_divisionLevel, max_divisionLevel + 1)]
pt_precision_array  = [correction_factor * pt_range  / math.pow(2, i) for i in range(min_divisionLevel, max_divisionLevel + 1)]

index_i = 1
index_j = 1

for i in phi_precision_array:
    index_j = 1
    for j in pt_precision_array:
        print("Phi precision = ", i, "qOverPt precision = ", j)
        inconfig["phi_precision"] = i
        inconfig["pt_precision"] = j
        outputFile_name_json = outputFile_prefix_json + str(index_i) + "_" + str(index_j) + ".json"
        outputFile_name_root = outputFile_prefix_root + str(index_i) + "_" + str(index_j)

        index_j = index_j + 1
        inconfig["outputFile"] = outputFile_name_root

        print(".. saving JSON", outputFile_name_json)
        with open(outputFile_name_json, "w") as outfile:
            json.dump(inconfig, outfile, indent=4)
    index_i = index_i + 1
