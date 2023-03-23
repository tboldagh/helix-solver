void Histogram_R_Phi(){

    // histogram of R and Phi
    TFile *file  =  new TFile("root_file/hough_tree_R_Phi.root");
    TTree *tree  =  (TTree*)file->Get("tree");

    float radius;
    float phi;

    tree -> SetBranchAddress("radius", &radius);
    tree -> SetBranchAddress("phi", &phi);

    float radius_min = 0.01;
    float radius_max = 0.20;

    float phi_min = -3.1;
    float phi_max = 3.1;

    int n_bins = 100;

    // histogram of radius
    TCanvas *c1 = new TCanvas("c1", "c1", 1100, 900);
    gStyle  ->  SetOptStat(0);
    TH1F *hist_radius = new TH1F("hist_radius", "Radius histogram;r;Counts", n_bins, radius_min, radius_max);
    TH1F *hist_phi    = new TH1F("hist_phi", "Phi angle histogram;#phi;Counts", 40, phi_min, phi_max);

    Int_t nentries = (Int_t)tree->GetEntries();
    for(Int_t i = 0; i<nentries; ++i){
        tree -> GetEntry(i);
        hist_radius -> Fill(radius);
        hist_phi -> Fill(phi);
    }

    hist_radius -> SetFillColor(kBlue-4);
    hist_phi -> SetFillColor(kBlue-4);

    hist_radius ->  Draw("HIST");
    c1          ->  SaveAs("output/histogram_radius.pdf");
    hist_phi    ->  Draw("HIST");
    c1          ->  SaveAs("output/histogram_phi.pdf");
}