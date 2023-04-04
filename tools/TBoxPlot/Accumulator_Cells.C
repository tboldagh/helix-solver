void Accumulator_Cells(
    const float Phi_min      =  0.0,
	const float Phi_max      =  0.6,
	const float qOverPt_min  =  -0.285714,
	const float qOverPt_max  =  0.290249){

    const float Phi_size = Phi_max - Phi_min;
    const float Phi_min_hist      =  Phi_min-Phi_size*0.05;
	const float Phi_max_hist      =  Phi_max+Phi_size*0.05;
    const float qOverPt_size = qOverPt_max - qOverPt_min;
	const float qOverPt_min_hist  =  qOverPt_min - qOverPt_size*0.05;
	const float qOverPt_max_hist  =  qOverPt_max + qOverPt_size*0.05;	

	// used for THist

	float min_size    = 100;		// the smallest area of a single cells
	const int n_bins  =  1000;
	float epsilon     =  0.2;		// added to float size_pow2 so that int(size_pow2) is always power of 2

	TH2F *hist   =   new TH2F("hist", ";#phi;q/p_{T}", n_bins, Phi_min_hist, Phi_max_hist, n_bins, qOverPt_min_hist, qOverPt_max_hist);
	TCanvas *c1  =   new TCanvas("c1", "c1", 1100, 900);
	gStyle       ->  SetOptStat(0);
	hist         ->  Draw();

	TFile *file_BoxPosition   =  new TFile("hough_tree_BoxPosition.root");
	assert(file_BoxPosition);
	TFile *file_SolutionPait  =  new TFile("hough_tree_SolutionPair.root");
	assert(file_SolutionPait);

   	TTree *tree_BoxPosition           =  (TTree*)file_BoxPosition->Get("tree");
	TTree *tree_BoxPosition_minSize   =  (TTree*)file_BoxPosition->Get("tree");
   	TTree *tree_SolutionPair          =  (TTree*)file_SolutionPait->Get("tree");

	float Phi_begin;
	float Phi_end;
	float qOverPt_begin;
	float qOverPt_end;

	float Phi_solution;
	float qOverPt_solution;

	tree_BoxPosition  ->  SetBranchAddress("Phi_begin",&Phi_begin);
	tree_BoxPosition  ->  SetBranchAddress("Phi_end",&Phi_end);
	tree_BoxPosition  ->  SetBranchAddress("qOverPt_begin",&qOverPt_begin);
	tree_BoxPosition  ->  SetBranchAddress("qOverPt_end",&qOverPt_end);

	tree_BoxPosition_minSize  ->  SetBranchAddress("Phi_begin",&Phi_begin);
	tree_BoxPosition_minSize  ->  SetBranchAddress("Phi_end",&Phi_end);
	tree_BoxPosition_minSize  ->  SetBranchAddress("qOverPt_begin",&qOverPt_begin);
	tree_BoxPosition_minSize  ->  SetBranchAddress("qOverPt_end",&qOverPt_end);

	tree_SolutionPair  ->  SetBranchAddress("Phi_solution", &Phi_solution);
	tree_SolutionPair  ->  SetBranchAddress("qOverPt_solution", &qOverPt_solution);

	double size_ratio;
	int size_pow2;

	// colors id copier from ColorBrewer2.0
	Int_t tier_0 = TColor::GetColor("#f7fcf5");
	Int_t tier_1 = TColor::GetColor("#e5f5e0");
	Int_t tier_2 = TColor::GetColor("#c7e9c0");
	Int_t tier_3 = TColor::GetColor("#a1d99b");
	Int_t tier_4 = TColor::GetColor("#74c476");
	Int_t tier_5 = TColor::GetColor("#41ab5d");
	Int_t tier_6 = TColor::GetColor("#238b45");
	Int_t tier_7 = TColor::GetColor("#006d2c");

	Int_t tier_8 = TColor::GetColor("#d0d1e6");
	Int_t tier_9 = TColor::GetColor("#a6bddb");
	Int_t tier_10 = TColor::GetColor("#74a9cf");
	Int_t tier_11 = TColor::GetColor("#3690c0");
	Int_t tier_12 = TColor::GetColor("#0570b0");
	Int_t tier_13 = TColor::GetColor("#ffeda0");

	Int_t tier_14 = TColor::GetColor("#fed976");
	Int_t tier_15 = TColor::GetColor("#feb24c");
	Int_t tier_16 = TColor::GetColor("#fd8d3c");
	Int_t tier_17 = TColor::GetColor("#fc4e2a");
	Int_t tier_18 = TColor::GetColor("#e31a1c");
	Int_t tier_19 = TColor::GetColor("#bd0026");

	Int_t tier_solution = TColor::GetColor("#67000d");

	Int_t nentries_BoxPosition = (Int_t)tree_BoxPosition->GetEntries();
	Int_t nentries_SolutionPair = (Int_t)tree_SolutionPair->GetEntries();

	for (Int_t k=0; k<nentries_BoxPosition; k++) {
      	tree_BoxPosition_minSize -> GetEntry(k);
		if ((Phi_end-Phi_begin)*(qOverPt_end-qOverPt_begin) < min_size)
		{
			min_size = (Phi_end-Phi_begin)*(qOverPt_end-qOverPt_begin);
		}
	}

	Int_t i;
	for (i=0; i<nentries_BoxPosition; i++) {
      	tree_BoxPosition->GetEntry(i);
		// No box outside the initial range
		if (Phi_begin < Phi_min || Phi_end > Phi_max || qOverPt_begin < qOverPt_min || qOverPt_end > qOverPt_max) {
			continue;
		}

		TBox *box = new TBox(Phi_begin, qOverPt_begin, Phi_end, qOverPt_end);
		size_ratio = int(((Phi_end-Phi_begin)*(qOverPt_end-qOverPt_begin))/min_size + epsilon);
		size_pow2 = int(std::log2(size_ratio));

		switch(size_pow2){
			case 0:
				box -> SetFillColor(tier_solution);
				break;
			case 1:
				box -> SetFillColor(tier_19);
				break;
			case 2:
				box -> SetFillColor(tier_18);
				break;
			case 3:
				box -> SetFillColor(tier_17);
				break;
			case 4:
				box -> SetFillColor(tier_16);
				break;
			case 5:
				box -> SetFillColor(tier_15);
				break;
			case 6:
				box -> SetFillColor(tier_14);
				break;
			case 7:
				box -> SetFillColor(tier_13);
				break;
			case 8:
				box -> SetFillColor(tier_12);
				break;
			case 9:
				box -> SetFillColor(tier_11);
				break;
			case 10:
				box -> SetFillColor(tier_10);
				break;
			case 11:
				box -> SetFillColor(tier_9);
				break;
			case 12:
				box -> SetFillColor(tier_8);
				break;
			case 13:
				box -> SetFillColor(tier_7);
				break;
			case 14:
				box -> SetFillColor(tier_6);
				break;
			case 15:
				box -> SetFillColor(tier_5);
				break;
			case 16:
				box -> SetFillColor(tier_4);
				break;
			case 17:
				box -> SetFillColor(tier_3);
				break;
			case 18:
				box -> SetFillColor(tier_2);
				break;
			default:
				std::cout<<size_pow2<<std::endl;
		}

		box -> Draw("same");

	}

	for(Int_t j=0; j<=4; j++){
		TLine *line_horizontal = new TLine(Phi_min,qOverPt_min+j*(qOverPt_max-qOverPt_min)/4,Phi_max,qOverPt_min+j*(qOverPt_max-qOverPt_min)/4);
  		line_horizontal  ->  SetLineColor(kRed);
		line_horizontal  ->  SetLineWidth(1);
  		line_horizontal  ->  Draw("same");

		TLine *line_vertical = new TLine(Phi_min+j*(Phi_max-Phi_min)/4,qOverPt_min,Phi_min+j*(Phi_max-Phi_min)/4,qOverPt_max);
  		line_vertical   ->  SetLineColor(kRed);
		line_vertical   ->  SetLineWidth(1);
  		line_vertical   ->  Draw("same");
	}

	for(Int_t j=0; j<nentries_SolutionPair; j++){
		tree_SolutionPair -> GetEntry(j);
		TMarker *point = new TMarker(Phi_solution, qOverPt_solution, 8);
		point -> Draw("same");
		point -> SetMarkerColor(kBlue);
		point -> SetMarkerSize(0.8);
	}

	c1 -> SaveAs("Accumulator_Cells_AllTracks.pdf");
}
