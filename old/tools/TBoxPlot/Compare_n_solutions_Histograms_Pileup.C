#include <iostream>

void Compare_n_solutions_Histograms_Pileup(){

    std::string hough_file_path_raw   =  "../../../build/detected-circles/compare_histograms_1.root";
    std::string hough_file_path_order =  "../../../build/detected-circles/compare_histograms_2.root";
    std::string hough_file_path_gauss =  "../../../build/detected-circles/compare_histograms_3.root";

    std::string truth_file_path =  "../../../data/ODD_ttbar_pu200_200ev/particles_initial.root";

    std::string hough_tree_name = "solutions";
    std::string truth_tree_name = "particles";

    using std::cout;
    using std::endl;
    delete gROOT -> FindObject("hist_raw");
    delete gROOT -> FindObject("hist_order");
    delete gROOT -> FindObject("hist_gauss");

    TCanvas *canvas1 = new TCanvas("canvas1", "canvas1", 800, 800);

    // truth transverse momentum
    const float min_truth_pt = 1.0;
    const float max_truth_pt = 10.0;

    // divisions phi
    const Int_t n_regions_phi = 8;
    const float min_phi = -M_PI;
    const float max_phi =  M_PI;
    const float phi_width = (max_phi - min_phi)/n_regions_phi;

    // divisions phi
    const Int_t n_regions_eta = 15;
    const float min_eta = -1.5;
    const float max_eta =  1.5;
    const float eta_width = (max_eta - min_eta)/n_regions_eta;

    // histograms
    const uint8_t n_solutions_min = 0;
    const uint16_t n_solutions_max = 600;
    const uint8_t n_bins = 80;
    TH1D *hist_raw = new TH1D("hist_raw", "", n_bins, n_solutions_min, n_solutions_max);
    TH1D *hist_order = new TH1D("hist_order", "", n_bins, n_solutions_min, n_solutions_max);
    TH1D *hist_gauss = new TH1D("hist_gauss", ";found solutions / truth particles;counts", n_bins, n_solutions_min, n_solutions_max);
    gStyle  ->  SetOptStat(0);
    gPad 	-> 	SetLeftMargin(0.15);

    // truth particles
    std::unique_ptr<TFile> file_truth(TFile::Open(truth_file_path.c_str()));
    
    if (file_truth == nullptr) {
        throw std::runtime_error("Can't open input file: " + truth_file_path);
    }

    std::unique_ptr<TTree> tree_truth(file_truth->Get<TTree>(truth_tree_name.c_str()));

    if (tree_truth == nullptr) {
        throw std::runtime_error("Can't access tree in the ROOT file: " + truth_file_path);
    }

    //Hough solutions
    std::unique_ptr<TFile> file_hough_raw(TFile::Open(hough_file_path_raw.c_str()));
    std::unique_ptr<TFile> file_hough_order(TFile::Open(hough_file_path_order.c_str()));
    std::unique_ptr<TFile> file_hough_gauss(TFile::Open(hough_file_path_gauss.c_str()));

    if (file_hough_raw == nullptr) {
        throw std::runtime_error("Can't open input file: " + hough_file_path_raw);
    }
    if (file_hough_order == nullptr) {
        throw std::runtime_error("Can't open input file: " + hough_file_path_order);
    }
    if (file_hough_gauss == nullptr) {
        throw std::runtime_error("Can't open input file: " + hough_file_path_gauss);
    }

    std::unique_ptr<TTree> tree_hough_raw(file_hough_raw->Get<TTree>(hough_tree_name.c_str()));
    std::unique_ptr<TTree> tree_hough_order(file_hough_order->Get<TTree>(hough_tree_name.c_str()));
    std::unique_ptr<TTree> tree_hough_gauss(file_hough_gauss->Get<TTree>(hough_tree_name.c_str()));

    if (tree_hough_raw == nullptr) {
        throw std::runtime_error("Can't access tree in the ROOT file: " + hough_file_path_raw);
    }
    if (tree_hough_order == nullptr) {
        throw std::runtime_error("Can't access tree in the ROOT file: " + hough_file_path_order);
    }
    if (tree_hough_gauss == nullptr) {
        throw std::runtime_error("Can't access tree in the ROOT file: " + hough_file_path_gauss);
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

    // raw solutions
    UInt_t event_id_hough_branch_raw;
    float eta_hough_branch_raw;
    float phi_hough_branch_raw;
    tree_hough_raw -> SetBranchAddress("event_id", &event_id_hough_branch_raw);
    tree_hough_raw -> SetBranchAddress("eta", &eta_hough_branch_raw);
    tree_hough_raw -> SetBranchAddress("phi", &phi_hough_branch_raw);
    const Int_t nentries_hough_raw = tree_hough_raw -> GetEntries();
    std::map<UInt_t, std::pair<std::vector<float>, std::vector<float>>> hough_events_raw;
    std::cout << "\n... Total number of solutions without any optimalization: " << nentries_hough_raw << std::endl;

    // solutions after dividing in terms of regions
    UInt_t event_id_hough_branch_order;
    float eta_hough_branch_order;
    float phi_hough_branch_order;
    tree_hough_order -> SetBranchAddress("event_id", &event_id_hough_branch_order);
    tree_hough_order -> SetBranchAddress("eta", &eta_hough_branch_order);
    tree_hough_order -> SetBranchAddress("phi", &phi_hough_branch_order);
    std::map<UInt_t, std::pair<std::vector<float>, std::vector<float>>> hough_events_order;
    const Int_t nentries_hough_order = tree_hough_order -> GetEntries();
    std::cout << "... Total number of solutions for order checking: " << nentries_hough_order << std::endl;

    // solutions after dividing in terms of regions and applyign Gauss filtering
    UInt_t event_id_hough_branch_gauss;
    float eta_hough_branch_gauss;
    float phi_hough_branch_gauss;
    tree_hough_gauss -> SetBranchAddress("event_id", &event_id_hough_branch_gauss);
    tree_hough_gauss -> SetBranchAddress("eta", &eta_hough_branch_gauss);
    tree_hough_gauss -> SetBranchAddress("phi", &phi_hough_branch_gauss);
    std::map<UInt_t, std::pair<std::vector<float>, std::vector<float>>> hough_events_gauss;
    const Int_t nentries_hough_gauss = tree_hough_gauss -> GetEntries();
    std::cout << "... Total number of solutions for order checking and gauss filtering: " << nentries_hough_gauss << "\n" << std::endl;

    // saving raw solutions in a single map type object
    for (Int_t index = 0; index < nentries_hough_raw; ++index){

        tree_hough_raw -> GetEntry(index);

        hough_events_raw.try_emplace(event_id_hough_branch_raw, std::pair<std::vector<float>, std::vector<float>>());
        hough_events_raw[event_id_hough_branch_raw].first.push_back(eta_hough_branch_raw);
        hough_events_raw[event_id_hough_branch_raw].second.push_back(phi_hough_branch_raw);
    }

    // saving solutions after order check in a single map type object
    for (Int_t index = 0; index < nentries_hough_order; ++index){

        tree_hough_order -> GetEntry(index);

        hough_events_order.try_emplace(event_id_hough_branch_order, std::pair<std::vector<float>, std::vector<float>>());
        hough_events_order[event_id_hough_branch_order].first.push_back(eta_hough_branch_order);
        hough_events_order[event_id_hough_branch_order].second.push_back(phi_hough_branch_order);
    }

    // saving solutions after order check andn Gauss filtering in a single map type object
    for (Int_t index = 0; index < nentries_hough_gauss; ++index){

        tree_hough_gauss -> GetEntry(index);

        hough_events_gauss.try_emplace(event_id_hough_branch_gauss, std::pair<std::vector<float>, std::vector<float>>());
        hough_events_gauss[event_id_hough_branch_gauss].first.push_back(eta_hough_branch_gauss);
        hough_events_gauss[event_id_hough_branch_gauss].second.push_back(phi_hough_branch_gauss);
    }

    // fill raw histogram
    for (auto event : hough_events_raw){

        uint32_t event_id = event.first;

        // Truth solutions
        tree_truth -> GetEntry(event_id);

        // Hough solutions
        auto solution_pair = event.second;
        std::vector<float> solution_eta = solution_pair.first;
        std::vector<float> solution_phi = solution_pair.second;
        double n_solutions_per_event = solution_phi.size();

        for (uint32_t index_phi = 0; index_phi < n_regions_phi; ++index_phi){
            for (uint32_t index_eta = 0; index_eta < n_regions_eta; ++index_eta){

                const float phi_region_min = min_phi + index_phi * phi_width;
                const float phi_region_max = min_phi + (index_phi + 1) * phi_width;

                const float eta_region_min = min_eta + index_eta * eta_width;
                const float eta_region_max = min_eta + (index_eta + 1) * eta_width;

                //std::cout << "phi: " << phi_region_min << ", " << phi_region_max << std::endl;
                //std::cout << "eta: " << eta_region_min << ", " << eta_region_max << std::endl;

                uint32_t hough_counter = 0;

                for (uint32_t index = 0; index < n_solutions_per_event; ++index){

                    float phi = solution_phi.at(index);
                    float eta = solution_eta.at(index);

                    if (phi > phi_region_min && phi < phi_region_max && eta > eta_region_min && eta < eta_region_max){

                        ++hough_counter;
                    }
                }

                // Truth particles
                uint32_t truth_counter = 0;
                for (uint64_t index_truth = 0; index_truth < phi_truth_branch -> size(); ++index_truth){

                    float phi_truth = (phi_truth_branch) -> at(index_truth);
                    float eta_truth = (eta_truth_branch) -> at(index_truth);
                    float pt_truth  = (pt_truth_branch)  -> at(index_truth);

                    if (phi_truth > phi_region_min && phi_truth < phi_region_max && 
                        eta_truth > eta_region_min && eta_truth < eta_region_max &&
                        pt_truth > min_truth_pt && pt_truth < max_truth_pt){

                        ++truth_counter;
                    }
                }
                //std::cout << hough_counter << ", " << truth_counter << ", " << hough_counter/truth_counter << "\n" << std::endl;
                float hough_per_truth = float(hough_counter) / truth_counter;

                hist_raw -> Fill(hough_per_truth);
            }
        }
    }


    // fill count changes histogram
    for (auto event : hough_events_order){

        uint32_t event_id = event.first;

        // Truth solutions
        tree_truth -> GetEntry(event_id);

        // Hough solutions
        auto solution_pair = event.second;
        std::vector<float> solution_eta = solution_pair.first;
        std::vector<float> solution_phi = solution_pair.second;
        double n_solutions_per_event = solution_phi.size();

        for (uint32_t index_phi = 0; index_phi < n_regions_phi; ++index_phi){
            for (uint32_t index_eta = 0; index_eta < n_regions_eta; ++index_eta){

                const float phi_region_min = min_phi + index_phi * phi_width;
                const float phi_region_max = min_phi + (index_phi + 1) * phi_width;

                const float eta_region_min = min_eta + index_eta * eta_width;
                const float eta_region_max = min_eta + (index_eta + 1) * eta_width;

                //std::cout << "phi: " << phi_region_min << ", " << phi_region_max << std::endl;
                //std::cout << "eta: " << eta_region_min << ", " << eta_region_max << std::endl;

                uint32_t hough_counter = 0;

                for (uint32_t index = 0; index < n_solutions_per_event; ++index){

                    float phi = solution_phi.at(index);
                    float eta = solution_eta.at(index);

                    if (phi > phi_region_min && phi < phi_region_max && eta > eta_region_min && eta < eta_region_max){

                        ++hough_counter;
                    }
                }

                // Truth particles
                uint32_t truth_counter = 0;
                for (uint64_t index_truth = 0; index_truth < phi_truth_branch -> size(); ++index_truth){

                    float phi_truth = (phi_truth_branch) -> at(index_truth);
                    float eta_truth = (eta_truth_branch) -> at(index_truth);
                    float pt_truth  = (pt_truth_branch)  -> at(index_truth);

                    if (phi_truth > phi_region_min && phi_truth < phi_region_max && 
                        eta_truth > eta_region_min && eta_truth < eta_region_max &&
                        pt_truth > min_truth_pt && pt_truth < max_truth_pt){

                        ++truth_counter;
                    }
                }
                //std::cout << hough_counter << ", " << truth_counter << ", " << hough_counter/truth_counter << "\n" << std::endl;
                float hough_per_truth = float(hough_counter) / truth_counter;

                hist_order -> Fill(hough_per_truth);
            }
        }
    }

    // fill Gauss histogram
    for (auto event : hough_events_gauss){

        uint32_t event_id = event.first;

        // Truth solutions
        tree_truth -> GetEntry(event_id);

        // Hough solutions
        auto solution_pair = event.second;
        std::vector<float> solution_eta = solution_pair.first;
        std::vector<float> solution_phi = solution_pair.second;
        double n_solutions_per_event = solution_phi.size();

        for (uint32_t index_phi = 0; index_phi < n_regions_phi; ++index_phi){
            for (uint32_t index_eta = 0; index_eta < n_regions_eta; ++index_eta){

                const float phi_region_min = min_phi + index_phi * phi_width;
                const float phi_region_max = min_phi + (index_phi + 1) * phi_width;

                const float eta_region_min = min_eta + index_eta * eta_width;
                const float eta_region_max = min_eta + (index_eta + 1) * eta_width;

                //std::cout << "phi: " << phi_region_min << ", " << phi_region_max << std::endl;
                //std::cout << "eta: " << eta_region_min << ", " << eta_region_max << std::endl;

                uint32_t hough_counter = 0;

                for (uint32_t index = 0; index < n_solutions_per_event; ++index){

                    float phi = solution_phi.at(index);
                    float eta = solution_eta.at(index);

                    if (phi > phi_region_min && phi < phi_region_max && eta > eta_region_min && eta < eta_region_max){

                        ++hough_counter;
                    }
                }

                // Truth particles
                uint32_t truth_counter = 0;
                for (uint64_t index_truth = 0; index_truth < phi_truth_branch -> size(); ++index_truth){

                    float phi_truth = (phi_truth_branch) -> at(index_truth);
                    float eta_truth = (eta_truth_branch) -> at(index_truth);
                    float pt_truth  = (pt_truth_branch)  -> at(index_truth);

                    if (phi_truth > phi_region_min && phi_truth < phi_region_max && 
                        eta_truth > eta_region_min && eta_truth < eta_region_max &&
                        pt_truth > min_truth_pt && pt_truth < max_truth_pt){

                        ++truth_counter;
                    }
                }
                //std::cout << hough_counter << ", " << truth_counter << ", " << hough_counter/truth_counter << "\n" << std::endl;
                float hough_per_truth = float(hough_counter) / truth_counter;

                hist_gauss -> Fill(hough_per_truth);
            }
        }
    }

    canvas1 -> SetLogy();

    hist_gauss -> Draw("HIST");

    hist_raw -> Draw("HIST SAME");
    hist_raw -> SetLineColor(kRed);

    hist_order -> SetLineColor(kBlue);
    hist_order -> Draw("HIST SAME");

    hist_gauss -> Draw("HIST SAME");
    hist_gauss -> SetLineColor(kBlack);
    

    canvas1 -> SaveAs("compare_histograms_regions_pileup.pdf");
}