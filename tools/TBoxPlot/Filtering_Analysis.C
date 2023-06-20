void Filtering_Analysis(){

    // file names
    // Used are two detected-circles files so that output what filtering is enable can be
    // compared to default setting.
    // File hough_events.root contsint event_id which passed selection based on r and z regions.

    std::string file_name_ending = "single_10k_count_changes_3_wider_range";

    std::string truth_file_path =  "../../../DATA/ODD_Single_muon_10k/particles_initial.root";
    std::string hough_file_path =  "detected-circles/detected-circles_single_10k_without_filtering_wider_range.root";
    std::string hough_filtered_file_path =  "detected-circles/detected-circles_" + file_name_ending + ".root";
    std::string events_file_path =  "hough_events.root";
    std::string output_path = "output/output_histograms_filtering_analysis_" + file_name_ending + ".root";
    std::string pdf_file_name = "output/filtering_analysis_" + file_name_ending + ".pdf";

    std::string truth_tree_name = "particles";
    std::string hough_tree_name = "solutions";
    std::string hough_filtered_tree_name = "solutions";
    std::string events_tree_name = "tree";

    // initial lines
    using std::cout;
    using std::endl;
    delete gROOT -> FindObject("teff_pt_no_filtering_ok_events");
    delete gROOT -> FindObject("teff_phi_no_filtering_ok_events");
    delete gROOT -> FindObject("hist_pt_no_filtering");
    delete gROOT -> FindObject("hist_phi_no_filtering");
    delete gROOT -> FindObject("hist_nsolutions_no_filtering");

    delete gROOT -> FindObject("teff_pt_filtering_ok_events");
    delete gROOT -> FindObject("teff_phi_filtering_ok_events");
    delete gROOT -> FindObject("hist_pt_filtering");
    delete gROOT -> FindObject("hist_phi_filtering");
    delete gROOT -> FindObject("hist_nsolutions_filtering");

    TCanvas *canvas1 = new TCanvas("canvas1", "canvas1", 12000, 12000);
    //canvas1 -> Divide(4, 3);

    const float min_hist_pt = 0.;
    const float max_hist_pt = 11.;
    const float min_hist_phi = - 1.05 * M_PI;
    const float max_hist_phi =   1.05 * M_PI;
    const Int_t min_solutions_per_event = 0;
    const Int_t max_solutions_per_event = 42;

    const Int_t n_bins_efficiency = 50;
    const Int_t n_bins_hist = max_solutions_per_event - min_solutions_per_event;
    const Int_t n_bins_scatterplot = 200;

    // not filtered histograms
    //TEfficiency *teff_pt_no_filtering_ok_events  = new TEfficiency("teff_pt_no_filtering_ok_events", "Reconstruction efficiency of p_{T} (before);p_{T} truth [GeV];efficiency", n_bins_efficiency, min_hist_pt, max_hist_pt);
    TEfficiency *teff_pt_no_filtering_ok_events  = new TEfficiency("teff_pt_no_filtering_ok_events", ";p_{T} [GeV];efficiency", n_bins_efficiency, min_hist_pt, max_hist_pt);
    TEfficiency *teff_phi_no_filtering_ok_events = new TEfficiency("teff_phi_no_filtering_ok_event", "Reconstruction efficiency of #phi (before);#phi truth [rad];efficiency", n_bins_efficiency, min_hist_phi, max_hist_phi);

    TH2D *hist_pt_no_filtering  = new TH2D("hist_pt_no_filtering", "Solutions per event by p_{T} (before);p_{T} truth [GeV];count", n_bins_scatterplot, min_hist_pt, max_hist_pt, n_bins_scatterplot, min_solutions_per_event, max_solutions_per_event);
    TH2D *hist_phi_no_filtering = new TH2D("hist_phi_no_filtering", "Solutions per event by #phi (before);#phi truth [rad];count", n_bins_scatterplot, min_hist_phi, max_hist_phi, n_bins_scatterplot, min_solutions_per_event, max_solutions_per_event);

    TH1D *hist_nsolutions_no_filtering = new TH1D("hist_nsolutions_no_filtering", "Histogram of number of solutions (before);number of solutions;count", n_bins_hist, min_solutions_per_event, max_solutions_per_event);
    //TH1D *hist_nsolutions_no_filtering = new TH1D("hist_nsolutions_no_filtering", ";number of solutions;counts", n_bins_hist, min_solutions_per_event, max_solutions_per_event);

    // filtered histograms
    //TEfficiency *teff_pt_filtering_ok_events  = new TEfficiency("teff_pt_filtering_ok_events", "Reconstruction efficiency of p_{T} (after);p_{T} truth [GeV];efficiency", n_bins_efficiency, min_hist_pt, max_hist_pt);
    TEfficiency *teff_pt_filtering_ok_events  = new TEfficiency("teff_pt_filtering_ok_events", ";p_{T} [GeV];efficiency", n_bins_efficiency, min_hist_pt, max_hist_pt);
    TEfficiency *teff_phi_filtering_ok_events = new TEfficiency("teff_phi_filtering_ok_event", "Reconstruction efficiency of #phi (after);#phi truth [rad];efficiency", n_bins_efficiency, min_hist_phi, max_hist_phi);

    TH2D *hist_pt_filtering  = new TH2D("hist_pt_filtering", "Solutions per event by p_{T} (after);p_{T} truth [GeV];count", n_bins_scatterplot, min_hist_pt, max_hist_pt, n_bins_scatterplot, min_solutions_per_event, max_solutions_per_event);
    TH2D *hist_phi_filtering = new TH2D("hist_phi_filtering", "Solutions per event by #phi (after);#phi truth [rad];count", n_bins_scatterplot, min_hist_phi, max_hist_phi, n_bins_scatterplot, min_solutions_per_event, max_solutions_per_event);

    //TH1D *hist_nsolutions_filtering = new TH1D("hist_nsolutions_filtering", "Histogram of number of solutions (after);number of solutions;count", n_bins_hist, min_solutions_per_event, max_solutions_per_event);
    TH1D *hist_nsolutions_filtering = new TH1D("hist_nsolutions_filtering", ";number of solutions;counts", n_bins_hist, min_solutions_per_event, max_solutions_per_event);
    gPad 	-> 	SetLeftMargin(0.15);
    gStyle  ->  SetOptStat(0);

    // open files
    std::unique_ptr<TFile> file_truth(TFile::Open(truth_file_path.c_str()));
    std::unique_ptr<TFile> file_hough(TFile::Open(hough_file_path.c_str()));
    std::unique_ptr<TFile> file_hough_filtered(TFile::Open(hough_filtered_file_path.c_str()));
    std::unique_ptr<TFile> file_events(TFile::Open(events_file_path.c_str()));

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

    if (file_events == nullptr) {
        throw std::runtime_error("Can't open input file: " + events_file_path);
    }

    // access trees in the files
    std::unique_ptr<TTree> tree_truth(file_truth->Get<TTree>(truth_tree_name.c_str()));
    std::unique_ptr<TTree> tree_hough(file_hough->Get<TTree>(hough_tree_name.c_str()));
    std::unique_ptr<TTree> tree_hough_filtered(file_hough_filtered->Get<TTree>(hough_filtered_tree_name.c_str()));
    std::unique_ptr<TTree> tree_events(file_events->Get<TTree>(events_tree_name.c_str()));


    if (tree_truth == nullptr) {
        throw std::runtime_error("Can't access tree in the ROOT file: " + truth_file_path);
    }

    if (tree_hough == nullptr) {
        throw std::runtime_error("Can't access tree in the ROOT file: " + hough_file_path);
    }

    if (tree_hough_filtered == nullptr) {
        throw std::runtime_error("Can't access tree in the ROOT file: " + hough_filtered_file_path);
    }
    if (tree_events == nullptr) {
        throw std::runtime_error("Can't access tree in the ROOT file: " + events_file_path);
    }

    // set branch address - truth solutions
    UInt_t event_id_truth_branch;
    std::vector<float> *pt_truth_branch = 0;
    std::vector<float> *phi_truth_branch = 0;
    std::vector<float> *eta_truth_branch = 0;

    tree_truth -> SetBranchAddress("event_id", &event_id_truth_branch);
    tree_truth -> SetBranchAddress("pt", &pt_truth_branch);
    tree_truth -> SetBranchAddress("phi", &phi_truth_branch);
    tree_truth -> SetBranchAddress("eta", &eta_truth_branch);

    const Int_t all_truth_nentries = tree_truth -> GetEntries();

    tree_truth -> GetEntry(0);
    Int_t min_event_id = event_id_truth_branch;
    tree_truth -> GetEntry(all_truth_nentries - 1);
    Int_t max_event_id = event_id_truth_branch;

    // set branch address and access solution pairs - Hough algorithm, not filtered events
    UInt_t event_id_hough_branch;
    float pt_hough_branch;
    float phi_hough_branch;

    tree_hough -> SetBranchAddress("event_id", &event_id_hough_branch);
    tree_hough -> SetBranchAddress("pt", &pt_hough_branch);
    tree_hough -> SetBranchAddress("phi", &phi_hough_branch);

    const Int_t all_hough_nentries = tree_hough -> GetEntries();
    std::map<UInt_t, std::pair<std::vector<float>, std::vector<float>>> hough_events;

    // set branch address and access solution pairs - Hough algorithm, filtered events
    UInt_t event_id_hough_filtered_branch;
    float pt_hough_filtered_branch;
    float phi_hough_filtered_branch;

    tree_hough_filtered -> SetBranchAddress("event_id", &event_id_hough_filtered_branch);
    tree_hough_filtered -> SetBranchAddress("pt", &pt_hough_filtered_branch);
    tree_hough_filtered -> SetBranchAddress("phi", &phi_hough_filtered_branch);

    const Int_t all_hough_filtered_nentries = tree_hough_filtered -> GetEntries();
    std::map<UInt_t, std::pair<std::vector<float>, std::vector<float>>> hough_filtered_events;

    // set branch address and access solution pairs - events after selection
    Int_t event_id_events_branch;

    tree_events -> SetBranchAddress("event_id", &event_id_events_branch);
    const Int_t all_events_nentries = tree_events -> GetEntries();

    // saving data to std::vector makes it possible to search values
    std::vector<Int_t> vector_events;
    for (Int_t index = 0; index < all_events_nentries; ++index){

        tree_events -> GetEntry(index);
        Int_t current_event_id = event_id_events_branch;
        vector_events.push_back(current_event_id);
    }

    // move Hough (not filtered) pt and phi to vector<float>, sort by event_id
    for (Int_t index_hough = 0; index_hough < all_hough_nentries; ++index_hough){

        tree_hough -> GetEntry(index_hough);

        hough_events.try_emplace(event_id_hough_branch, std::pair<std::vector<float>, std::vector<float>>());
        hough_events[event_id_hough_branch].first.push_back(pt_hough_branch);
        hough_events[event_id_hough_branch].second.push_back(phi_hough_branch);
    }

    // move Hough (filtered) pt and phi to vector<float>, sort by event_id
    for (Int_t index_hough = 0; index_hough < all_hough_nentries; ++index_hough){

        tree_hough_filtered -> GetEntry(index_hough);

        hough_filtered_events.try_emplace(event_id_hough_filtered_branch, std::pair<std::vector<float>, std::vector<float>>());
        hough_filtered_events[event_id_hough_filtered_branch].first.push_back(pt_hough_filtered_branch);
        hough_filtered_events[event_id_hough_filtered_branch].second.push_back(phi_hough_filtered_branch);
    }

    // eta range - comparison between truth particles and accepted in further analysis (not rejected by r and z)
    float min_eta_truth{};
    float max_eta_truth{};
    for (Int_t event_index = 0; event_index < all_truth_nentries; ++event_index){

        tree_truth -> GetEntry(event_index);
        auto eta_current_vector = eta_truth_branch;

        for (Int_t index = 0; index < eta_current_vector -> size(); ++index){

            float eta_current = eta_current_vector -> at(index);
            if (eta_current > max_eta_truth){
                max_eta_truth = eta_current;
            } else if (eta_current < min_eta_truth){
                min_eta_truth = eta_current;
            }
        }
    }
    cout << "... Minimal value of eta in particles initial: " << min_eta_truth << ", maximal value: " << max_eta_truth << endl;

    float min_eta_Hough{};
    float max_eta_Hough{};

    for (Int_t index_Hough = 0; index_Hough < all_hough_nentries; ++index_Hough){

        tree_hough -> GetEntry(index_Hough);
        float event_id_current = event_id_hough_branch;
        tree_truth -> GetEntry(event_id_current);
        float truth_eta = eta_truth_branch -> at(0);

        if (truth_eta > max_eta_Hough){
            max_eta_Hough = truth_eta;
        } else if (truth_eta < min_eta_Hough){
            min_eta_Hough = truth_eta;
        }
    }
    cout << "... Minimal value of eta in Hough solutions: " << min_eta_Hough << ", maximal value: " << max_eta_Hough << "\n" << endl;

    ////////////////////////////////////////
    // efficiency for unfiltered events
    ////////////////////////////////////////

    // efficiency for ok events
    Int_t count_found_distinct_solutions_no_filtering{};
    for (const auto event_id : vector_events){

        tree_truth -> GetEntry(event_id);
        float pt_truth = (pt_truth_branch) -> at(0);
        float phi_truth = (phi_truth_branch) -> at(0);

        bool bool_tefficiency{};
        Int_t hough_solutions_current_event{};
        if (hough_events.find(event_id) != hough_events.end()){

            ++count_found_distinct_solutions_no_filtering;
            bool_tefficiency = 1;
            hough_solutions_current_event = hough_events[event_id].first.size();
        } else {
            cout << "... No solution found for event " << event_id << " before filtration." << endl;
        }

        teff_pt_no_filtering_ok_events  -> Fill(bool_tefficiency, pt_truth);
        teff_phi_no_filtering_ok_events -> Fill(bool_tefficiency, phi_truth);

        hist_nsolutions_no_filtering -> Fill(hough_solutions_current_event);

        hist_pt_no_filtering -> Fill(pt_truth, hough_solutions_current_event);
        hist_phi_no_filtering -> Fill(phi_truth, hough_solutions_current_event);
    }
    cout << "\n";


    ////////////////////////////////////////
    // efficiency for filtered events
    ////////////////////////////////////////

    // efficiency for ok events
    Int_t count_found_distinct_solutions_filtering{};
    for (const auto event_id : vector_events){

        tree_truth -> GetEntry(event_id);
        float pt_truth = (pt_truth_branch) -> at(0);
        float phi_truth = (phi_truth_branch) -> at(0);

        bool bool_tefficiency{};
        Int_t hough_solutions_current_event{};
        if (hough_filtered_events.find(event_id) != hough_filtered_events.end()){

            ++count_found_distinct_solutions_filtering;
            bool_tefficiency = 1;
            hough_solutions_current_event = hough_filtered_events[event_id].first.size();
        } else {
            cout << "... No solution found for event " << event_id << " after filtration." << endl;
        }

        teff_pt_filtering_ok_events  -> Fill(bool_tefficiency, pt_truth);
        teff_phi_filtering_ok_events -> Fill(bool_tefficiency, phi_truth);

        hist_nsolutions_filtering -> Fill(hough_solutions_current_event);

        hist_pt_filtering -> Fill(pt_truth, hough_solutions_current_event);
        hist_phi_filtering -> Fill(phi_truth, hough_solutions_current_event);
    }

    // general statistics
    cout << "\n... Total number of solutions found without filtering: " << all_hough_nentries << endl;
    cout << "... Total number of solutions found with filtering on: " << all_hough_filtered_nentries << endl;
    cout << "... Reduction in number of solutions: " << 100*float(all_hough_nentries - all_hough_filtered_nentries)/all_hough_nentries << "%\n" << endl;

    cout << "... Average number of solutions found for single particle without filtering: " << float(all_hough_nentries) / count_found_distinct_solutions_no_filtering << endl;
    cout << "... Average number of solutions found for single particle with filtering on: " << float(all_hough_filtered_nentries) / count_found_distinct_solutions_filtering << "\n" << endl;

    cout << "... Solutions for " << count_found_distinct_solutions_no_filtering << "/" << all_events_nentries << " were found witout using filtering." << endl;
    cout << "... Solutions for " << count_found_distinct_solutions_filtering << "/" << all_events_nentries << " were found using filtering on.\n" << endl;


    // save all the histograms to ROOT file
    teff_pt_no_filtering_ok_events -> Write();
    teff_phi_no_filtering_ok_events -> Write();
    hist_pt_no_filtering -> Write();
    hist_phi_no_filtering -> Write();
    hist_nsolutions_no_filtering -> Write();
    teff_pt_filtering_ok_events -> Write();
    teff_phi_filtering_ok_events -> Write();
    hist_pt_filtering -> Write();
    hist_phi_filtering -> Write();
    hist_nsolutions_filtering -> Write();

    file_output -> Close();

    // plot all the histograms

    //hist_nsolutions_no_filtering -> Draw("HIST");
    //teff_pt_np_filtering_ok_events -> Draw();

    //teff_pt_filtering_ok_events -> Draw();
    hist_nsolutions_filtering -> Draw("HIST");
/*
    // // q/pt
    // not filtered
    canvas1 -> cd(1);
    teff_pt_no_filtering_ok_events -> Draw();

    canvas1 -> cd(5);
    hist_pt_no_filtering -> Draw("SCAT");

    canvas1 -> cd(9);
    hist_nsolutions_no_filtering -> Draw("HIST");

    // filtered
    canvas1 -> cd(2);
    teff_pt_filtering_ok_events -> Draw();

    canvas1 -> cd(6);
    hist_pt_filtering -> Draw("SCAT");

    canvas1 -> cd(10);
    hist_nsolutions_filtering -> Draw("HIST");

    // // phi
    // not filtered

    canvas1 -> cd(3);
    teff_phi_no_filtering_ok_events -> Draw();

    canvas1 -> cd(7);
    hist_phi_no_filtering -> Draw("SCAT");

    // filtered
    canvas1 -> cd(4);
    teff_phi_filtering_ok_events -> Draw();

    canvas1 -> cd(8);
    hist_phi_filtering -> Draw("SCAT");
*/

    // save the canvas to pdf file
    canvas1 -> SaveAs(pdf_file_name.c_str());

}