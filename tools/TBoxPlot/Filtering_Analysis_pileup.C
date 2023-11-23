#include <cmath>
#include <iostream>
#include <algorithm>
#include <TCanvas.h>

#include "../../include/HelixSolver/Debug.h"
#include "../../include/HelixSolver/ZPhiPartitioning.h"

void Filtering_Analysis_pileup(){

    // Used are two detected-circles files so that output when filtering is enable, can be
    // compared to default setting. File hough_events.root contains event_id which passed selection based on r and z regions.
    std::string file_name_ending = "muon_1k";

    // files for muons
    // std::string truth_file_path =  "../../../data/ODD_Single_muon_10k/particles_initial.root";
    // std::string hough_file_path =  "../../../build/detected-circles/detected-circles_muon.root";

    // files for pions
    // std::string truth_file_path =  "../../../data/ODD_Single_pion_10k/particles_initial.root";
    // std::string hough_file_path =  "../../../build/detected-circles/detected-circles_pion.root";

    // files for pileup
    std::string truth_file_path =  "../../../data/ODD_ttbar_pu200_200ev/particles_initial.root";
    std::string hough_file_path =  "../../../build/detected-circles/detected-circles_ttbar.root";
    
    // other files
    std::string output_path = "output/output_histograms_filtering_analysis_" + file_name_ending + ".root";
    std::string pdf_file_name = "output/filtering_analysis_" + file_name_ending + ".pdf";
    std::string hough_filtered_file_path =  hough_file_path;

    // trees names
    std::string truth_tree_name = "particles";
    std::string hough_tree_name = "solutions";
    std::string hough_filtered_tree_name = "solutions";

    // initial lines
    delete gROOT -> FindObject("scatt_eta");
    delete gROOT -> FindObject("scatt_pt");
    delete gROOT -> FindObject("scatt_1_over_pt");

    delete gROOT -> FindObject("teff_pt");
    delete gROOT -> FindObject("teff_phi");
    delete gROOT -> FindObject("teff_eta");
    delete gROOT -> FindObject("teff_d0");
    
    using std::cout;
    using std::endl;

    TCanvas *canvas1 = new TCanvas("canvas1", "canvas1", 12000, 12000);
    canvas1 -> Divide(3, 2);
    
    // scatter plots
    // parameters for histograms
    const float min_hist_pt = 1.;
    const float max_hist_pt = 10.;

    const float min_hist_1_over_pt = 0;
    const float max_hist_1_over_pt = 1.05;

    const float min_hist_eta = -1.5;
    const float max_hist_eta =  1.5;

    const float min_hist_d0 = 0.0;
    const float max_hist_d0 = 1200.0;

    const float min_hist_phi = -M_PI;
    const float max_hist_phi =  M_PI;

    const Int_t n_bins_scatterplot = 200;
    const Int_t n_bins_efficiency = 80;

    TH2D *scatt_eta = new TH2D("scatt_eta", "Scatterplot of truth #eta vs Hough #eta;truth #eta;Hough #eta", n_bins_scatterplot, min_hist_eta, max_hist_eta, n_bins_scatterplot, min_hist_eta, max_hist_eta);
    TH2D *scatt_pt  = new TH2D("scatt_pt", "Scatterplot of truth p_{T} vs Hough p_{T};truth p_{T};Hough p_{T}", n_bins_scatterplot, min_hist_pt, max_hist_pt, n_bins_scatterplot, min_hist_pt, max_hist_pt);
    TH2D *scatt_1_over_pt  = new TH2D("scatt_1_over_pt", "Scatterplot of truth 1/p_{T} vs Hough 1/p_{T};truth 1/p_{T};Hough 1/p_{T}", n_bins_scatterplot, min_hist_1_over_pt, max_hist_1_over_pt, n_bins_scatterplot, min_hist_1_over_pt, max_hist_1_over_pt);

    TEfficiency *teff_pt  = new TEfficiency("teff_pt",  "Reconstruction efficiency of p_{T};p_{T} [GeV];efficiency", n_bins_efficiency, min_hist_pt, max_hist_pt);
    TEfficiency *teff_phi = new TEfficiency("teff_phi", "Reconstruction efficiency of #varphi;#varphi truth [rad];efficiency", n_bins_efficiency, min_hist_phi, max_hist_phi);
    TEfficiency *teff_eta = new TEfficiency("teff_eta", "Reconstruction efficiency of #eta;#eta truth;efficiency", n_bins_efficiency, min_hist_eta, max_hist_eta);
    TEfficiency *teff_d0  = new TEfficiency("teff_d0",  "Reconstruction efficiency of d_0;d_0 truth;efficiency", 30, min_hist_d0, max_hist_d0);

    gPad 	-> 	SetLeftMargin(0.15);
    gStyle  ->  SetOptStat(0);

    // parameters of the analyzed division region
    const Int_t eta_n_regions = 15;
    const float max_eta =  1.5;
    const float min_eta = -1.5;
    const float eta_width = max_eta - min_eta;
    const float excess_wedge_eta_width = 0.12;
    const float eta_wedge_width = 0.5 * eta_width/eta_n_regions + excess_wedge_eta_width;

    const Int_t phi_n_regions = 8;
    const float min_phi = - M_PI;
    const float max_phi =   M_PI;
    const float phi_width = max_phi - min_phi;
    const float excess_wedge_phi_width = 0.12;
    const float phi_wedge_width = 0.5 * phi_width/phi_n_regions + excess_wedge_phi_width;

    const float min_truth_pt = 1.0;
    const float max_truth_pt = 10.0;

    const float min_truth_eta = -10.0;
    const float max_truth_eta =  10.0;

    const float min_truth_phi = -5.0;
    const float max_truth_phi =  5.0;

    // max delta
    const float max_delta_phi = 0.05;
    const float max_delta_eta = 1.2 * eta_wedge_width;
    const float max_delta_q = 0.5;
    const float max_delta_1_over_pt = 0.1;

    uint32_t total_fakes = 0;
    uint32_t total_duplicates = 0;

    // open files
    std::unique_ptr<TFile> file_truth(TFile::Open(truth_file_path.c_str()));
    std::unique_ptr<TFile> file_hough(TFile::Open(hough_file_path.c_str()));
    std::unique_ptr<TFile> file_hough_filtered(TFile::Open(hough_filtered_file_path.c_str()));

    TFile* file_output = new TFile(output_path.c_str(), "RECREATE"); // ROOT file to save all the output histograms

    if (file_truth == nullptr) {
        throw std::runtime_error("Can't open input file: " + truth_file_path);
    }

    if (file_hough == nullptr) {
        throw std::runtime_error("Can't open input file: " + hough_file_path);
    }

    if (file_hough_filtered == nullptr) {
        throw std::runtime_error("Can't open input file: " + hough_filtered_file_path);
    }
   
    std::unique_ptr<TTree> tree_truth(file_truth->Get<TTree>(truth_tree_name.c_str()));
    std::unique_ptr<TTree> tree_hough(file_hough->Get<TTree>(hough_tree_name.c_str()));
    std::unique_ptr<TTree> tree_hough_filtered(file_hough_filtered->Get<TTree>(hough_filtered_tree_name.c_str()));

    if (tree_truth == nullptr) {
        throw std::runtime_error("Can't access tree in the ROOT file: " + truth_file_path);
    }

    if (tree_hough == nullptr) {
        throw std::runtime_error("Can't access tree in the ROOT file: " + hough_file_path);
    }

    if (tree_hough_filtered == nullptr) {
        throw std::runtime_error("Can't access tree in the ROOT file: " + hough_filtered_file_path);
    }

    // set branch address - truth solutions
    UInt_t event_id_truth_branch;
    std::vector<float> *pt_truth_branch = 0;
    std::vector<float> *phi_truth_branch = 0;
    std::vector<float> *eta_truth_branch = 0;
    std::vector<float> *q_truth_branch = 0;
    std::vector<float> *vx_truth_branch = 0;
    std::vector<float> *vy_truth_branch = 0;
    std::vector<float> *particle_type_truth_branch = 0;

    tree_truth -> SetBranchAddress("event_id", &event_id_truth_branch);
    tree_truth -> SetBranchAddress("pt", &pt_truth_branch);
    tree_truth -> SetBranchAddress("phi", &phi_truth_branch);
    tree_truth -> SetBranchAddress("eta", &eta_truth_branch);
    tree_truth -> SetBranchAddress("q", &q_truth_branch);
    tree_truth -> SetBranchAddress("vx", &vx_truth_branch);
    tree_truth -> SetBranchAddress("vy", &vy_truth_branch);
    tree_truth -> SetBranchAddress("particle_type", &particle_type_truth_branch);
    const Int_t all_truth_nentries = tree_truth -> GetEntries();

    // set branch address and access solution pairs - Hough algorithm, not filtered events
    UInt_t event_id_hough_branch;
    float pt_hough_branch;
    float phi_hough_branch;
    float eta_hough_branch;
    float q_hough_branch;

    tree_hough -> SetBranchAddress("event_id", &event_id_hough_branch);
    tree_hough -> SetBranchAddress("pt", &pt_hough_branch);
    tree_hough -> SetBranchAddress("phi", &phi_hough_branch);
    tree_hough -> SetBranchAddress("eta", &eta_hough_branch);
    tree_hough -> SetBranchAddress("q", &q_hough_branch);

    const Int_t all_hough_nentries = tree_hough -> GetEntries();
    std::map<UInt_t, std::vector<std::array<float, 4>>> hough_events;

    // set branch address and access solution pairs - Hough algorithm, filtered events
    UInt_t event_id_hough_filtered_branch;
    float pt_hough_filtered_branch;
    float phi_hough_filtered_branch;
    float eta_hough_filtered_branch;
    float q_hough_filtered_branch;

    tree_hough_filtered -> SetBranchAddress("event_id", &event_id_hough_filtered_branch);
    tree_hough_filtered -> SetBranchAddress("pt", &pt_hough_filtered_branch);
    tree_hough_filtered -> SetBranchAddress("phi", &phi_hough_filtered_branch);
    tree_hough_filtered -> SetBranchAddress("eta", &eta_hough_filtered_branch);
    tree_hough_filtered -> SetBranchAddress("q", &q_hough_filtered_branch);

    const Int_t all_hough_filtered_nentries = tree_hough_filtered -> GetEntries();
    std::map<UInt_t, std::vector<std::array<float, 4>>> hough_filtered_events;

    // move Hough (not filtered) pt and phi to vector<float>, sort by event_id
    for (Int_t index_truth_hough = 0; index_truth_hough < all_hough_nentries; ++index_truth_hough){

        tree_hough -> GetEntry(index_truth_hough);

        hough_events.try_emplace(event_id_hough_branch, std::vector<std::array<float, 4>>());
        hough_events[event_id_hough_branch].push_back({phi_hough_branch, eta_hough_branch, pt_hough_branch, q_hough_branch});

    }

    // // print particle parameters for each event
    // for (auto event : hough_events){

    //     auto one_of_event_particle = event.second;
    //     for (uint8_t index_truth = 0; index_truth < one_of_event_particle.size(); ++index_truth){

    //         auto particle_parameters = one_of_event_particle.at(index_truth);
    //         cout << "event_id = " << event.first << ", phi: " << particle_parameters[0] << ", eta: " << particle_parameters[1] << ", pt = " << particle_parameters[2] << endl;
    //     }
    //     cout << "\n";
    // }


    // move Hough (filtered) pt and phi to vector<float>, sort by event_id
    for (Int_t index_truth_hough = 0; index_truth_hough < all_hough_filtered_nentries; ++index_truth_hough){

        tree_hough -> GetEntry(index_truth_hough);

        hough_filtered_events.try_emplace(event_id_hough_branch, std::vector<std::array<float, 4>>());
        hough_filtered_events[event_id_hough_branch].push_back({phi_hough_filtered_branch, eta_hough_filtered_branch, pt_hough_filtered_branch, q_hough_filtered_branch});
    }

    // // eta range - comparison between truth particles and accepted in further analysis (not rejected by r and z)
    // float min_eta_truth{};
    // float max_eta_truth{};
    // for (Int_t event_index_truth = 0; event_index_truth < all_truth_nentries; ++event_index_truth){

    //     tree_truth -> GetEntry(event_index_truth);
    //     auto eta_current_vector = eta_truth_branch;

    //     for (Int_t index_truth = 0; index_truth < eta_current_vector -> size(); ++index_truth){

    //         float eta_current = eta_current_vector -> at(index_truth);
    //         if (eta_current > max_eta_truth){
    //             max_eta_truth = eta_current;
    //         } else if (eta_current < min_eta_truth){
    //             min_eta_truth = eta_current;
    //         }
    //     }
    // }
    // cout << "... Minimal value of eta in particles initial: " << min_eta_truth << ", maximal value: " << max_eta_truth << endl;

    // float min_eta_hough = 0;
    // float max_eta_hough = 0;

    // for (Int_t index_hough = 0; index_hough < all_hough_nentries; ++index_hough){

    //     tree_hough -> GetEntry(index_hough);
    //     float hough_eta = eta_hough_branch;

    //     if (hough_eta > max_eta_hough){
    //         max_eta_hough = hough_eta;
    //     } 
        
    //     if (hough_eta < min_eta_hough){
    //         min_eta_hough = hough_eta;
    //     }
    // }
    // cout << "... Minimal value of eta in Hough solutions: " << min_eta_hough << ", maximal value: " << max_eta_hough << "\n" << endl;

    ////////////////////////////////////////
    // efficiency for unfiltered events
    ////////////////////////////////////////

    // efficiency for ok events
    uint64_t found_truth_particles = 0;
    uint64_t truth_particles = 0; 
    bool tefficiency = 0;
    
    for (uint64_t event_id = 0; event_id < all_truth_nentries; ++event_id){

        tree_truth -> GetEntry(event_id);
        auto hough_particles = hough_events[event_id];

        // array storing all the indices for the given event_id, deleted are indices corresponding to 
        // Hough solutions which have been assigned to truth particles, size of the array informs about number of fakes
        std::vector<uint64_t> total_fakes_array;
        for (uint64_t index = 0; index < hough_particles.size(); ++index){

            total_fakes_array.push_back(index);
        }


        for (uint64_t index_truth = 0; index_truth < phi_truth_branch -> size(); ++index_truth){

            float phi_truth = (phi_truth_branch) -> at(index_truth);
            float eta_truth = (eta_truth_branch) -> at(index_truth);
            float pt_truth  = (pt_truth_branch)  -> at(index_truth);
            float q_truth   = (q_truth_branch)   -> at(index_truth);
            float vx_truth  = (vx_truth_branch)  -> at(index_truth);
            float vy_truth  = (vy_truth_branch)  -> at(index_truth); 
            float particle_type_truth = (particle_type_truth_branch) -> at(index_truth);
            float d0_truth  = std::hypot(vx_truth, vy_truth);

            tefficiency = 0;
 
            if (pt_truth < max_truth_pt && pt_truth > min_truth_pt &&
                eta_truth < max_truth_eta && eta_truth > min_truth_eta &&
                phi_truth < max_truth_phi && phi_truth > min_truth_phi &&
                hough_particles.size() > 0){
                ++truth_particles;

                for (uint64_t index_hough = 0; index_hough < hough_particles.size(); ++index_hough){

                    auto particle_parameters = hough_particles.at(index_hough);
                    float phi_hough = particle_parameters[0];
                    float eta_hough = particle_parameters[1];
                    float pt_hough  = particle_parameters[2];
                    float q_hough   = particle_parameters[3];

                    float delta_phi = std::fabs(Wedge::phi_dist(phi_truth, phi_hough));
                    float delta_eta = std::fabs(eta_truth - eta_hough);
                    float delta_q   = std::fabs(q_truth - q_hough);
                    float delta_1_over_pt = std::fabs(1./pt_truth - 1./pt_hough);

                    if (
                        delta_phi < max_delta_phi &&
                        delta_eta < max_delta_eta &&
                        delta_q   < max_delta_q   && 
                        delta_1_over_pt < max_delta_1_over_pt
                        ){

                        // remove from the total_fakes_array solution if it has been assigned to any truth particle
                        // auto iterator = std::find(total_fakes_array.begin(), total_fakes_array.end(), index_hough);
                        std::vector<uint64_t>::iterator iterator = std::find(total_fakes_array.begin(), total_fakes_array.end(), index_hough);
                        if (iterator != total_fakes_array.end()){
                            //uint32_t position = iterator - total_fakes_array.begin();
                            total_fakes_array.erase(iterator);
                        }

                        if (tefficiency == 0){
                            ++found_truth_particles;
                            hough_events[event_id].at(index_hough) = {-100., -100., -100., -100.};

                            scatt_eta -> Fill(eta_truth, eta_hough);
                            scatt_pt  -> Fill(pt_truth, pt_hough);
                            scatt_1_over_pt  -> Fill(1. / pt_truth, 1. / pt_hough);

                        } 

                        tefficiency = 1;
                    }
                }
                teff_pt  -> Fill(tefficiency, pt_truth);
                teff_phi -> Fill(tefficiency, phi_truth);
                teff_eta -> Fill(tefficiency, eta_truth);
                teff_d0  -> Fill(tefficiency, d0_truth);

                // analysis of particles which don't have corresponding solutions
                // if (tefficiency == 0){

                    // std::cout << "phi = " << phi_truth << std::endl;
                    // std::cout << "eta = " << eta_truth << std::endl;
                    // std::cout << "q/pt = " << q_truth/pt_truth << std::endl;
                    // std::cout << "d0 = " << d0_truth << std::endl;
                    // std::cout << "particle type = " << particle_type_truth << std::endl;
                    // std::cout << std::endl;
                // }
            }
        }

        total_fakes += total_fakes_array.size();
    }

    total_duplicates = all_hough_nentries - total_fakes - found_truth_particles;
    cout << ".. Found truth particles: " << found_truth_particles << endl;
    cout << ".. Truth particles: " << truth_particles << endl;
    cout << ".. Algorithm efficiency: " << 100.*found_truth_particles/truth_particles << "%\n" << endl;

    cout << ".. Number of found solutions: " << all_hough_nentries << endl;
    cout << ".. Number of found solutions/truth particle: " << float(all_hough_nentries)/found_truth_particles << "\n" << endl;

    cout << ".. Number of solutions assigned to truth particle: " << found_truth_particles << " -> (" << 100.*found_truth_particles/all_hough_nentries << "%)" << endl;
    cout << ".. Number of fake solutions: " << total_fakes << " -> (" << 100.*total_fakes/all_hough_nentries << "%), among unused solutions: " << 100.*total_fakes/(total_fakes + total_duplicates) << "%" << endl;
    cout << ".. Number of solution duplicates: " << total_duplicates << " -> (" << 100.*total_duplicates/all_hough_nentries << "%), among unused solutions: " << 100.*total_duplicates/(total_fakes + total_duplicates) << "%\n" << endl;

    // save all the histograms to ROOT file
    scatt_eta -> Write();
    scatt_pt -> Write();

    file_output -> Close();
    
    canvas1 -> cd(1);
    scatt_eta -> Draw("SCAT");

    canvas1 -> cd(2);
    scatt_pt -> Draw("SCAT");

    canvas1 -> cd(3);
    //canvas1 -> SetLogx();
    //scatt_1_over_pt -> Draw("SCAT");
    teff_d0 -> Draw();

    canvas1 -> cd(4);
    teff_phi -> Draw();

    canvas1 -> cd(5);
    teff_pt -> Draw();

    canvas1 -> cd(6);
    teff_eta -> Draw();

    // save the canvas to pdf file
    canvas1 -> SaveAs(pdf_file_name.c_str());


}