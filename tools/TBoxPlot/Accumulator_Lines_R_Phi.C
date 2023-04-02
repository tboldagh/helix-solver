void Accumulator_Lines_R_Phi(
    const float Phi_min      =  0.0990196,
	const float Phi_max      =  0.60098,
	const float qOverPt_min  =  -0.285714,
	const float qOverPt_max  =  0.290249 ){

    const float Phi_size = Phi_max - Phi_min;
    const float Phi_min_hist      =  Phi_min-Phi_size*0.05;
	const float Phi_max_hist      =  Phi_max+Phi_size*0.05;
    const float qOverPt_size = qOverPt_max - qOverPt_min;
	const float qOverPt_min_hist  =  qOverPt_min - qOverPt_size*0.05;
	const float qOverPt_max_hist  =  qOverPt_max + qOverPt_size*0.05;

    TCanvas *c1 = new TCanvas("c1", "c1", 1100, 900);
	c1->SetGrid();
    gStyle  ->  SetOptStat(0);

    TFile *file  =  new TFile("hough_tree_RPhi.root");
    assert(file);
    TTree *tree  =  (TTree*)file->Get("tree");

    float radius;
    float phi;

    tree -> SetBranchAddress("radius", &radius);
    tree -> SetBranchAddress("phi", &phi);

    int n_bins = 100;
    TH2F *hist = new TH2F("hist", ";#phi;q/p_{T}", n_bins, Phi_min_hist, Phi_max_hist, n_bins, qOverPt_min_hist, qOverPt_max_hist);
    hist -> Draw();

    float y_Top;
    float y_Bottom;
    float x_Left;
    float x_Right;

    Int_t nentries = (Int_t)tree->GetEntries();
    for(Int_t i = 0; i<nentries; ++i){
        tree -> GetEntry(i);

        y_Top  = qOverPt_max;
        y_Bottom = qOverPt_min;
        x_Left   =  - radius * y_Top + phi;
        x_Right  =  - radius * y_Bottom + phi;

        if(x_Left > Phi_min && x_Right < Phi_max)
        {
            TLine *line = new TLine(x_Left, y_Top, x_Right, y_Bottom);
  		    line  ->  SetLineColor(kRed);
		    line  ->  SetLineWidth(1);
  		    line  ->  Draw("same");

        } else if(x_Left < Phi_min && x_Right < Phi_max && x_Right > Phi_min)
        {
            y_Top = (Phi_min - phi)/(- radius);
            x_Left = Phi_min;

            TLine *line = new TLine(x_Left, y_Top, x_Right, y_Bottom);
  		    line  ->  SetLineColor(kRed);
		    line  ->  SetLineWidth(1);
  		    line  ->  Draw("same");
        } else if(x_Left > Phi_min && x_Left < Phi_max && x_Right > Phi_max)
        {
            y_Bottom = (Phi_max - phi)/(- radius);
            x_Right = Phi_max;

            TLine *line = new TLine(x_Left, y_Top, x_Right, y_Bottom);
  		    line  ->  SetLineColor(kRed);
		    line  ->  SetLineWidth(1);
  		    line  ->  Draw("same");
        }

    }

    // add border lines
    TLine *line1 = new TLine(Phi_min, qOverPt_min, Phi_min, qOverPt_max);
  	line1  ->  SetLineColor(kBlue);
	line1  ->  SetLineWidth(2);
  	line1  ->  Draw("same");

    TLine *line2 = new TLine(Phi_max, qOverPt_min, Phi_max, qOverPt_max);
  	line2  ->  SetLineColor(kBlue);
	line2  ->  SetLineWidth(2);
  	line2  ->  Draw("same");

    TLine *line3 = new TLine(Phi_min, qOverPt_min, Phi_max, qOverPt_min);
  	line3  ->  SetLineColor(kBlue);
	line3  ->  SetLineWidth(2);
  	line3  ->  Draw("same");

    TLine *line4 = new TLine(Phi_min, qOverPt_max, Phi_max, qOverPt_max);
  	line4  ->  SetLineColor(kBlue);
	line4  ->  SetLineWidth(2);
  	line4 ->  Draw("same");

    // Solutions of the algorithm
    TFile *file_SolutionPair  =  new TFile("hough_tree_SolutionPair.root");
    assert(file_SolutionPair);
    TTree *tree_SolutionPair  =  (TTree*)file_SolutionPair->Get("tree");
    assert(tree_SolutionPair);
    float Phi_solution;
	float qOverPt_solution;
    tree_SolutionPair  ->  SetBranchAddress("Phi_solution", &Phi_solution);
	tree_SolutionPair  ->  SetBranchAddress("qOverPt_solution", &qOverPt_solution);
    Int_t nentries_SolutionPair = (Int_t)tree_SolutionPair->GetEntries();

	for(Int_t j=0; j<nentries_SolutionPair; j++){
		tree_SolutionPair -> GetEntry(j);
		TMarker *point = new TMarker(Phi_solution - M_PI_2, qOverPt_solution, 8);
		point -> Draw("same");
		point -> SetMarkerColor(kGreen);
		point -> SetMarkerSize(0.8);
	}

    c1  ->  SaveAs("Accumulator_Lines_R_Phi.pdf");

}