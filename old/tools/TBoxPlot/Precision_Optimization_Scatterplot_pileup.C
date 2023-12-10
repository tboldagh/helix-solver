#include <cmath>
#include <iostream>
#include <TCanvas.h>

void Precision_Optimization_Scatterplot_pileup(){

    // initial lines
    delete gROOT -> FindObject("scatterplot_nsolutions");
    delete gROOT -> FindObject("scatterplot_nspacepoints");
    delete gROOT -> FindObject("canvas");

    const Int_t counter = 5;

    // definition of names of file and trees used later

    std::string file_name_raw = "detected-circles_threshold_precision/detected-circles_single_10k_";
    std::string tree_name = "solutions";

    // histograms definition
    const Int_t min_phi_index = 8;
    const Int_t min_qOverPt_index = 4;

    const Int_t max_phi_index = 14;
    const Int_t max_qOverPt_index = 9;

    const Int_t n_phi_bins_hist = max_phi_index - min_phi_index + 1;
    const Int_t n_qOverPt_bins_hist = max_qOverPt_index - min_qOverPt_index + 1;
    TH2F *scatterplot_nsolutions    =   new TH2F("scatterplot_nsolutions", ";precision in #varphi (kind of);precision in q/p_{T} (kind of)", n_phi_bins_hist, min_phi_index - 0.5, max_phi_index + 0.5, n_qOverPt_bins_hist, min_qOverPt_index - 0.5, max_qOverPt_index + 0.5);
    TCanvas *canvas  =   new TCanvas("canvas", "canvas", 1200, 1200);
	gStyle  ->  SetOptStat(0);
  	gPad 	-> 	SetRightMargin(0.15);


    // acces data in the inpout file
    const Int_t max_phi_loop = 7;
    const Int_t max_qOverPt_loop = 6;

    Int_t divisionLevel_phi{};
    Int_t divisionLevel_qOverPt{};

    double phi_precision{};
    double qOverPt_precision{};

    double phi_range = 2 * 3.15;
    double qOverPt_range = 2.1;

    // proper loop over all the files
    for (Int_t phi_index = 1; phi_index <= max_phi_loop; ++phi_index){
        for (Int_t qOverPt_index = 1; qOverPt_index <= max_qOverPt_loop; ++qOverPt_index){

            divisionLevel_phi = min_phi_index + phi_index - 1;
            divisionLevel_qOverPt = min_qOverPt_index + qOverPt_index - 1;

            phi_precision = phi_range / TMath::Power(2, divisionLevel_phi);
            qOverPt_precision = qOverPt_range / TMath::Power(2, divisionLevel_qOverPt);

            // redefinition of name of the files
            std::string file_name = file_name_raw + std::to_string(phi_index) + "_" + std::to_string(qOverPt_index) + "_" + std::to_string(counter) + ".root";
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
            std::cout << "... phi precision = " << phi_precision << ", qOverPt precision = " << qOverPt_precision << std::endl;
            std::cout << "... counter = " << counter << std::endl;
            std::cout << "... n_solutions = " << nentries << std::endl;

            scatterplot_nsolutions -> Fill(divisionLevel_phi, divisionLevel_qOverPt, nentries);
        }
    }

    canvas -> SetLogz();
    scatterplot_nsolutions -> Draw("COLZ");
    canvas -> SaveAs("output/precision_optimization_scatterplot_nsolutions.pdf");

}