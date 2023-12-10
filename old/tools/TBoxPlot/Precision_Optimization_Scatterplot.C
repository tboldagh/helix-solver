#include <cmath>
#include <iostream>
#include <TCanvas.h>

void Precision_Optimization_Scatterplot(
    const Int_t divisionLevel_min      =  2,
    const Int_t divisionLevel_max      =  14
){

    // initial lines
    delete gROOT -> FindObject("scatterplot_nsolutions");
    delete gROOT -> FindObject("scatterplot_percentage");
    delete gROOT -> FindObject("scatterplot_nspacepoints");
    delete gROOT -> FindObject("canvas");

    const Int_t pileup_count = 1;
    const Int_t threshold = 6;

    // definition of names of file and trees used later
    std::string spacepoints_file_name = "spacepoints/spacepoints_single_10k.root";
    std::string spacepoints_tree_name = "spacepoints";

    std::string file_name_raw = "detected-circles_article/detected-circles_single_10k_";
    std::string tree_name = "solutions";

    // histograms definition
    const Int_t n_bins_hist = divisionLevel_max - divisionLevel_min + 1;
    TH2F *scatterplot_nsolutions    =   new TH2F("scatterplot_nsolutions", ";division levels in #veta;division levels in q/p_{T}", n_bins_hist, divisionLevel_min - 0.5, divisionLevel_max + 0.5, n_bins_hist, divisionLevel_min - 0.5, divisionLevel_max + 0.5);
    TH2F *scatterplot_percentage    =   new TH2F("scatterplot_percentage", ";division levels in #varphi;division levels in q/p_{T}", n_bins_hist, divisionLevel_min - 0.5, divisionLevel_max + 0.5, n_bins_hist, divisionLevel_min - 0.5, divisionLevel_max + 0.5);
    TCanvas *canvas  =   new TCanvas("canvas", "canvas", 1200, 1200);
	gStyle  ->  SetOptStat(0);
  	gPad 	-> 	SetRightMargin(0.15);


    // acces data in file spacepoints
    std::unique_ptr<TFile> spacepoints_file(TFile::Open(spacepoints_file_name.c_str()));

    if ( spacepoints_file == nullptr ) {
        throw std::runtime_error("Can't open ROOT file " + spacepoints_file_name);
    }
    std::unique_ptr<TTree> spacepoints_tree(spacepoints_file->Get<TTree>(spacepoints_tree_name.c_str()));

    if (spacepoints_tree == nullptr) {
        throw std::runtime_error("Can't access tree in the ROOT file." + spacepoints_file_name);
    }

    UInt_t event_id_spacepoints;
    spacepoints_tree -> SetBranchAddress("event_id", &event_id_spacepoints);
    Int_t nentries_spacepoints = Int_t(spacepoints_tree -> GetEntries());
    std::cout << "... File " << spacepoints_file_name << " constaint " <<  nentries_spacepoints << " entries" << std::endl;

    // map of spacepoints which will be used to determine number of events, for which
    // phi and qOverPt could be reconstructed - number of entries > threshold
    std::map<Int_t, Int_t> map_of_spacepoints;
    for (Int_t index_spacepoints = 0; index_spacepoints < nentries_spacepoints; ++index_spacepoints){

        spacepoints_tree -> GetEntry(index_spacepoints);
        map_of_spacepoints.try_emplace(event_id_spacepoints, 0);
        map_of_spacepoints[event_id_spacepoints] += 1;
    }

    // check number of event, in which solutions could have possibly be found - number of entries greater than threshold
    Int_t count_acceptable_events{};
    for (const auto & map_element : map_of_spacepoints){

        if (map_element.second >= threshold) ++count_acceptable_events;
    }

    // acces data in the inpout file
    const Int_t max_qOverPt_index = 13;
    Int_t divisionLevel_phi{};
    Int_t divisionLevel_qOverPt{};

    double phi_precision{};
    double qOverPt_precision{};

    double phi_range = 2 * 3.15;
    double qOverPt_range = 2;

    // proper loop over all the files
    for (Int_t phi_index = 1; phi_index <= max_qOverPt_index; ++phi_index){
        for (Int_t qOverPt_index = 1; qOverPt_index <= max_qOverPt_index; ++qOverPt_index){

            divisionLevel_phi = divisionLevel_min + phi_index - 1;
            divisionLevel_qOverPt = divisionLevel_min + qOverPt_index - 1;

            phi_precision = phi_range / TMath::Power(2, divisionLevel_phi);
            qOverPt_precision = qOverPt_range / TMath::Power(2, divisionLevel_qOverPt);

            // redefinition of name of the files
            std::string file_name = file_name_raw + std::to_string(phi_index) + "_" + std::to_string(qOverPt_index) + ".root";
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

            // map collecting event_id and number of solution for each of them
            std::map<Int_t, Int_t> map_of_solutions;
            for (Int_t index = 0; index < nentries; ++index){

                tree -> GetEntry(index);
                map_of_solutions.try_emplace(event_id, 0);
                map_of_solutions[event_id] += 1;
            }

            // number of events, for which at least one solution has been found
            Int_t n_event_id = map_of_solutions.size();

            std::cout << "\n... Filename: " << file_name << std::endl;
            std::cout << "... Phi precision equal " << phi_precision << " resulted in " << nentries << " solutions" << std::endl;
            std::cout << "... qOverPt precision equal " << qOverPt_precision << " resulted in " << nentries << " solutions" << std::endl;
            std::cout << "... Number of reconstructed events: " << n_event_id << "/" << count_acceptable_events << std::endl;

            count_acceptable_events = 2573;
            scatterplot_nsolutions -> Fill(divisionLevel_phi, divisionLevel_qOverPt, float(nentries)/(n_event_id * pileup_count));
            scatterplot_percentage -> Fill(divisionLevel_phi, divisionLevel_qOverPt, float(n_event_id) / count_acceptable_events);
        }
    }

    scatterplot_percentage -> Draw("COLZ");
    canvas -> SaveAs("output/precision_optimization_scatterplot_percentage.pdf");

    canvas -> SetLogz();
    scatterplot_nsolutions -> Draw("COLZ");
    canvas -> SaveAs("output/precision_optimization_scatterplot_nsolutions.pdf");

}