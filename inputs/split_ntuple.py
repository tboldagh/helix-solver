#!/usr/bin/env python3.10

import ROOT
import sys
args = sys.argv[1:]
# if len(args) != 4:
#     print("Usage: split_ntuple.py input_file.root tree_name output_file_prefix n_events")
#     sys.exit(-1)
input_file_name = args[0]
input_tree_name = args[1]
output_file_prefix = args[2]
n_events_per_file = int(args[3])

print("Input File: ", input_file_name )
ifile = ROOT.TFile.Open(input_file_name, "OLD")
if not ifile:
    print("Can't open the file: ", input_file_name)
    sys.exit(-1)

itree = ifile.Get(input_tree_name)
if not itree:
    print("Can't read tree ", input_tree_name, " form the file: ", input_file_name)
    sys.exit(-1)

entries = itree.GetEntriesFast()
import math
n_output_files = int(math.ceil(entries/n_events_per_file))
print("Input tree has ", entries, " entries, will generate ", n_output_files, " files")

# generate list of start index, size for outputs
for file_number in range(n_output_files):
    # in the last file record all events till the end of it
    first = file_number*n_events_per_file
    n_events = n_events_per_file-1 if file_number != n_output_files-1 else entries-first
    file_name = output_file_prefix+str(file_number)+".root"
    print("... in file", file_name, " will save ", n_events , " starting from ", first)
    ofile = ROOT.TFile.Open(file_name, "RECREATE")
    # = ROOT.TTree(itree.GetName(), itree.GetTitle())
    output_tree = itree.CopyTree("", "", n_events, first)
    ofile.Write()
    ofile.Close()
ifile.Close()
