import json

inconfig = None

with open("config.json", "r") as infile:
    inconfig = json.load(infile)

print(".. loaded config.json")

outputFile_prefix_json = "config_"
outputFile_prefix_root = "detected-circles/detected-circles_single_1k_"

phi_range = 6.28
pt_range = 2;

min_divisionLevel = 5;
max_divisionLevel = 30;

phi_precision_min = 0.0001
phi_precision_max = 0.01
phi_nfiles = 50

pt_precison_min = 0.001
pt_precison_max = 0.5
pt_nfiles = 50

phi_precision_array = [phi_precision_min]
#phi_precision_array = [round(phi_precision_min + i * (phi_precision_max - phi_precision_min) / (phi_nfiles - 1), 4) for i in range(phi_nfiles)]
pt_precision_array  = [round(pt_precison_min + i * (pt_precison_max - pt_precison_min) / (pt_nfiles - 1), 4) for i in range(pt_nfiles)]

index_i = 1
index_j = 1

for i in phi_precision_array:
    index_j = 1
    for j in pt_precision_array:
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
