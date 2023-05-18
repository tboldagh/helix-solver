#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include "TFile.h"
#include "TTree.h"

int main(int argc, char** argv)
{

    // To execute the file:
    // g++ -o split_files split_files.cpp `root-config --cflags --libs`
    // ./split_files ODD_Single_muon_10k/spacepoints.root ODD_Single_muon_10k/spacepoints_single_1k_10k 1000

    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " input_file output_file max_events_per_file\n";
        return 1;
    }

    // Open input file
    TFile* input_file = TFile::Open(argv[1]);
    if (!input_file || input_file->IsZombie()) {
        std::cerr << "Error: could not open input file '" << argv[1] << "'\n";
        return 1;
    }

    // Get input TTree
    std::string tree_name = "spacepoints";
    TTree* input_tree = dynamic_cast<TTree*>(input_file->Get(tree_name.c_str()));
    if (!input_tree) {
        std::cerr << "Error: input file does not contain a TTree named " + tree_name + "\n";
        return 1;
    }

    UInt_t event_id;
    input_tree -> SetBranchAddress("event_id", &event_id);

    // Get maximum number of events per file
    int max_events_per_file = std::stoi(argv[3]);

    // Get total number of events in input tree
    int num_entries = input_tree->GetEntries();
    std::cout << "Input file contains " << num_entries << " entries\n";

    // Define range of events
    Int_t event_min = 0;
    input_tree -> GetEntry(num_entries - 1);
    int event_max = event_id;
    std::cout<< "Input file contains " << event_max << "events\n";

    // Split input tree into multiple files
    for (int i = 0; i < event_max; i += max_events_per_file) {
        // Create output file name
        std::stringstream output_name;
        output_name << argv[2] << "_" << std::setw(4) << std::setfill('0') << i << ".root";

        // Open output file
        TFile* output_file = TFile::Open(output_name.str().c_str(), "recreate");

        // Create output TTree
        TTree* output_tree = input_tree->CloneTree(0);

        // Fill output TTree with events from input TTree
        for (int j = 0; j < num_entries; j++) {

            // Fill tree only with events falling into the analyzed range
            input_tree->GetEntry(j);
            UInt_t current_event_id = event_id;

            if (current_event_id >= i && current_event_id < i + max_events_per_file){
                output_tree->Fill();
            }
        }

        std::cout << "Wrote file " << output_name.str() << " with "  << output_tree -> GetEntries() << " events\n";
        // Write output TTree and close output file
        output_tree->Write();
        output_file->Close();
    }

    // Close input file
    input_file->Close();

    return 0;
}
