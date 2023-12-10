#include <cmath>
#include <iostream>
#include <TCanvas.h>
#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TCanvas.h"


void Spacepoints(){

    // initials steps
    delete gROOT -> FindObject("canvas");
    delete gROOT -> FindObject("hist_hits");
    delete gROOT -> FindObject("hist_delta_r");
    delete gROOT -> FindObject("hist_min_delta_r");
    delete gROOT -> FindObject("scatt_phi");
    delete gROOT -> FindObject("scatt_eta");

    // access data in tree
    std::string spacepoints_path =  "../../../data/ODD_Single_pion_10k/spacepoints.root";
    std::string particles_initial_path =  "../../../data/ODD_Single_pion_10k/particles_initial.root";
    std::string tree_name = "spacepoints";
    std::string particles_initial_tree_name = "particles";

    // open file
    std::unique_ptr<TFile> spacepoints_file(TFile::Open(spacepoints_path.c_str()));
    if (spacepoints_file == nullptr){

        throw std::runtime_error("Can't open input file " + spacepoints_path);
    }
    
    std::unique_ptr<TFile> particles_initial_file(TFile::Open(particles_initial_path.c_str()));
    if (particles_initial_file == nullptr){

        throw std::runtime_error("Can't open input file " + particles_initial_path);
    }

    // read tree
    std::unique_ptr<TTree> spacepoints_tree(spacepoints_file -> Get<TTree>(tree_name.c_str()));
    if (spacepoints_tree == nullptr){

        throw std::runtime_error("Can't open tree " + tree_name + " in file " + spacepoints_path);
    }

    std::unique_ptr<TTree> particles_initial_tree(particles_initial_file -> Get<TTree>(particles_initial_tree_name.c_str()));
    if (particles_initial_tree == nullptr){

        throw std::runtime_error("Can't open tree " + particles_initial_tree_name + " in file " + particles_initial_path);
    }

    // define parameters for the histograms
    const uint32_t min_hits_per_truth = 7;
    const uint32_t max_hits_per_truth = 21;

    const float min_delta_r = 0.;
    const float max_delta_r = 4.;

    const float min_phi = -M_PI;
    const float max_phi =  M_PI;

    const float min_eta = -1.5;
    const float max_eta =  1.5;

    const float min_delta_phi = 0.;
    const float max_delta_phi = 0.015;

    const uint32_t hist_hits_n_bins = max_hits_per_truth - min_hits_per_truth + 1;
    const uint32_t hist_delta_r_n_bins = 80;
    const uint32_t scatt_n_bins = 100;
    const uint32_t scatt_n_hits_n_bins = 40;

    // define histograms
    TCanvas *canvas = new TCanvas("canvas", "canvas", 12000, 8000);
    TH1I* hist_hits = new TH1I("hist_hits", "Hits / particle truth;number of hits;counts", hist_hits_n_bins, min_hits_per_truth, max_hits_per_truth);
    TH1D* hist_delta_r = new TH1D("hist_delta_r", "#Delta r for each pair;#Delta r;counts", hist_delta_r_n_bins, min_delta_r, max_delta_r);
    TH1D* hist_min_delta_r = new TH1D("hist_min_delta_r", "Min #Delta r for each pair;#Delta r;counts", hist_delta_r_n_bins, min_delta_r, max_delta_r);

    TH2D* scatt_delta = new TH2D("scatt_delta", "#Delta #varphi vs #Delta r;#Delta #phi;#Delta r", scatt_n_bins, min_delta_phi, max_delta_phi, scatt_n_bins, min_delta_r, max_delta_r);
    TH2D* scatt_phi   = new TH2D("scatt_phi", "Hits / particle truth by #varphi;#varphi;number of hits", scatt_n_hits_n_bins, min_phi, max_phi, hist_hits_n_bins, min_hits_per_truth, max_hits_per_truth);
    TH2D* scatt_eta   = new TH2D("scatt_eta", "Hits / particle truth by #eta;#eta;number of hits", scatt_n_hits_n_bins, min_eta, max_eta, hist_hits_n_bins, min_hits_per_truth, max_hits_per_truth);

    gStyle -> SetOptStat(0);

    // access variables in the file
    UInt_t event_id;
    float x;
    float y;
    float z;

    spacepoints_tree -> SetBranchAddress("event_id", &event_id);
    spacepoints_tree -> SetBranchAddress("x", &x);
    spacepoints_tree -> SetBranchAddress("y", &y);
    spacepoints_tree -> SetBranchAddress("z", &z);
    uint32_t nentries = spacepoints_tree -> GetEntries();
    std::cout << "File spacepoints.root containt " << nentries << " spacepoints." << std::endl;

    std::map<uint32_t, std::pair<std::vector<float>, std::vector<float>>> spacepoints;

    // set branch address - truth solutions
    UInt_t event_id_truth_branch;
    std::vector<float> *pt_truth_branch = 0;
    std::vector<float> *phi_truth_branch = 0;
    std::vector<float> *eta_truth_branch = 0;
    std::vector<float> *q_truth_branch = 0;

    particles_initial_tree -> SetBranchAddress("event_id", &event_id_truth_branch);
    particles_initial_tree -> SetBranchAddress("pt", &pt_truth_branch);
    particles_initial_tree -> SetBranchAddress("phi", &phi_truth_branch);
    particles_initial_tree -> SetBranchAddress("eta", &eta_truth_branch);
    particles_initial_tree -> SetBranchAddress("q", &q_truth_branch);
    const Int_t nentries_hough = particles_initial_tree -> GetEntries();

    // save x, y, z coordinates as r and phi in new object
    for(uint32_t index = 0; index < nentries; ++index){

        spacepoints_tree -> GetEntry(index);

        spacepoints.try_emplace(event_id, std::pair<std::vector<float>, std::vector<float>>());
        spacepoints[event_id].first.push_back(std::hypot(x, y));
        spacepoints[event_id].second.push_back(std::atan2(y, x));
    }

    // iterate over all the events
    uint32_t min_hits = 100;
    uint32_t max_hits = 0;
    for (auto event : spacepoints){

        std::pair<std::vector<float>, std::vector<float>> particle_pair = event.second;
        std::vector<float> rs = particle_pair.first;
        std::vector<float> phis = particle_pair.second;

        uint32_t n_hits = rs.size();
        hist_hits -> Fill(n_hits);

        float min_delta_r = 100.;
        float min_delta_phi = 100.;

        float delta_r{};
        float delta_phi{};

        // truth particles
        particles_initial_tree -> GetEntry(event.first);
        float phi_truth = phi_truth_branch -> at(0);
        float eta_truth = eta_truth_branch -> at(0);
        scatt_phi -> Fill(phi_truth, n_hits);
        scatt_eta -> Fill(eta_truth, n_hits);

        for (uint32_t index_hit = 0; index_hit < n_hits; ++index_hit){
            for (uint32_t index_hit_2 = 0; index_hit_2 < index_hit; ++index_hit_2){

                float r1 = rs[index_hit];
                float r2 = rs[index_hit_2];

                float phi1 = phis[index_hit];
                float phi2 = phis[index_hit_2];

                float delta_r = std::fabs(r1 - r2);
                float delta_phi =  std::fabs(phi1 - phi2);

                if (delta_r < min_delta_r){

                    min_delta_r = delta_r;
                }

                hist_delta_r -> Fill(delta_r);
                scatt_delta  -> Fill(delta_phi, delta_r);

            }
        }

        hist_min_delta_r -> Fill(min_delta_r);
    }

    canvas -> Divide(3, 2);
    
    canvas -> cd(1);
    hist_hits -> Draw();

    canvas -> cd(2);
    hist_delta_r -> Draw();

    canvas -> cd(3);
    hist_min_delta_r -> Draw();

    canvas -> cd(4);
    scatt_delta -> Draw("COLZ");

    canvas -> cd(5);
    scatt_phi -> Draw("COLZ");

    canvas -> cd(6);
    scatt_eta -> Draw("COLZ");
    
    
    canvas -> SaveAs("output/spacepoint.pdf");

}