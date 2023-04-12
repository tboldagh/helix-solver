void Accumulator_Lines_R_Phi(
    const float Phi_min       =  0.1,
	const float Phi_max       =  0.6,
	const float qOverPt_min   =  -1./3.5,
	const float qOverPt_max   =  +1./3.5){

    const float margin = 0.05;
    const float Phi_size      = Phi_max - Phi_min;
    const float qOverPt_size  = qOverPt_max - qOverPt_min;

    const float Phi_min_hist      =  Phi_min-Phi_size * margin;
	const float Phi_max_hist      =  Phi_max+Phi_size * margin;
	const float qOverPt_min_hist  =  qOverPt_min - qOverPt_size * margin;
	const float qOverPt_max_hist  =  qOverPt_max + qOverPt_size * margin;
    const float A_const = 3e-4;

    TCanvas *c1 = new TCanvas("c1", "c1", 1100, 900);
	c1->SetGrid();
    gStyle  ->  SetOptStat(0);

    TFile *file  =  new TFile("hough_tree_RPhi.root");
    assert(file);
    TTree *tree  =  (TTree*)file->Get("tree");
    cout << "\n.. Will draw " << tree->GetEntries() << " lines\n";

    float radius;
    float phi;

    tree -> SetBranchAddress("radius", &radius);
    tree -> SetBranchAddress("phi", &phi);

    int n_bins = 100;
    TH2F *hist = new TH2F("hist", ";#phi;q/p_{T}", n_bins, Phi_min_hist, Phi_max_hist, n_bins, qOverPt_min_hist, qOverPt_max_hist);
    hist -> Draw();

    float y_Left;
    float y_Right;
    float x_Left;
    float x_Right;

    Int_t nentries = (Int_t)tree->GetEntries();
    for(Int_t i = 0; i<nentries; ++i){
        tree -> GetEntry(i);

        y_Left   = qOverPt_min;
        y_Right  = qOverPt_max;

        x_Left   =  A_const * radius * y_Left + phi;
        x_Right  =  A_const * radius * y_Right + phi;

        if(x_Left > Phi_min && x_Right < Phi_max)
        {
            TLine *line = new TLine(x_Left, y_Left, x_Right, y_Right);
  		    line  ->  SetLineColor(kRed);
		    line  ->  SetLineWidth(1);
  		    line  ->  Draw("same");

        } else if(x_Left < Phi_min && x_Right < Phi_max && x_Right > Phi_min)
        {
            x_Left = Phi_min;
            y_Left = (x_Left - phi)/(A_const * radius);

            TLine *line = new TLine(x_Left, y_Left, x_Right, y_Right);
  		    line  ->  SetLineColor(kRed);
		    line  ->  SetLineWidth(1);
  		    line  ->  Draw("same");
        } else if(x_Left > Phi_min && x_Left < Phi_max && x_Right > Phi_max)
        {
            x_Right = Phi_max;
            y_Right = (x_Right - phi)/(A_const * radius);

            TLine *line = new TLine(x_Left, y_Left, x_Right, y_Right);
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
    cout << ".. File contains " << tree_SolutionPair->GetEntries() << " solutions\n";

    float Phi_solution;
	float qOverPt_solution;
    int divisionLevel;

    float xLeftSolution;
    float yLeftSolution;
    float xRightSolution;
    float yRightSolution;

    tree_SolutionPair  ->  SetBranchAddress("Phi_solution", &Phi_solution);
	tree_SolutionPair  ->  SetBranchAddress("qOverPt_solution", &qOverPt_solution);
    tree_SolutionPair  ->  SetBranchAddress("divisionLevel", &divisionLevel);

    tree_SolutionPair  ->  SetBranchAddress("xLeftSolution",  &xLeftSolution);
    tree_SolutionPair  ->  SetBranchAddress("yLeftSolution",  &yLeftSolution);
    tree_SolutionPair  ->  SetBranchAddress("xRightSolution", &xRightSolution);
    tree_SolutionPair  ->  SetBranchAddress("yRightSolution", &yRightSolution);

    int count_solutions_in_ares = 0;
    int divLevel_min =  100;
    int divLevel_max = -100;
    int divLevel_avg =  0;

    Int_t nentries_SolutionPair = (Int_t)tree_SolutionPair->GetEntries();
	for(Int_t j=0; j<nentries_SolutionPair; j++){
		tree_SolutionPair -> GetEntry(j);
        if (Phi_solution > Phi_min && Phi_solution < Phi_max && qOverPt_solution > qOverPt_min && qOverPt_solution < qOverPt_max){
            TMarker *point = new TMarker(Phi_solution, qOverPt_solution, 8);

            ++count_solutions_in_ares;
		    point -> Draw("same");
		    point -> SetMarkerColor(kGreen);
		    point -> SetMarkerSize(0.8);

            // limits of the solution cell
            TLine *lineSolution1 = new TLine(xLeftSolution, yLeftSolution, xRightSolution, yLeftSolution);
  	        lineSolution1  ->  SetLineColor(kBlue);
	        lineSolution1  ->  SetLineWidth(2);
  	        lineSolution1  ->  Draw("same");

            TLine *lineSolution2 = new TLine(xRightSolution, yLeftSolution, xRightSolution, yRightSolution);
  	        lineSolution2  ->  SetLineColor(kBlue);
	        lineSolution2  ->  SetLineWidth(2);
  	        lineSolution2  ->  Draw("same");

            TLine *lineSolution3 = new TLine(xLeftSolution, yRightSolution, xRightSolution, yRightSolution);
  	        lineSolution3  ->  SetLineColor(kBlue);
	        lineSolution3  ->  SetLineWidth(2);
  	        lineSolution3  ->  Draw("same");

            TLine *lineSolution4 = new TLine(xLeftSolution, yLeftSolution, xLeftSolution, yRightSolution);
  	        lineSolution4  ->  SetLineColor(kBlue);
	        lineSolution4  ->  SetLineWidth(2);
  	        lineSolution4 ->  Draw("same");

            divLevel_avg += divisionLevel;
            if(divisionLevel < divLevel_min){
                divLevel_min = divisionLevel;
            } else if (divisionLevel > divLevel_max){
                divLevel_max = divisionLevel;
            }

        }
	}
    cout << ".. In the selected range of phi and q/pt " << count_solutions_in_ares << " solutions are present." << endl;
    cout << ".. Min divisio level: " << divLevel_min << ", max: " << divLevel_max << ", average division level: " << float(divLevel_avg) / count_solutions_in_ares << ".\n" << endl;

    c1  ->  SaveAs("Accumulator_Lines_R_Phi.pdf");

}