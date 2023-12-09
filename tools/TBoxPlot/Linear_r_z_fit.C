#include <cmath>
#include <iostream>
#include <TCanvas.h>

void Linear_r_z_fit(){

    // general and intro
    float max_event_id = 10000;
    delete gROOT -> FindObject("hist");
    delete gROOT -> FindObject("Hist_pileup");
    delete gROOT -> FindObject("c1");

    // define histogram
    const float min_r2 = 0.7;
    const float max_r2 = 1.0;
    const Int_t n_bins = 100;

    TCanvas *c1 = new TCanvas("c1", "c1", 8000, 12000);
    c1 -> Divide(2, 1);
    TH1D *hist = new TH1D("hist", "Histogram of R^{2} - single;R^{2};counts", n_bins, min_r2, max_r2); 
    TH1D *hist_pileup = new TH1D("hist_pileup", "Histogram of R^{2} - pileup;R^{2};counts", n_bins, min_r2, max_r2); 

    // name of the file with spacepoints
    const std::string spacepoints_file_name = "../../../data/ODD_Single_pion_10k/spacepoints.root";
    const std::string filename_r2 = "hough_tree_files/hough_r2.root";

    std::string spacepoints_tree_name = "spacepoints";
    std::string r2_tree_name = "tree";

    // acces data in file spacepoints
    std::unique_ptr<TFile> spacepoints_file(TFile::Open(spacepoints_file_name.c_str()));

    if ( spacepoints_file == nullptr ) {
        throw std::runtime_error("Can't open ROOT file " + spacepoints_file_name);
    }
    std::unique_ptr<TTree> spacepoints_tree(spacepoints_file->Get<TTree>(spacepoints_tree_name.c_str()));

    if (spacepoints_tree == nullptr) {
        throw std::runtime_error("Can't access tree in the ROOT file." + spacepoints_file_name);
    }

    // acces data in file R2
    std::unique_ptr<TFile> r2_file(TFile::Open(filename_r2.c_str()));

    if ( r2_file == nullptr ) {
        throw std::runtime_error("Can't open ROOT file " + filename_r2);
    }
    std::unique_ptr<TTree> r2_tree(r2_file->Get<TTree>(r2_tree_name.c_str()));

    if (r2_tree == nullptr) {
        throw std::runtime_error("Can't access tree in the ROOT file." + filename_r2);
    }

    // access spacepoint branches
    UInt_t event_id_spacepoints;
    float x;
    float y;
    float z;

    spacepoints_tree -> SetBranchAddress("event_id", &event_id_spacepoints);
    spacepoints_tree -> SetBranchAddress("x", &x);
    spacepoints_tree -> SetBranchAddress("y", &y);
    spacepoints_tree -> SetBranchAddress("z", &z);

    Int_t nentries_spacepoints = Int_t(spacepoints_tree -> GetEntries());
    std::cout << "... Spacepoints file constaints " <<  nentries_spacepoints << " entries" << std::endl;

    // access R2 branches
    float r2;

    r2_tree -> SetBranchAddress("R2", &r2);
    Int_t nentries_r2 = Int_t(r2_tree -> GetEntries());

    // map of spacepoints which will be used to determine number of events, for which
    // phi and qOverpt could be reconstructed - number of entries > threshold
    std::map<Int_t, std::pair<std::vector<float>, std::vector<float>>> map_spacepoints;
    for (Int_t index_spacepoints = 0; index_spacepoints < nentries_spacepoints; ++index_spacepoints){

        spacepoints_tree -> GetEntry(index_spacepoints);
        map_spacepoints.try_emplace(event_id_spacepoints, std::pair<std::vector<float>, std::vector<float>>());
        map_spacepoints[event_id_spacepoints].first.push_back(std::hypot(x, y));
        map_spacepoints[event_id_spacepoints].second.push_back(z);
    }


    // calculate linear fint coefficients
    // y - radius
    // x - z - coordinate

    float y_mean{};
    float x_mean{};

    for (uint32_t event_id = 0; event_id < max_event_id; ++event_id){

        std::vector<float> r_vector = map_spacepoints[event_id].first;
        std::vector<float> z_vector = map_spacepoints[event_id].second;
        uint16_t n_spacepoints = r_vector.size();

        x_mean = 0;
        y_mean = 0;

        // calculate x and y mean
        for (Int_t spacepoint_index = 0; spacepoint_index < n_spacepoints; ++spacepoint_index){

            x_mean += z_vector.at(spacepoint_index);
            y_mean += r_vector.at(spacepoint_index);

        }

        x_mean /= n_spacepoints;
        y_mean /= n_spacepoints;

        //std::cout << event_id << " " << x_mean << " " << y_mean << std::endl;

        // calculate b_1
        float numerator{};
        float denominator{};

        for (Int_t spacepoint_index = 0; spacepoint_index < n_spacepoints; ++spacepoint_index){

            numerator += (z_vector.at(spacepoint_index) - x_mean) * (r_vector.at(spacepoint_index) - y_mean);
            denominator += std::pow(z_vector.at(spacepoint_index) - x_mean, 2);
        }

        float b1 = numerator / denominator;
        float b0 = y_mean - b1 * x_mean;

        //std::cout << event_id << " " << b0 << " " << b1 << "\n";
        for (Int_t spacepoint_index = 0; spacepoint_index < n_spacepoints; ++spacepoint_index){

            //std::cout << r_vector.at(spacepoint_index) << " " << b0 + b1 * z_vector.at(spacepoint_index) << "\n";
        }
        //std::cout << "\n" << std::endl;

        // calculate R^2 coefficient
        float ss_res{};
        float ss_tot{};

        for (Int_t spacepoint_index = 0; spacepoint_index < n_spacepoints; ++spacepoint_index){

            ss_res += std::pow(r_vector.at(spacepoint_index) - std::fma(b1, z_vector.at(spacepoint_index), b0), 2);
            ss_tot += std::pow(r_vector.at(spacepoint_index) - y_mean, 2);
        }

        float R2 = 1 - ss_res / ss_tot;
        hist -> Fill(R2);

        //std::cout << event_id << " " << R2 << std::endl;
    }

    // fill pileup histogram with R2
    for  (uint32_t index = 0; index < nentries_r2; ++index){

        r2_tree -> GetEntry(index);
        hist_pileup -> Fill(r2);

    }

    c1 -> cd(1);
    hist -> Draw();

    c1 -> cd(2);
    hist_pileup -> Draw();
}