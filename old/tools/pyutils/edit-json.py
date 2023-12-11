#!/usr/bin/env python3
import argparse

# Construct the argument parser
ap = argparse.ArgumentParser(

    epilog="""Replaces values in the JSON file given key string\n
    Example:\n
    ./edit-json.py -i ../../../build/config.json -o t.json -k x_wpi -v 32
    """
)

# Add the arguments to the parser
ap.add_argument("-i", "--input", required=True, help="input JSON file")
ap.add_argument("-o", "--output", required=True, help="output JSON file")
ap.add_argument("-k", "--key", required=True, help="key/name of parameter")
ap.add_argument("-v", "--val", required=True, help="new value for parameter")

args = vars(ap.parse_args())

import json
inconfig = None
with open(args["input"], "r") as infile:
    inconfig = json.load(infile)

print(".. loaded JSON")



def search(dictionary):
    for k, v in dictionary.items():
        if isinstance(v, dict):
            print(".... looking into sub section ", k )
            found = search(v)
            if found:
                return True
        else:
            if args["key"] == str(k): # convert from unicode
                print(".. update value under key: ", str(k), "from value:", dictionary[k], "to value:", v)
                dictionary[k] = v
                return True
    return False
print(".... looking into top level section" )
updated = search(inconfig)
if updated:
    print(".. saving JSON ", args["output"])
    with open(args["output"], "w") as outfile:
        json.dump(inconfig, outfile, indent=4)
else:
    print(".. no update, error in the key name?")