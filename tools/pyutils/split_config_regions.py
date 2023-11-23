import json
import math

inconfig = None

with open("config_pileup.json", "r") as infile:
    inconfig = json.load(infile)

print(".. loaded config.json")

outputFile_prefix_json = "config_"
outputFile_prefix_root = "detected-circles/detected-circles_pileup_"

min_phi_div =  2
max_phi_div = 20

min_eta_div = 5
max_eta_div = 20

phi_div_array = [i for i in range(min_phi_div, max_phi_div + 1)]
eta_div_array = [i for i in range(min_eta_div, max_eta_div + 1)]

index_i = 1
index_j = 1

for i in phi_div_array:
    index_j = 1
    for j in eta_div_array:
        print("Phi n_div = ", i, "eta n_div = ", j, "\n")
        inconfig["n_phi_regions"] = i
        inconfig["n_eta_regions"] = j
        outputFile_name_json = outputFile_prefix_json + str(index_i) + "_" + str(index_j) + ".json"
        outputFile_name_root = outputFile_prefix_root + str(index_i) + "_" + str(index_j)

        index_j = index_j + 1
        inconfig["outputFile"] = outputFile_name_root

        print(".. saving JSON", outputFile_name_json)
        with open(outputFile_name_json, "w") as outfile:
            json.dump(inconfig, outfile, indent=4)
    index_i = index_i + 1

