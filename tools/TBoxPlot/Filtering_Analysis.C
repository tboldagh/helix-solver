#include <cmath>
#include <iostream>
#include <TCanvas.h>

void Filtering_Analysis(){

    // Used are two detected-circles files so that output when filtering is enable, can be
    // compared to default setting. File hough_events.root contains event_id which passed selection based on r and z regions.
    std::string file_name_ending = "muon_1k";

    std::string truth_file_path =  "../../../DATA/ODD_Single_muon_1k/particles_initial_single_1k_0.root";
    std::string hough_file_path =  "../../../build/detected-circles/detected-circles.root";
    std::string hough_filtered_file_path =  "../../../build/detected-circles/detected-circles.root";
    std::string events_file_path =  "hough_events_1k.root";
    std::string wedge_counts_file_path =  "../../../build/hough_wedge_counts.root";
    std::string output_path = "output/output_histograms_filtering_analysis_" + file_name_ending + ".root";
    std::string pdf_file_name = "output/filtering_analysis_" + file_name_ending + ".pdf";

    std::string truth_tree_name = "particles";
    std::string hough_tree_name = "solutions";
    std::string hough_filtered_tree_name = "solutions";
    std::string events_tree_name = "tree";
    std::string wedge_counts_tree_name = "tree";

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
    delete gROOT -> FindObject("teff_eta_filtering_ok_events");
    delete gROOT -> FindObject("hist_pt_filtering");
    delete gROOT -> FindObject("hist_phi_filtering");
    delete gROOT -> FindObject("hist_nsolutions_filtering");
    delete gROOT -> FindObject("hist_wedge_count");

    TCanvas *canvas1 = new TCanvas("canvas1", "canvas1", 12000, 8000);

    // parameters of the analyzed division region
    const float wedge_eta_center = 0.4;
    const float wedge_phi_center = M_PI;
    const Int_t eta_n_regions = 39;
    const Int_t phi_n_regions = 8;
    const float max_eta =  4.;
    const float min_eta = -4.;
    const float excess_wedge_eta_width = 0.;
    const float excess_wedge_phi_width = 0.;
    const float wedge_eta_width = (max_eta - min_eta)/eta_n_regions + excess_wedge_eta_width;
    const float wedge_phi_width = (2 * M_PI)/phi_n_regions + excess_wedge_phi_width;

    const float min_hist_pt = 0.;
    const float max_hist_pt = 11.;
    const float min_hist_phi = - 1.05 * M_PI;
    const float max_hist_phi =   1.05 * M_PI;
    const Int_t min_solutions_per_event = 0;
    const Int_t max_solutions_per_event = 42;

    const Int_t n_bins_efficiency = 50;
    const Int_t n_bins_hist = max_solutions_per_event - min_solutions_per_event;
    const Int_t n_bins_scatterplot = 200;

    const Int_t phi_n_wedge = 8;
    float phi_min_wedge = - M_PI;
    float phi_max_wedge =   M_PI;
    const float phi_width = (phi_max_wedge - phi_min_wedge)/phi_n_wedge;
    phi_min_wedge -= phi_width/2;
    phi_max_wedge -= phi_width/2;

    const Int_t eta_n_wedge = 39;
    float eta_min_wedge = -4.;
    float eta_max_wedge =  4.;
    const float eta_width = (eta_max_wedge - eta_min_wedge)/eta_n_wedge;
    eta_min_wedge -= eta_width/2;
    eta_max_wedge -= eta_width/2;

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
    TEfficiency *teff_eta_filtering_ok_events = new TEfficiency("teff_eta_filtering_ok_event", "Reconstruction efficiency of #eta (after);#eta truth [rad];efficiency", n_bins_efficiency, min_eta, max_eta);

    TH2D *hist_pt_filtering  = new TH2D("hist_pt_filtering", "Solutions per event by p_{T} (after);p_{T} truth [GeV];count", n_bins_scatterplot, min_hist_pt, max_hist_pt, n_bins_scatterplot, min_solutions_per_event, max_solutions_per_event);
    TH2D *hist_phi_filtering = new TH2D("hist_phi_filtering", "Solutions per event by #phi (after);#phi truth [rad];count", n_bins_scatterplot, min_hist_phi, max_hist_phi, n_bins_scatterplot, min_solutions_per_event, max_solutions_per_event);

    //TH1D *hist_nsolutions_filtering = new TH1D("hist_nsolutions_filtering", "Histogram of number of solutions (after);number of solutions;count", n_bins_hist, min_solutions_per_event, max_solutions_per_event);
    TH1D *hist_nsolutions_filtering = new TH1D("hist_nsolutions_filtering", ";number of solutions;counts", n_bins_hist, min_solutions_per_event, max_solutions_per_event);

    // histogram for number of spacepoints detected in each wedge
    TH2D *wedge_counts_scatter = new TH2D("hist_wedge_count", "Number of spacepoints in wedge;#eta center;#phi center", eta_n_wedge, eta_min_wedge, eta_max_wedge, phi_n_wedge, phi_min_wedge, phi_max_wedge);

    gPad 	-> 	SetLeftMargin(0.15);
    gStyle  ->  SetOptStat(0);

    // open files
    std::unique_ptr<TFile> file_truth(TFile::Open(truth_file_path.c_str()));
    std::unique_ptr<TFile> file_hough(TFile::Open(hough_file_path.c_str()));
    std::unique_ptr<TFile> file_hough_filtered(TFile::Open(hough_filtered_file_path.c_str()));
    std::unique_ptr<TFile> file_events(TFile::Open(events_file_path.c_str()));
    std::unique_ptr<TFile> file_wedge_counts(TFile::Open(wedge_counts_file_path.c_str()));

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

    if (file_wedge_counts == nullptr) {
        throw std::runtime_error("Can't open input file: " + wedge_counts_file_path);
    }

    // access trees in the files
    std::unique_ptr<TTree> tree_truth(file_truth->Get<TTree>(truth_tree_name.c_str()));
    std::unique_ptr<TTree> tree_hough(file_hough->Get<TTree>(hough_tree_name.c_str()));
    std::unique_ptr<TTree> tree_hough_filtered(file_hough_filtered->Get<TTree>(hough_filtered_tree_name.c_str()));
    std::unique_ptr<TTree> tree_events(file_events->Get<TTree>(events_tree_name.c_str()));
    std::unique_ptr<TTree> tree_wedge_counts(file_wedge_counts->Get<TTree>(wedge_counts_tree_name.c_str()));


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
    if (tree_wedge_counts == nullptr) {
        throw std::runtime_error("Can't access tree in the ROOT file: " + wedge_counts_file_path);
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
    float eta_hough_filtered_branch;

    tree_hough_filtered -> SetBranchAddress("event_id", &event_id_hough_filtered_branch);
    tree_hough_filtered -> SetBranchAddress("pt", &pt_hough_filtered_branch);
    tree_hough_filtered -> SetBranchAddress("phi", &phi_hough_filtered_branch);
    tree_hough_filtered -> SetBranchAddress("eta", &eta_hough_filtered_branch);

    const Int_t all_hough_filtered_nentries = tree_hough_filtered -> GetEntries();
    std::map<UInt_t, std::pair<std::vector<float>, std::vector<float>>> hough_filtered_events;

    // set branch address and access solution pairs - events after selection
    Int_t event_id_events_branch;

    tree_events -> SetBranchAddress("event_id", &event_id_events_branch);
    const Int_t all_events_nentries = tree_events -> GetEntries();

    // set branch address and access solution pairs - phi and eta center and number of spacepoints
    float wedge_phi_branch;
    float wedge_eta_branch;
    Int_t wedge_counts_branch;

    tree_wedge_counts -> SetBranchAddress("phi", &wedge_phi_branch);
    tree_wedge_counts -> SetBranchAddress("eta", &wedge_eta_branch);
    tree_wedge_counts -> SetBranchAddress("counts", &wedge_counts_branch);
    const Int_t all_wedge_nentries = tree_wedge_counts -> GetEntries();

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
    for (Int_t index_hough = 0; index_hough < all_hough_filtered_nentries; ++index_hough){

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
            //cout << "... No solution found for event " << event_id << " before filtration." << endl;
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
        float eta_truth = (eta_truth_branch) -> at(0);

        bool bool_tefficiency{};
        Int_t hough_solutions_current_event{};
        if (hough_filtered_events.find(event_id) != hough_filtered_events.end()){

            ++count_found_distinct_solutions_filtering;
            bool_tefficiency = 1;
            hough_solutions_current_event = hough_filtered_events[event_id].first.size();
        } else {
            //cout << "... No solution found for event " << event_id << " after filtration." << endl;
        }

        teff_pt_filtering_ok_events  -> Fill(bool_tefficiency, pt_truth);
        teff_phi_filtering_ok_events -> Fill(bool_tefficiency, phi_truth);
        teff_eta_filtering_ok_events -> Fill(bool_tefficiency, eta_truth);

        hist_nsolutions_filtering -> Fill(hough_solutions_current_event);

        hist_pt_filtering -> Fill(pt_truth, hough_solutions_current_event);
        hist_phi_filtering -> Fill(phi_truth, hough_solutions_current_event);
    }


    // scatterpot of number of spacepoints in each wedge
    for (Int_t index_wedge = 0; index_wedge < all_wedge_nentries; ++index_wedge){

        tree_wedge_counts -> GetEntry(index_wedge);
        wedge_counts_scatter -> Fill(wedge_eta_branch, wedge_phi_branch, wedge_counts_branch);
    }


    ////////////////////////////////////////
    // number of truth and detected-circles solutions in the given wedge
    ////////////////////////////////////////
    // truth events
    Int_t n_truth_tracks{};
    for (const auto event_id : vector_events){

        tree_truth -> GetEntry(event_id);
        float pt_truth = (pt_truth_branch) -> at(0);
        float phi_truth = (phi_truth_branch) -> at(0);
        float eta_truth = (eta_truth_branch) -> at(0);

        if (std::fabs(wedge_eta_center - eta_truth) < 0.95*wedge_eta_width && std::fabs(wedge_phi_center - phi_truth) < 0.95*wedge_phi_width){

            n_truth_tracks++;
        }
    }

    // Hough events
    //for (const auto event_id : vector_events){

        //tree_hough -> GetEntry(event_id);
        //auto hough_pair = hough_filtered_events[tree_hough];

        //if (std::fabs(wedge_eta_center - eta_hough) < 0.95*wedge_eta_width && std::fabs(wedge_phi_center - phi_hough) < 0.95*wedge_phi_width){

        //    n_hough_tracks++;
        //}
    //}
    Int_t n_hough_tracks{};
    for (Int_t index_hough = 0; index_hough < all_hough_nentries; ++index_hough){

        tree_hough_filtered -> GetEntry(index_hough);
        float phi_hough = phi_hough_filtered_branch;
        float eta_hough = eta_hough_filtered_branch;

        if (std::fabs(wedge_eta_center - eta_hough) < 0.95*wedge_eta_width && std::fabs(wedge_phi_center - phi_hough) < 0.95*wedge_phi_width){

            ++n_hough_tracks;
        }
    }

    // general statistics
    cout << "\n... Total number of solutions found without filtering: " << all_hough_nentries << endl;
    cout << "... Total number of solutions found with filtering on: " << all_hough_filtered_nentries << endl;
    cout << "... Reduction in number of solutions: " << 100*float(all_hough_nentries - all_hough_filtered_nentries)/all_hough_nentries << "%\n" << endl;

    cout << "... Average number of solutions found for single particle without filtering: " << float(all_hough_nentries) / count_found_distinct_solutions_no_filtering << endl;
    cout << "... Average number of solutions found for single particle with filtering on: " << float(all_hough_filtered_nentries) / count_found_distinct_solutions_filtering << "\n" << endl;

    cout << "... Solutions for " << count_found_distinct_solutions_no_filtering << "/" << all_events_nentries << " were found witout using filtering." << endl;
    cout << "... Solutions for " << count_found_distinct_solutions_filtering << "/" << all_events_nentries << " were found using filtering on.\n" << endl;

    cout << "... Number of truth particle tracks within wedge eta_center = " << wedge_eta_center << ", phi_center = " << wedge_phi_center << ": " << n_truth_tracks << endl;
    cout << "... Number of Hough particle tracks within wedge eta_center = " << wedge_eta_center << ", phi_center = " << wedge_phi_center << ": " << n_hough_tracks << "\n" << endl;

    // save all the histograms to ROOT file
    teff_pt_no_filtering_ok_events -> Write();
    teff_phi_no_filtering_ok_events -> Write();
    hist_pt_no_filtering -> Write();
    hist_phi_no_filtering -> Write();
    hist_nsolutions_no_filtering -> Write();
    teff_pt_filtering_ok_events -> Write();
    teff_phi_filtering_ok_events -> Write();
    teff_eta_filtering_ok_events -> Write();
    hist_pt_filtering -> Write();
    hist_phi_filtering -> Write();
    hist_nsolutions_filtering -> Write();
    wedge_counts_scatter -> Write();

    file_output -> Close();

    // plot all the histograms

    //hist_nsolutions_no_filtering -> Draw("HIST");
    //teff_pt_np_filtering_ok_events -> Draw();

    //teff_pt_filtering_ok_events -> Draw();
    //hist_nsolutions_filtering -> Draw("HIST");

    // // q/pt
    // not filtered
    //wedge_counts_scatter->GetXaxis()->SetRangeUser(-3.5, 3.5);
    //wedge_counts_scatter -> Draw("COLZ2");

    //canvas1 -> Divide(4, 3);

    /*
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
    canvas1 -> cd(11);
    teff_eta_filtering_ok_events -> Draw();

    //canvas1 -> cd(12);

   //canvas1 -> SetLogz();
   //wedge_counts_scatter -> Draw("COLZ2");


    // save the canvas to pdf file
    canvas1 -> SaveAs(pdf_file_name.c_str());

}