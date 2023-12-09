#include <cmath>
#include <iostream>
#include <TCanvas.h>

void Division_Optimization_Scatterplot(
    const Int_t n_div_phi_min =  2,
    const Int_t n_div_phi_max =  20,

    const Int_t n_div_eta_min =  5,
    const Int_t n_div_eta_max =  20
){

    // initial lines
    delete gROOT -> FindObject("scatterplot_nsolutions");
    delete gROOT -> FindObject("canvas");

    const uint16_t partices_truth_per_event = 542;

    // definition of names of file and trees used later
    std::string file_name_raw = "../../../detected-circles_pileup_scatt/detected-circles_pileup_";
    std::string tree_name = "solutions";

    // histograms definition
    const Int_t n_bins_phi_hist = n_div_phi_max - n_div_phi_min + 1;
    const Int_t n_bins_eta_hist = n_div_eta_max - n_div_eta_min + 1;
    TH2F *scatterplot_nsolutions    =   new TH2F("scatterplot_nsolutions", "Solutions/truth particle (1 GeV < pt < 10 GeV);number of regions in #eta;number of regions in #varphi", n_bins_eta_hist, n_div_eta_min - 0.5, n_div_eta_max + 0.5, n_bins_phi_hist, n_div_phi_min - 0.5, n_div_phi_max + 0.5);
    TCanvas *canvas  =   new TCanvas("canvas", "canvas", 1200, 1200);
	gStyle  ->  SetOptStat(0);
  	gPad 	-> 	SetRightMargin(0.15);

    const uint8_t min_phi_index = 1;
    const uint8_t max_phi_index = 19;

    const uint8_t min_eta_index = 1;
    const uint8_t max_eta_index = 16;

    uint8_t n_div_phi{};
    uint8_t n_div_eta{};

    // proper loop over all the files
    for  (uint8_t phi_index = min_phi_index; phi_index <= max_phi_index; ++phi_index){
        for (uint8_t eta_index = min_eta_index; eta_index <= max_eta_index; ++eta_index){

            n_div_phi = n_div_phi_min + phi_index - 1;
            n_div_eta = n_div_eta_min + eta_index - 1;

            // redefinition of name of the files
            std::string file_name = file_name_raw + std::to_string(phi_index) + "_" + std::to_string(eta_index) + ".root";
            std::unique_ptr<TFile> file(TFile::Open(file_name.c_str()));

            // if a given file does not exist we want to simply omit this particular file
            if ( file == nullptr ) continue;
            std::unique_ptr<TTree> tree(file->Get<TTree>(tree_name.c_str()));

            if (tree == nullptr) {
                throw std::runtime_error("Can't access tree in the ROOT file." + file_name);
            }

            UInt_t event_id;
            tree -> SetBranchAddress("event_id", &event_id);
            Int_t nentries = (Int_t)tree->GetEntries();

            std::cout << "\n... Filename: " << file_name << std::endl;
            std::cout << "... phi n_div = " << int(n_div_phi) << std::endl;
            std::cout << "... eta n_div = " << int(n_div_eta) << " resulted in " << nentries << " solutions" << std::endl;

            scatterplot_nsolutions -> Fill(n_div_eta, n_div_phi, float(nentries)/partices_truth_per_event);
        }
    }

    //canvas -> SetLogz();
    scatterplot_nsolutions -> Draw("COLZ");
    canvas -> SaveAs("output/precision_optimization_scatterplot_nsolutions.pdf");

}