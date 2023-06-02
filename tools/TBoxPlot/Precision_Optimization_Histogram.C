void Precision_Optimization_Histogram(
    const float Phi_precision          =  0.01,
	const float qOverPt_precision_min  =  0.001,
	const float qOverPt_precision_max  =  0.03
){

    // initial lines
    delete gROOT -> FindObject("hist");
    delete gROOT -> FindObject("hist2");
    delete gROOT -> FindObject("canvas");

    const Int_t pileup_count = 1;
    const Int_t threshold = 6;

    // definition of names of file and trees used later
    std::string spacepoints_file_name = "spacepoints/spacepoints_single_1k.root";
    std::string spacepoints_tree_name = "spacepoints";

    std::string file_name_raw = "detected-circles_phi_0.01/detected-circles_single_1k_";
    std::string tree_name = "solutions";

    // histograms definition
    const Int_t n_bins_hist = 50;
    const Int_t n_bins_scatterplot = 200;
    const Int_t min_solutions_per_event = 0;
    const Int_t max_solutions_per_event = 800;
    TH1F *hist         =   new TH1F("hist", "Avg number of solutions by q/p_{T} precision;#phi precision;Counts", n_bins_hist, qOverPt_precision_min, qOverPt_precision_max);
    TH2F *scatterplot  =   new TH2F("hist2", "Number of solutions by q/p_{T} precision for each event;#phi precision;Counts", n_bins_scatterplot, qOverPt_precision_min, qOverPt_precision_max, n_bins_scatterplot, min_solutions_per_event, max_solutions_per_event);
	TCanvas *canvas    =   new TCanvas("canvas", "canvas", 1100, 900);
	gStyle  ->  SetOptStat(0);

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
    // phi and qOverpt could be reconstructed - number of entries > threshold
    std::map<Int_t, Int_t> map_of_spacepoints;
    for (Int_t index_spacepoints = 0; index_spacepoints < nentries_spacepoints; ++index_spacepoints){

        spacepoints_tree -> GetEntry(index_spacepoints);
        map_of_spacepoints.try_emplace(event_id_spacepoints, 0);
        map_of_spacepoints[event_id_spacepoints] += 1;
    }

    // check number of event, in which solutions could have possibly be found - numbe rof entries greater than threshold
    Int_t count_acceptable_events{};
    for (const auto & map_element : map_of_spacepoints){

        if (map_element.second >= threshold) ++count_acceptable_events;
    }

    // acces data in the inpout file
    const Int_t max_qOverPt_index = 50;
    const Int_t phi_index = 1;

    float qOverPt_precision;

    // proper loop over all the files
    for (Int_t qOverPt_index = 1; qOverPt_index <= max_qOverPt_index; ++qOverPt_index){

        // redefinition of name of the files
        std::string file_name = file_name_raw + "1_" + std::to_string(qOverPt_index) + ".root";
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

        qOverPt_precision = qOverPt_precision_min + qOverPt_index * (qOverPt_precision_max - qOverPt_precision_min) / (max_qOverPt_index - 1);

        std::cout << "\n... Filename: " << file_name << std::endl;
        std::cout << "... qOverPt precision equal " << qOverPt_precision << " resulted in " << nentries << " solutions" << std::endl;
        std::cout << "... Number of reconstructed events: " << n_event_id << "/" << count_acceptable_events << std::endl;

        for (auto solution : map_of_solutions){
            scatterplot -> Fill(qOverPt_precision, solution.second);
        }

        hist -> Fill(qOverPt_precision, nentries/(n_event_id * pileup_count));
    }

    scatterplot -> Draw("SCAT");
    canvas -> SaveAs("precision_optimization_scatterplot_0.01.pdf");

    canvas -> Clear();
    hist -> SetMarkerStyle(5);
    hist -> SetMarkerSize(2.0);
    hist -> Draw("HIST P");
    hist -> Draw("HIST SAME");
    canvas -> SaveAs("precision_optimization_histogram_0.01.pdf");

}