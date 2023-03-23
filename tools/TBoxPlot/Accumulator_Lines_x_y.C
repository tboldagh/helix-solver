void Accumulator_Lines_x_y(){

    const float Phi_min      =  M_PI_2 + 0.0990196;
	const float Phi_max      =  M_PI_2 + 0.60098;
	const float qOverPt_min  =  -0.285714;
	const float qOverPt_max  =  0.290249;

    const float Phi_min_hist      =  1.63;
	const float Phi_max_hist      =  2.2;
	const float qOverPt_min_hist  = -0.32;
	const float qOverPt_max_hist  =  0.32;

    TCanvas *c1 = new TCanvas("c1", "c1", 1100, 900);
    gStyle  ->  SetOptStat(0);

    TFile *file  =  new TFile("root_file/hough_tree_LinePosition.root");
    TTree *tree  =  (TTree*)file->Get("tree");

    float xLeft;
    float yLeft;
    float xRight;
    float yRight;

    tree -> SetBranchAddress("xLeft", &xLeft);
    tree -> SetBranchAddress("yLeft", &yLeft);
    tree -> SetBranchAddress("xRight", &xRight);
    tree -> SetBranchAddress("yRight", &yRight);

    int n_bins = 100;

    TH2F *hist = new TH2F("hist", ";#phi;q/p_{T}", n_bins, Phi_min_hist, Phi_max_hist, n_bins, qOverPt_min_hist, qOverPt_max_hist);
    hist -> Draw();

    Int_t nentries = (Int_t)tree->GetEntries();
    for(Int_t i = 0; i<nentries; ++i){
        tree -> GetEntry(i);

        xLeft += M_PI_2;
        xRight += M_PI_2;

        if(xLeft > xRight)
        {
            float temp = xLeft;
            xLeft = xRight;
            xRight = temp;

            temp = yLeft;
            yLeft = yRight;
            yRight = temp;
        }

        if((xLeft < Phi_min && xRight < Phi_min) || (xLeft > Phi_max && xRight > Phi_max))
        {
            // do nothing - throw away
        } else if(xLeft > Phi_min && xLeft < Phi_max && xRight > Phi_min && xRight < Phi_max)
        {   // lines fully within range
            if(yLeft > qOverPt_max){
                float a = (yLeft - yRight)/(xLeft - xRight);
                float b = yLeft - a * xLeft;

                xLeft = (qOverPt_max - b)/a;
                yLeft = qOverPt_max;
            }

            if(yRight > qOverPt_max){
                float a = (yLeft - yRight)/(xLeft - xRight);
                float b = yLeft - a * xLeft;

                xRight = (qOverPt_max - b)/a;
                yRight = qOverPt_max;
            }

            TLine *line = new TLine(xLeft, yLeft, xRight, yRight);
  		    line  ->  SetLineColor(kRed);
		    line  ->  SetLineWidth(1);
  		    line  ->  Draw("same");
        } else if(xLeft < Phi_min && xRight > Phi_min && xRight < Phi_max)
        {
            // line starting to the left, ending within the box
            float a = (yLeft - yRight)/(xLeft - xRight);
            float b = yLeft - a * xLeft;

            yLeft = a * Phi_min + b;
            xLeft = Phi_min;

            TLine *line = new TLine(xLeft, yLeft, xRight, yRight);
  		    line  ->  SetLineColor(kRed);
		    line  ->  SetLineWidth(1);
  		    line  ->  Draw("same");
        } else if(xLeft > Phi_max && xLeft < Phi_max && xRight > Phi_max)
        {
            // line starting within the box, ending to the right
            float a = (yLeft - yRight)/(xLeft - xRight);
            float b = yLeft - a * xLeft;

            yRight = a * Phi_max + b;
            xRight = Phi_max;

            TLine *line = new TLine(xLeft, yLeft, xRight, yRight);
  		    line  ->  SetLineColor(kRed);
		    line  ->  SetLineWidth(1);
  		    line  ->  Draw("same");
        } else if(xLeft < Phi_min && xRight > Phi_max)
        {
            // line starting to the left, ending to the right
            float a = (yLeft - yRight)/(xLeft - xRight);
            float b = yLeft - a * xLeft;

            yLeft = a * Phi_min + b;
            xLeft = Phi_min;

            yRight = a * Phi_max + b;
            xRight = Phi_max;

            TLine *line = new TLine(xLeft, yLeft, xRight, yRight);
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

    // SOlutions of the algorithm
    TFile *file_SolutionPait  =  new TFile("root_file/hough_tree_SolutionPair.root");
    TTree *tree_SolutionPair  =  (TTree*)file_SolutionPait->Get("tree");

    float Phi_solution;
	float qOverPt_solution;

    tree_SolutionPair  ->  SetBranchAddress("Phi_solution", &Phi_solution);
	tree_SolutionPair  ->  SetBranchAddress("qOverPt_solution", &qOverPt_solution);

    Int_t nentries_SolutionPair = (Int_t)tree_SolutionPair->GetEntries();
	for(Int_t j=0; j<nentries_SolutionPair; j++){
		tree_SolutionPair -> GetEntry(j);
		TMarker *point = new TMarker(Phi_solution, qOverPt_solution, 8);
		point -> Draw("same");
		point -> SetMarkerColor(kGreen);
		point -> SetMarkerSize(0.8);
	}

    c1 -> SaveAs("output/Accumulator_Lines_x_y.pdf");
}