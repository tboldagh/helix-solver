#include <iostream>
#include <memory>
#include <TFile.h>
#include <TTree.h>
#include "SingleRunMap.h"

using namespace std;

int main(int argc, char *argv[]) {

    if (argc < 3) {
        std::cerr << "You must pass path to ROOT file and name of tree" << std::endl;
        exit(EXIT_FAILURE);
    }

    TFile l_file(argv[1], "READ");

    TTree *tree = dynamic_cast<TTree *>(l_file.Get(argv[2]));

    if (tree == nullptr) {
        exit(EXIT_FAILURE);
    }

    uint32_t entriesInTree = tree->GetEntries();

    int most = -1;
    std::string mostFileName = "";

    for (uint32_t i = 0; i < entriesInTree; ++i) {
        SingleRunMap singleEvent(tree, i);

        std::string fileName(std::string("out/entries-" +
                                         std::to_string(singleEvent.getSize()) +
                                         "-event-" +
                                         std::to_string(i) +
                                         ".txt"));

        if (singleEvent.getSize() > most) {
            most = singleEvent.getSize();
            mostFileName = fileName;
        }

        singleEvent.dumpToFile(fileName);
    }

    l_file.Close();

    std::cout << "Most entries: " << mostFileName << std::endl;

    return 0;
}
