void Reconstruction_Efficiency_Single(){

// file names
    std::string truth_file_path =  "particles_initial/particles_initial_single_10k.root";
    std::string hough_file_path =  "detected-circles/detected-circles_this_should_work.root";
    std::string spacepoints_file_path =  "spacepoints/spacepoints_single_10k.root";
    std::string output_path = "output_histograms_single_10k_this_should_work.root";

    std::string truth_tree_name = "particles";
    std::string hough_tree_name = "solutions";
    std::string spacepoints_tree_name = "spacepoints";

    // if equal to yes then for each event information on number of solutions found fill be displaye
    const bool DISPLAY_EACH_HOUGH_EVENT_STATS = 0;

// initial - delete objects, define canvas
    using std::cout;
    using std::endl;
    delete gROOT -> FindObject("canvas1");
    delete gROOT -> FindObject("scatt_pt");
    delete gROOT -> FindObject("scatt_phi");
    delete gROOT -> FindObject("scatt_delta_pt");
    delete gROOT -> FindObject("scatt_delta_phi");
    delete gROOT -> FindObject("teff_pt");
    delete gROOT -> FindObject("teff_phi");
    delete gROOT -> FindObject("teff_eta");
    delete gROOT -> FindObject("hist_delta_phi");
    delete gROOT -> FindObject("hist_n_solutions");
    delete gROOT -> FindObject("scatt_nhits_eta");
    delete gROOT -> FindObject("scatt_detector_geo");

    TCanvas *canvas1 = new TCanvas("canvas1", "canvas1", 2000, 2000);
    canvas1 -> Divide(3, 3);


// define histograms
    const float min_hist_pt = 0.;
    const float max_hist_pt = 11.;
    const float min_hist_phi = - M_PI;
    const float max_hist_phi =   M_PI;
    const float min_hist_eta = - 4;
    const float max_hist_eta =   4;
    const float min_hist_delta_phi = 0;
    const float max_hist_delta_phi = 0.01;
    const float min_hist_n_solutions = 1;
    const float max_hist_n_solutions = 45;

    const float min_delta_pt = -8;
    const float max_delta_pt =  12;
    const float min_delta_phi = -0.03;
    const float max_delta_phi =  0.03;
    const uint8_t min_n_hits = 5;
    const uint8_t max_n_hits =  22;
    const float r_min = 0;
    const float r_max = 1150;
    const float z_max = 3300;
    const float z_min = - z_max;


    const Int_t n_bins_scatterplot = 200;
    const Int_t n_bins_efficiency = 50;
    const Int_t n_bins_hist = 50;

    TH2D *scatt_pt   = new TH2D("scatt_pt", "Scatterplot of p_{T} - truth vs Hough solutions;p_{t} truth;p_{T} Hough", n_bins_scatterplot, min_hist_pt, max_hist_pt, n_bins_scatterplot, min_hist_pt, max_hist_pt + 9);
    TH2D *scatt_phi  = new TH2D("scatt_phi", "Scatterplot of #phi - truth vs Hough solutions;#phi truth;#phi Hough", n_bins_scatterplot, min_hist_phi, max_hist_phi, n_bins_scatterplot, min_hist_phi, max_hist_phi);

    TH2D *scatt_delta_pt  = new TH2D("scatt_delta_pt", "Scatterplot of #Delta p_{T} = p_{T} truth - p_{T} Hough vs p_{T} truth;p_{T} truth [GeV];precision", n_bins_efficiency, min_hist_pt, max_hist_pt, n_bins_efficiency, min_delta_pt, max_delta_pt);
    TH2D *scatt_delta_phi = new TH2D("scatt_delta_phi", "Scatterplot of #Delta#phi = #phi_{truth} - #phi_{Hough} vs #phi_{truth};phi truth [rad];precision", n_bins_efficiency, min_hist_phi, max_hist_phi, n_bins_efficiency, min_delta_phi, max_delta_phi);

    TEfficiency *teff_pt  = new TEfficiency("teff_pt", "Reconstruction efficiency of p_{T};p_{T} truth [GeV];efficiency", n_bins_efficiency, min_hist_pt, max_hist_pt);
    TEfficiency *teff_phi = new TEfficiency("teff_phi", "Reconstruction efficiency of #phi;#phi truth [rad];efficiency", n_bins_efficiency, min_hist_phi, max_hist_phi);
    TEfficiency *teff_eta = new TEfficiency("teff_eta", "Reconstruction efficiency of #eta;#eta truth;efficiency", n_bins_efficiency, min_hist_eta, max_hist_eta);

    TH1D *hist_delta_phi = new TH1D("hist_delta_phi", "Histogram of #Delta#phi = #phi_{truth} - #phi_{Hough};#Delta#phi;counts", n_bins_hist, min_hist_delta_phi, max_hist_delta_phi);
    TH1D *hist_n_solutions = new TH1D("hist_n_solutions", "Histogram of Hough solutions / single muon;number of solutions;counts", max_hist_n_solutions - min_hist_n_solutions + 1, min_hist_n_solutions, max_hist_n_solutions);
    TH2D *scatt_nhits_eta = new TH2D("scatt_nhits_eta", "Scatterplot of n_hits vs #eta;#eta truth;Number of spacepoints", n_bins_efficiency, min_hist_eta, max_hist_eta, n_bins_efficiency, min_n_hits, max_n_hits);
    TH2D *scatt_detector_geo = new TH2D("scatt_detector_geo", "Detector geometry;z [mm];r [mm]", n_bins_scatterplot, z_min, z_max, n_bins_scatterplot, r_min, r_max);

    gStyle->SetOptStat(0);


// open file - solutions reconstructed particles_initial_single_1k.root
    cout << "\n.. Truth data file: " << truth_file_path << endl;
    cout << ".. Hough solutions data file: " << hough_file_path << endl;
    cout << ".. Spacepoints data file: " << spacepoints_file_path << endl;

    std::unique_ptr<TFile> file_truth(TFile::Open(truth_file_path.c_str()));
    std::unique_ptr<TFile> file_hough(TFile::Open(hough_file_path.c_str()));
    std::unique_ptr<TFile> file_spacepoints(TFile::Open(spacepoints_file_path.c_str()));

    // root file, where all the output histograms will be saved
    TFile* file_output = new TFile(output_path.c_str(), "RECREATE");

    if (file_truth == nullptr) {
        throw std::runtime_error("Can't open input file: " + truth_file_path);
    }

    if (file_hough == nullptr) {
        throw std::runtime_error("Can't open input file: " + hough_file_path);
    }

    if (file_spacepoints == nullptr) {
        throw std::runtime_error("Can't open input file: " + spacepoints_file_path);
    }


// access trees in the files
    std::unique_ptr<TTree> tree_truth(file_truth->Get<TTree>(truth_tree_name.c_str()));
    std::unique_ptr<TTree> tree_hough(file_hough->Get<TTree>(hough_tree_name.c_str()));
    std::unique_ptr<TTree> tree_spacepoints(file_spacepoints->Get<TTree>(spacepoints_tree_name.c_str()));

    if (tree_truth == nullptr) {
        throw std::runtime_error("Can't access tree in the ROOT file: " + truth_file_path);
    }

    if (tree_hough == nullptr) {
        throw std::runtime_error("Can't access tree in the ROOT file: " + hough_file_path);
    }

    if (tree_spacepoints == nullptr) {
        throw std::runtime_error("Can't access tree in the ROOT file: " + spacepoints_file_path);
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


// set branch address and access solution pairs - Hough algorithm
    UInt_t event_id_hough_branch;
    float pt_hough_branch;
    float phi_hough_branch;

    tree_hough -> SetBranchAddress("event_id", &event_id_hough_branch);
    tree_hough -> SetBranchAddress("pt", &pt_hough_branch);
    tree_hough -> SetBranchAddress("phi", &phi_hough_branch);

    const Int_t all_hough_nentries = tree_hough -> GetEntries();
    std::map<UInt_t, std::pair<std::vector<float>, std::vector<float>>> hough_events;


// move Hough pt and phi to vector<float>, sort by event_id
    for (Int_t index_hough = 0; index_hough < all_hough_nentries; ++index_hough){

        tree_hough -> GetEntry(index_hough);

        hough_events.try_emplace(event_id_hough_branch, std::pair<std::vector<float>, std::vector<float>>());
        hough_events[event_id_hough_branch].first.push_back(pt_hough_branch);
        hough_events[event_id_hough_branch].second.push_back(phi_hough_branch);
    }


// set branch address and access solution pairs - spacepoints
    UInt_t event_id_spacepoints_branch;
    float x_coord_spacepoints_branch;
    float y_coord_spacepoints_branch;
    float z_coord_spacepoints_branch;

    tree_spacepoints -> SetBranchAddress("event_id", &event_id_spacepoints_branch);
    tree_spacepoints -> SetBranchAddress("x", &x_coord_spacepoints_branch);
    tree_spacepoints -> SetBranchAddress("y", &y_coord_spacepoints_branch);
    tree_spacepoints -> SetBranchAddress("z", &z_coord_spacepoints_branch);

    const Int_t all_spacepoints_nentries = tree_spacepoints -> GetEntries();
    std::map<UInt_t, std::pair<std::vector<float>, std::vector<float>>> spacepoints_nhits;


// move spacepoints to vector<float>, sort by event_id
    for (Int_t index_spacepoints = 0; index_spacepoints < all_spacepoints_nentries; ++index_spacepoints){

        tree_spacepoints -> GetEntry(index_spacepoints);

        float x = x_coord_spacepoints_branch;
        float y = y_coord_spacepoints_branch;
        float z = z_coord_spacepoints_branch;

        spacepoints_nhits.try_emplace(event_id_spacepoints_branch, std::pair<std::vector<float>, std::vector<float>>());
        spacepoints_nhits[event_id_spacepoints_branch].first.push_back(z);
        spacepoints_nhits[event_id_spacepoints_branch].second.push_back(TMath::Sqrt(x*x + y * y));
    }

// calculate efficiencies
    Int_t bool_tefficiency{};
    const float symmetric_cut_on_eta = 3.;
    const float symmetric_cut_on_phi = 4.;
    std::vector<float> all_delta_pt;
    std::vector<float> all_delta_phi;
    std::map<Int_t, std::vector<float>> all_delta_by_event_pt;
    std::map<Int_t, std::vector<float>> all_delta_by_event_phi;

    tree_truth -> GetEntry(0);
    Int_t min_event_id = event_id_truth_branch;
    tree_truth -> GetEntry(all_truth_nentries - 1);
    Int_t max_event_id = event_id_truth_branch;
    tree_spacepoints -> GetEntry(all_spacepoints_nentries - 1);
    Int_t max_event_id_spacepoints = event_id_spacepoints_branch;

    for (Int_t event_id = min_event_id; event_id <= max_event_id; event_id++){

        tree_truth -> GetEntry(event_id - min_event_id);

        float pt_truth = (pt_truth_branch) -> at(0);
        float phi_truth = (phi_truth_branch) -> at(0);
        float eta_truth = (eta_truth_branch) -> at(0);

        if (std::fabs(eta_truth) > symmetric_cut_on_eta) continue;
        if (std::fabs(phi_truth) > symmetric_cut_on_phi) continue;

        // check if there is any reconstructed particle for a given truth event_id
        bool_tefficiency = 0;
        if (hough_events.find(event_id) != hough_events.end()){

            auto solution_pair_hough = hough_events.at(event_id);
            std::vector<float> pt_hough = solution_pair_hough.first;
            std::vector<float> phi_hough = solution_pair_hough.second;
            const Int_t hough_solutions_current_event = pt_hough.size();

            // fill histogram with efficiency for each truth-hough solution pair
            for (Int_t hough_index = 0; hough_index < hough_solutions_current_event; ++hough_index){

                bool_tefficiency = 1;
                scatt_pt  -> Fill(pt_truth, pt_hough.at(hough_index));
                scatt_phi -> Fill(phi_truth, phi_hough.at(hough_index));

                float delta_pt  = pt_hough.at(hough_index) - pt_truth;
                float delta_phi = phi_hough.at(hough_index) - phi_truth;

                all_delta_pt.push_back(delta_pt);
                all_delta_phi.push_back(delta_phi);

                all_delta_by_event_pt.try_emplace(event_id, std::vector<float>());
                all_delta_by_event_pt[event_id].push_back(delta_pt);
                all_delta_by_event_pt.try_emplace(event_id, std::vector<float>());
                all_delta_by_event_phi[event_id].push_back(delta_phi);

                hist_delta_phi -> Fill(delta_phi);
                scatt_delta_pt  -> Fill(pt_truth, delta_pt);
                scatt_delta_phi -> Fill(phi_truth, delta_phi);
            }
        }
        teff_pt -> Fill(bool_tefficiency, pt_truth);
        teff_phi -> Fill(bool_tefficiency, phi_truth);
        teff_eta -> Fill(bool_tefficiency, eta_truth);

        auto z_nhits = spacepoints_nhits[event_id].first;
        auto r_nhits = spacepoints_nhits[event_id].second;
        float nhits_event_nentries = z_nhits.size();
        scatt_nhits_eta -> Fill(eta_truth, nhits_event_nentries);

        for (Int_t index_nhits = 0; index_nhits < nhits_event_nentries; ++index_nhits){
            scatt_detector_geo -> Fill(z_nhits.at(index_nhits), r_nhits.at(index_nhits));
        }
    }


// print files summary
    cout << "\n.. File " << truth_file_path << " contains " << tree_truth -> GetEntries() << " events" << endl;
    cout << ".. File " << hough_file_path << " contains " << hough_events.size() << " events and " << tree_hough -> GetEntries() << " solutions" << endl;
    cout << ".. File " << spacepoints_file_path << " contains " << (max_event_id_spacepoints + 1) <<  " events and " << tree_spacepoints -> GetEntries() << " spacepoints" << endl;

    cout<< "\n.. Average number of solutions for a single truth particle: " << float(tree_hough -> GetEntries()) / hough_events.size() << endl;
    cout << ".. Cut in eta set for |eta| > " << symmetric_cut_on_eta << endl;
    cout << ".. Cut in phi set for |phi| > " << symmetric_cut_on_phi << endl;

    for (const auto& hough_event : hough_events){

        Int_t event_id_cout = hough_event.first;
        tree_hough -> GetEntry(event_id_cout);

        auto solution_pair = hough_event.second;
        std::vector<float> pt_hough = solution_pair.first;

        Int_t nentries_event = pt_hough.size();
        hist_n_solutions -> Fill(nentries_event);

        if (DISPLAY_EACH_HOUGH_EVENT_STATS){
            cout << ".. Event " << event_id_cout << " - hough solutions: " << nentries_event << endl;
        }
    }


    // basic stats of pt and phi solutions discrepancy
    TH1D *scatt_delta_pt_y_projection = scatt_delta_pt -> ProjectionY();
    float mean_pt = scatt_delta_pt_y_projection -> GetMean();
    float sd_pt = scatt_delta_pt_y_projection -> GetRMS();
    cout << "\n.. Average pt reconstruction discrepancy: " << mean_pt << endl;
    cout << ".. Stdev pt reconstruction discrepancy: " << sd_pt << endl;

    TH1D *scatt_delta_phi_y_projection = scatt_delta_phi -> ProjectionY();
    float mean_phi = scatt_delta_phi_y_projection -> GetMean();
    float sd_phi = scatt_delta_phi_y_projection -> GetRMS();
    cout << "\n.. Average phi reconstruction discrepancy: " << mean_phi << endl;
    cout << ".. Stdev phi reconstruction discrepancy: " << sd_phi << endl;

    // percentage of reconstructed particles - total
    auto teff_pt_passed = teff_pt -> GetCopyPassedHisto();
    auto teff_pt_total = teff_pt -> GetCopyTotalHisto();
    float passed_events = teff_pt_passed -> GetEntries();
    float total_events = teff_pt_total -> GetEntries();
    cout << "\n.. Number of reconstructed particles (eta < " << symmetric_cut_on_eta << "): " << passed_events << "/" << total_events << " = " << TMath::Nint(10000. * passed_events/total_events)/100. << "%\n" <<endl;


    // delta pt and phi required to account for p percent of all Hough solutions
    sort(all_delta_pt.begin(), all_delta_pt.end());
    sort(all_delta_phi.begin(), all_delta_phi.end());
    std::vector<std::pair<float, float>> delta_threshold;

    float delta_nentries = all_delta_pt.size();
    std::vector<float> percentage_delta = {0.9, 0.95, 0.99, 0.999, 1};
    for (const float p : percentage_delta){

        delta_threshold.push_back(std::make_pair(all_delta_pt.at(p*(delta_nentries - 1)), all_delta_phi.at(p*(delta_nentries - 1))));
        cout << ".. To include at least " << 100 * p << "\% of all Hough solutions -> delta pt > " << all_delta_pt.at(p*(delta_nentries - 1)) << ", delta phi > " << all_delta_phi.at(p*(delta_nentries - 1)) << endl;
    }
    cout << endl;


    // delta pt and phi required to account for p percent of all truth solutions
    std::vector<float> min_delta_pt_each_event;
    std::vector<float> min_delta_phi_each_event;

    for (const auto event_delta : all_delta_by_event_pt){

        std::vector<float> pt_delta = event_delta.second;
        auto max_pt = *std::min_element(pt_delta.begin(), pt_delta.end());
        min_delta_pt_each_event.push_back(max_pt);
    }

    for (const auto event_delta : all_delta_by_event_phi){

        std::vector<float> phi_delta = event_delta.second;
        auto max_phi = *std::min_element(phi_delta.begin(), phi_delta.end());
        min_delta_phi_each_event.push_back(max_phi);
    }

    Int_t nentries_delta = min_delta_phi_each_event.size();
    for (const auto delta_pair : delta_threshold){

        Int_t count_passed = 0;
        for (Int_t index_delta = 0; index_delta < nentries_delta; ++index_delta){

            if (min_delta_pt_each_event.at(index_delta) <= delta_pair.first && min_delta_pt_each_event.at(index_delta) <= delta_pair.second){
                ++count_passed;
            }
        }
        cout << ".. Condition of delta pt < " << delta_pair.first << " and delta phi < " << delta_pair.second << " results in " << TMath::Nint(1000000. * float(count_passed) / nentries_delta)/10000. << "\% of events accepted" << endl;
    }
    cout << endl;


    float min_delta_pt_accepting_all{};
    float min_delta_phi_accepting_all{};
    for (Int_t index_delta = 0; index_delta < nentries_delta; ++index_delta){

        min_delta_pt_accepting_all = (min_delta_pt_each_event[index_delta] > min_delta_pt_accepting_all) ? min_delta_pt_each_event[index_delta] : min_delta_pt_accepting_all;
        min_delta_phi_accepting_all = (min_delta_phi_each_event[index_delta] > min_delta_phi_accepting_all) ? min_delta_phi_each_event[index_delta] : min_delta_phi_accepting_all;
    }
    cout << ".. Minimal delta pt and delta phi required to accept all the events: delta pt = " << min_delta_pt_accepting_all << ", delta phi = " << min_delta_phi_accepting_all << "\n" <<endl;


    // save all histograms to root file
    scatt_pt -> Write();
    scatt_phi -> Write();
    scatt_delta_pt -> Write();
    scatt_delta_phi -> Write();
    teff_pt -> Write();
    teff_phi -> Write();
    teff_eta -> Write();
    hist_delta_phi -> Write();
    hist_n_solutions -> Write();
    scatt_nhits_eta -> Write();
    scatt_detector_geo -> Write();

    file_output -> Close();


// draw phi histograms, save canvas to pdf file, close files
    // phi histograms
    canvas1 -> cd(1);
    scatt_phi -> Draw("SCAT");

    canvas1 -> cd(2);
    scatt_delta_phi -> Draw("SCAT");

    canvas1 -> cd(3);
    teff_phi -> Draw("AP");

    canvas1 -> cd(7);
    hist_delta_phi -> Draw("HIST");


    // pt histograms
    canvas1 -> cd(4);
    scatt_pt -> Draw("SCAT");

    canvas1 -> cd(5);
    scatt_delta_pt -> Draw("SCAT");

    canvas1 -> cd(6);
    teff_pt -> Draw("AP");


    // other histograms
    canvas1 -> cd(8);
    teff_eta -> Draw("AP");

    canvas1 -> cd(9);
    //hist_n_solutions -> Draw("HIST");
    scatt_nhits_eta -> Draw("SCAT");
    //scatt_detector_geo -> Draw("SCAT");

    canvas1 -> SaveAs("solutions_reconstruction_efficiency_single_one_file.pdf");
    cout << endl;
}