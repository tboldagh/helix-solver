void Reconstruction_Efficiency(
    const float PT_PRECISION = 0.5,
    const float PHI_PRECISION = 0.1
){


// initial - delete objects, define canvas
    delete gROOT -> FindObject("canvas1");
    delete gROOT -> FindObject("hist_pt");
    delete gROOT -> FindObject("hist_phi");

    delete gROOT -> FindObject("pt_efficiency");
    delete gROOT -> FindObject("phi_efficiency");

    TCanvas *canvas1 = new TCanvas("canvas1", "canvas1", 800, 1000);
    canvas1 -> Divide(2, 2);


// define histograms
    const float min_hist_pt = 0.;
    const float max_hist_pt = 10.;
    const float min_hist_phi = - M_PI;
    const float max_hist_phi =   M_PI;

    const Int_t n_bins_th2d = 200;
    const Int_t n_bins_tefficiency = 50;

    TH2D *hist_pt_scatterplot   = new TH2D("hist_pt", "Efficiency of p_{T} - truth vs Hough solutions;p_{t} truth;p_{T} Hough", n_bins_th2d, min_hist_pt, max_hist_pt, n_bins_th2d, min_hist_pt, max_hist_pt);
    TH2D *hist_phi_scatterplot  = new TH2D("hist_phi", "Efficiency of #phi - truth vs Hough solutions;#phi truth;#phi Hough", n_bins_th2d, min_hist_phi, max_hist_phi, n_bins_th2d, min_hist_phi, max_hist_phi);

    TEfficiency *pt_efficiency  = new TEfficiency("pt_efficiency", "p_{T} reconstruction efficiency;p_{T} [GeV];efficiency", n_bins_tefficiency, min_hist_pt, max_hist_pt);
    TEfficiency *phi_efficiency = new TEfficiency("phi_efficiency", "#phi reconstruction efficiency;#phi [rad];efficiency", n_bins_tefficiency, min_hist_phi, max_hist_phi);

    gStyle->SetOptStat(0);


// open file - solutions reconstructed
    std::string truth_filename =  "fatras_particles_initial.root";
    std::string hough_filename =  "detected-circles.root";

    cout << "\n.. Truth data file: " << truth_filename << endl;
    cout << ".. Hough solutions data file: " << hough_filename << endl;

    TFile *file_truth  =  new TFile(truth_filename.c_str());
    assert(file_truth);
    TFile *file_hough  =  new TFile(hough_filename.c_str());
    assert(file_hough);


// access trees in the files
    TTree *tree_truth  =  (TTree*)file_truth->Get("particles");
    TTree *tree_hough  =  (TTree*)file_hough->Get("solutions");


// set branch address and access solution pairs - Hough algorithm
    UInt_t event_id_hough_branch;
    float pt_hough_branch;
    float phi_hough_branch;

    tree_hough -> SetBranchAddress("event_id", &event_id_hough_branch);
    tree_hough -> SetBranchAddress("pt", &pt_hough_branch);
    tree_hough -> SetBranchAddress("phi", &phi_hough_branch);

    const Int_t all_hough_nentries = tree_hough -> GetEntries();
    std::map<UInt_t, std::pair<std::vector<float>, std::vector<float>>> hough_events;
    //std::vector<UInt_t> hough_events_id;    // to store


// move Hough pt and phi to vector<float>, sort by event_id
    for (Int_t index_hough = 0; index_hough < all_hough_nentries; ++index_hough){

        tree_hough -> GetEntry(index_hough);

        hough_events.try_emplace(event_id_hough_branch, std::pair<std::vector<float>, std::vector<float>>());
        hough_events[event_id_hough_branch].first.push_back(pt_hough_branch);
        hough_events[event_id_hough_branch].second.push_back(phi_hough_branch);

    }


// set branch address - truth solutions
    UInt_t event_id_truth_branch;
    std::vector<float> *pt_truth_branch = 0;
    std::vector<float> *phi_truth_branch = 0;

    tree_truth -> SetBranchAddress("event_id", &event_id_hough_branch);
    tree_truth -> SetBranchAddress("pt", &pt_truth_branch);
    tree_truth -> SetBranchAddress("phi", &phi_truth_branch);


// containers definition
    std::map<Int_t, std::vector<float>> count_solutions_per_truth;
    std::map<Int_t, Int_t> hough_solutions_identified;  // couts number of solutions in each event succesfully identified


// calculate efficiencies
    bool bool_efficiency = 0;   // argument in Fill methods of TEfficiency, bool_efficiency = 1 if Hough
    for (const auto& hough_event : hough_events){

        Int_t current_event_id = hough_event.first;
        auto solution_pair = hough_event.second;

        // solutions vectors (pt and phi) for the analysed event_id
        std::vector<float> pt_searched_vector = solution_pair.first;
        std::vector<float> phi_searched_vector = solution_pair.second;

        // number of solution pairs detected in the analysed event_id
        const Int_t hough_solutions_current_event = pt_searched_vector.size();

        hough_solutions_identified.emplace(current_event_id, Int_t());
        hough_solutions_identified[current_event_id] = 0;

        // the block of code below iterates over all the Hough solutions in a given event_id, and tries to math truth solutions,
        // both discrepancy in pt and phi must be smaller than PT_PRECISION and PHI_PRECISION respectively
        for (Int_t hough_index = 0; hough_index < hough_solutions_current_event; ++hough_index){

            bool_efficiency = 0;
            // pt_searched and phi_searche are the Hough solution pair which we try to fid in the set of truth solutions, for the given event_id
            const float pt_searched  = pt_searched_vector.at(hough_index);
            const float phi_searched = phi_searched_vector.at(hough_index);

            tree_truth -> GetEntry(current_event_id);
            std::vector<float> pt_truth_current_event_id  = *pt_truth_branch;
            std::vector<float> phi_truth_current_event_id = *phi_truth_branch;

            const Int_t truth_solutions_current_event = pt_truth_current_event_id.size();

            for (Int_t truth_index = 0; truth_index < truth_solutions_current_event; ++truth_index){

                const float pt_difference = pt_searched - pt_truth_current_event_id.at(truth_index);
                const float phi_difference = phi_searched - phi_truth_current_event_id.at(truth_index);

                if (fabs(pt_difference) < PT_PRECISION && fabs(phi_difference) < PHI_PRECISION){

                    bool_efficiency = 1;

                    // pt and phi scatterplot
                    hist_pt_scatterplot  -> Fill(pt_truth_current_event_id.at(truth_index), pt_searched);
                    hist_phi_scatterplot -> Fill(phi_truth_current_event_id.at(truth_index), phi_searched);
                }
            }

            bool_efficiency ? ++hough_solutions_identified[current_event_id] : 0 ;

            pt_efficiency  -> Fill(bool_efficiency, pt_searched);
            phi_efficiency -> Fill(bool_efficiency, phi_searched);
        }
    }


// print files summary
    cout << ".. File " << hough_filename << " contains " << hough_events.size() << " events and " << tree_hough -> GetEntries() << " solutions\n" << endl;
    Int_t total_hough_reconstructed = 0;

    for (const auto& hough_event : hough_events){

        Int_t event_id_cout = hough_event.first;
        tree_truth -> GetEntry(event_id_cout);

        total_hough_reconstructed += hough_solutions_identified[event_id_cout];
        float reconstructed_fraction = float(hough_solutions_identified[event_id_cout])/hough_event.second.first.size();

        cout << ".. Event " << event_id_cout << " - truth solutions: " << pt_truth_branch -> size() << ", correctly reconstructed solutions: " << hough_solutions_identified[event_id_cout] << "/" << hough_event.second.first.size() << " = " << float(TMath::Nint(reconstructed_fraction * 10000)) / 100 << "\%" << endl;
    }


// calculated efficiency summary
    cout << "\n.. Allowed discrepancy for pt: " << PT_PRECISION << ", and for phi: " << PHI_PRECISION << endl;
    cout << ".. Global number of solutions found: " << total_hough_reconstructed << "/" << tree_hough -> GetEntries() << " = " << float(TMath::Nint(float(total_hough_reconstructed) / tree_hough -> GetEntries() * TMath::Power(10, 4))) / TMath::Power(10, 2) << "\%\n" << endl;


// draw phi histograms, save canvas to pdf file, close files
    canvas1 -> cd(1);
    hist_pt_scatterplot -> Draw();

    canvas1 -> cd(2);
    pt_efficiency -> Draw("AP");

    canvas1 -> cd(3);
    hist_phi_scatterplot -> Draw();

    canvas1 -> cd(4);
    phi_efficiency -> Draw("AP");

    canvas1 -> SaveAs("solutions_reconstruction_efficiency.png");

    file_hough -> Close();
    file_truth -> Close();
}