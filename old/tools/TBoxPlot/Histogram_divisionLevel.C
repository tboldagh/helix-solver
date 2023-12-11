void Histogram_divisionLevel(
    const Int_t min_divisionLevel = 0,
    const Int_t max_divisionLevel = 12){

// initial, remove objects
    delete gROOT -> FindObject("canvas");
    delete gROOT -> FindObject("hist_pt_hough");

    TCanvas *canvas = new TCanvas("canvas", "canvas", 800, 1000);


// define histogram
    const Int_t n_bins = max_divisionLevel - min_divisionLevel;
    TH1I *hist_divisionLevel = new TH1I("hist_divisionLevel", "Histogram fo Hough division level; division level; counts", n_bins, min_divisionLevel, max_divisionLevel);

    //gStyle->SetOptStat(0);


// access file and tree
    std::string filename_hough = "hough_tree_SolutionPair.root";
    TFile *file_hough = new TFile(filename_hough.c_str());
    assert(file_hough);

    TTree *tree_hough  =  (TTree*)file_hough->Get("tree");

    Int_t divisionLevel;
    tree_hough -> SetBranchAddress("divisionLevel", &divisionLevel);

    const Int_t nentries = tree_hough -> GetEntries();
    for (Int_t index = 0; index < nentries; ++index){

        tree_hough -> GetEntry(index);
        hist_divisionLevel -> Fill(divisionLevel);
    }

    hist_divisionLevel -> Draw();

    canvas -> SaveAs("divisionLevel_histogram_old.png");

}