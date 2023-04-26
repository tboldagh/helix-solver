void Histogram_pt_phi(){

// initial, remove objects
    delete gROOT -> FindObject("canvas");
    delete gROOT -> FindObject("hist_pt_hough");
    delete gROOT -> FindObject("hist_phi_hough");
    delete gROOT -> FindObject("hist_pt_hough_truth");
    delete gROOT -> FindObject("hist_phi_hough_truth");

    TCanvas *canvas = new TCanvas("canvas", "canvas", 800, 1000);
    canvas -> Divide(2, 2);

// define histograms
    const Int_t n_bins = 100;
    const float pt_hough_hist_min = 0;
    const float pt_hough_hist_max = 11;
    const float phi_hough_hist_min = - 1.1 * M_PI;
    const float phi_hough_hist_max =   1.1 * M_PI;

    TH1D *hist_pt_hough  = new TH1D("hist_pt_hough", "p_{T} Hough histogram;p_{T} [GeV];counts", n_bins, pt_hough_hist_min, pt_hough_hist_max);
    TH1D *hist_phi_hough = new TH1D("hist_phi_hough", "#phi Hough histogram;#phi_hough [rad];counts", n_bins, phi_hough_hist_min, phi_hough_hist_max);

    TH1D *hist_pt_truth  = new TH1D("hist_pt_hough_truth", "p_{T} truth histogram;p_{T} [GeV];counts", n_bins, pt_hough_hist_min, pt_hough_hist_max);
    TH1D *hist_phi_truth = new TH1D("hist_phi_hough_truth", "#phi truth histogram;#phi_hough [rad];counts", n_bins, phi_hough_hist_min, phi_hough_hist_max);

    gStyle->SetOptStat(0);


// open file
    std::string filename_hough = "detected-circles.root";
    std::string filename_truth = "fatras_particles_initial.root";

    TFile *file_hough = new TFile(filename_hough.c_str());
    TFile *file_truth = new TFile(filename_truth.c_str());

    assert(file_hough);
    assert(file_truth);


// acces root file, read branches - Hough solutions
    TTree *tree_hough  =  (TTree*)file_hough->Get("solutions");
    float pt_hough;
    float phi_hough;

    tree_hough -> SetBranchAddress("pt", &pt_hough);
    tree_hough -> SetBranchAddress("phi", &phi_hough);


// acces root file, read branches - truth solutions
    TTree *tree_truth  =  (TTree*)file_truth->Get("particles");
    std::vector<float> *pt_truth = 0;
    std::vector<float> *phi_truth = 0;

    tree_truth -> SetBranchAddress("pt", &pt_truth);
    tree_truth -> SetBranchAddress("phi", &phi_truth);


// fill histograms - Hough
    Int_t nentries = tree_hough -> GetEntries();
    for (Int_t index = 0; index < nentries; ++index){

        tree_hough -> GetEntry(index);
        hist_pt_hough  -> Fill(pt_hough);
        hist_phi_hough -> Fill(phi_hough);
    }


// fill histograms - truth
    Int_t nevents_truth = tree_truth -> GetEntries();
    for (Int_t index_event = 0; index_event < nevents_truth; ++index_event){

        tree_truth -> GetEntry(index_event);
        std::vector<float> *pt_event  = pt_truth;
        std::vector<float> *phi_event = phi_truth;

        Int_t nentries_event = pt_event -> size();

        for (Int_t index = 0; index < nentries_event; ++index){

            hist_pt_truth  -> Fill(pt_event -> at(index));
            hist_phi_truth -> Fill(phi_event -> at(index));
        }

    }

// draw histograms
    canvas -> cd(1);
    hist_pt_hough -> Draw("E0");
    hist_pt_hough -> Draw("SAME HIST");

    canvas -> cd(2);
    hist_phi_hough -> Draw("E0");
    hist_phi_hough -> Draw("SAME HIST");

    canvas -> cd(3);
    hist_pt_truth -> Draw("E0");
    hist_pt_truth -> Draw("SAME HIST");

    canvas -> cd(4);
    hist_phi_truth -> Draw("E0");
    hist_phi_truth -> Draw("SAME HIST");

// final, close file
    canvas -> SaveAs("pt_hough_phi_hough_histograms.png");
    file_hough -> Close();
}