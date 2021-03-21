#include <iostream>
#include <memory>
#include <TFile.h>
#include <TTree.h>
#include "SingleRunMap.h"

using namespace std;

int main(int argc, char* argv[]) {

    if (argc < 5) {
        std::cerr << "You must pass path to ROOT file, name of tree, number of entry and output file name" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::unique_ptr<TFile> l_file = std::make_unique<TFile>(argv[1], "READ");
    std::unique_ptr<SingleRunMap> l_run =
            std::make_unique<SingleRunMap>((TTree*)l_file->Get(argv[2]), stoi(argv[3]));

    l_file->Close();

    std::string_view l_outputFileName = argv[4];
    l_run->dumpToFile(l_outputFileName);
    return 0;
}
