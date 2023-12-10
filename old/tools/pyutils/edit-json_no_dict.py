import json

inconfig = None

with open("../../../build/config.json", "r") as infile:
    inconfig = json.load(infile)

print(".. loaded config.json")

outputFile_prefix_json = "config_"
outputFile_prefix_root = "detected-circles_single_1k_"

phi_precision_min = 0.001
phi_precision_max = 0.01
phi_nfiles = 50

qpt_precision_min = 0.05
qpt_precision_max = 0.5
qpt_nfiles = 50

phi_precision_array = [phi_precision_min + i * (phi_precision_max - phi_precision_min) / (phi_nfiles - 1) for i in range()]
qpt_precision_array = [0.05 + i * (0.5 - 0.05) / (22 - 1) for i in range(22)]

index_i = 1
index_j = 1

for i in x_precision_array:
    index_j = 1
    for j in y_precision_array:
        inconfig["x_precision"] = i
        inconfig["y_precision"] = j
        outputFile_name_json = outputFile_prefix_json + str(index_i) + "_" + str(index_j) + ".json"
        outputFile_name_root = outputFile_prefix_root + str(index_i) + "_" + str(index_j)

        index_j = index_j + 1
        inconfig["outputFile"] = outputFile_name_root

        print(".. saving JSON", outputFile_name_json)
        with open(outputFile_name_json, "w") as outfile:
            json.dump(inconfig, outfile, indent=4)
    index_i = index_i + 1
