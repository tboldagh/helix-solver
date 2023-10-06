void Compare_n_solutions_Histograms(){

    std::string hough_file_path_raw =  "detected-circles/compare_histograms_regions_raw.root";
    std::string hough_file_path_order =  "detected-circles/compare_histograms_regions_order.root";
    std::string hough_file_path_gauss =  "detected-circles/compare_histograms_regions_gauss.root";

    std::string hough_tree_name = "solutions";

    using std::cout;
    using std::endl;
    delete gROOT -> FindObject("hist_raw");
    delete gROOT -> FindObject("hist_order");
    delete gROOT -> FindObject("hist_gauss");

    TCanvas *canvas1 = new TCanvas("canvas1", "canvas1", 12000, 12000);

    const Int_t n_regions_phi = 8;
    const Int_t n_regions_eta = 39;
    const float excess_wedge_phi_width = 0.12;
    const float excess_wedge_eta_width = 0.11;
    const float phi_center = M_PI;
    const float phi_width = 10; //(2*M_PI/n_regions_phi) * 0.5 + excess_wedge_phi_width;
    const float eta_center = 0.4;
    const float eta_width = 10; //= (2*4/n_regions_eta) * 0.5 + excess_wedge_eta_width;;

    const uint32_t ok_events = 2573;
    const uint8_t n_solutions_min = 0;
    const uint8_t n_solutions_max = 100;
    const uint8_t n_bins = (n_solutions_max - n_solutions_min)/2;
    TH1D *hist_raw = new TH1D("hist_raw", "", n_bins, n_solutions_min, n_solutions_max);
    TH1D *hist_order = new TH1D("hist_order", "", n_bins, n_solutions_min, n_solutions_max);
    TH1D *hist_gauss = new TH1D("hist_gauss", ";number of solutions;counts", n_bins, n_solutions_min, n_solutions_max);
    gStyle  ->  SetOptStat(0);

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

    // raw solutions
    UInt_t event_id_hough_branch_raw;
    float eta_hough_branch_raw;
    float phi_hough_branch_raw;
    tree_hough_raw -> SetBranchAddress("event_id", &event_id_hough_branch_raw);
    tree_hough_raw -> SetBranchAddress("eta", &eta_hough_branch_raw);
    tree_hough_raw -> SetBranchAddress("phi", &phi_hough_branch_raw);
    const Int_t nentries_hough_raw = tree_hough_raw -> GetEntries();
    std::map<UInt_t, std::pair<std::vector<float>, std::vector<float>>> hough_events_raw;
    std::cout << "Total number of solutions without any optimalization: " << nentries_hough_raw << " (" << float(nentries_hough_raw)/ok_events << ")" << std::endl;

    // solutions after dividing in terms of regions
    UInt_t event_id_hough_branch_order;
    float eta_hough_branch_order;
    float phi_hough_branch_order;
    tree_hough_order -> SetBranchAddress("event_id", &event_id_hough_branch_order);
    tree_hough_order -> SetBranchAddress("eta", &eta_hough_branch_order);
    tree_hough_order -> SetBranchAddress("phi", &phi_hough_branch_order);
    std::map<UInt_t, std::pair<std::vector<float>, std::vector<float>>> hough_events_order;
    const Int_t nentries_hough_order = tree_hough_order -> GetEntries();
    std::cout << "Total number of solutions for order checking: " << nentries_hough_order << " (" << float(nentries_hough_order)/ok_events << ")" << std::endl;

    // solutions after dividing in terms of regions and applyign Gauss filtering
    UInt_t event_id_hough_branch_gauss;
    float eta_hough_branch_gauss;
    float phi_hough_branch_gauss;
    tree_hough_gauss -> SetBranchAddress("event_id", &event_id_hough_branch_gauss);
    tree_hough_gauss -> SetBranchAddress("eta", &eta_hough_branch_gauss);
    tree_hough_gauss -> SetBranchAddress("phi", &phi_hough_branch_gauss);
    std::map<UInt_t, std::pair<std::vector<float>, std::vector<float>>> hough_events_gauss;
    const Int_t nentries_hough_gauss = tree_hough_gauss -> GetEntries();
    std::cout << "Total number of solutions for order checking and gauss filtering: " << nentries_hough_gauss << " (" << float(nentries_hough_gauss)/ok_events << ")\n" << std::endl;

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

        auto solution_pair = event.second;

        std::vector<float> solution_eta = solution_pair.first;
        std::vector<float> solution_phi = solution_pair.second;

        double n_solutions_per_event = solution_phi.size();
        float avg_phi{};
        float avg_eta{};

        for (auto phi : solution_phi){
            avg_phi += phi;
        }

        for (auto eta : solution_eta){
            avg_eta += eta;
        }

        avg_phi /= n_solutions_per_event;
        avg_eta /= n_solutions_per_event;

        if (fabs(avg_phi - phi_center) < phi_width && fabs(avg_eta - eta_center) < eta_width){
            hist_raw -> Fill(n_solutions_per_event);
        }
    }

    // fill order check histogram
    for (auto event : hough_events_order){

        auto solution_pair = event.second;

        std::vector<float> solution_eta = solution_pair.first;
        std::vector<float> solution_phi = solution_pair.second;

        double n_solutions_per_event = solution_phi.size();
        float avg_phi{};
        float avg_eta{};

        for (auto phi : solution_phi){
            avg_phi += phi;
        }

        for (auto eta : solution_eta){
            avg_eta += eta;
        }

        avg_phi /= n_solutions_per_event;
        avg_eta /= n_solutions_per_event;

        if (fabs(avg_phi - phi_center) < phi_width && fabs(avg_eta - eta_center) < eta_width){
            hist_order -> Fill(n_solutions_per_event);
        }
    }

    // fill order check and Gauss filtering histogram
    for (auto event : hough_events_gauss){

        auto solution_pair = event.second;

        std::vector<float> solution_eta = solution_pair.first;
        std::vector<float> solution_phi = solution_pair.second;

        double n_solutions_per_event = solution_phi.size();
        float avg_phi{};
        float avg_eta{};

        for (auto phi : solution_phi){
            avg_phi += phi;
        }

        for (auto eta : solution_eta){
            avg_eta += eta;
        }

        avg_phi /= n_solutions_per_event;
        avg_eta /= n_solutions_per_event;

        if (fabs(avg_phi - phi_center) < phi_width && fabs(avg_eta - eta_center) < eta_width){
            hist_gauss -> Fill(n_solutions_per_event);
        }
    }

    canvas1 -> SetLogy();

    hist_gauss -> Draw("HIST");
    hist_gauss -> SetLineColor(kBlack);

    hist_order -> SetLineColor(kBlue);
    hist_order -> Draw("HIST SAME");

    hist_raw -> Draw("HIST SAME");
    hist_raw -> SetLineColor(kRed);

    canvas1 -> SaveAs("compare_histograms_regions.pdf");
}