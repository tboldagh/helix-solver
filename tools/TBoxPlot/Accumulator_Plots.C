void Accumulator_Plots(
    const float Phi_min      =  -3.2,
	const float Phi_max      =  3.2,
	const float qOverPt_min  =  -1.1,
	const float qOverPt_max  =  1.1){


	// define initial values
	const bool DISPLAY_CELLS_COLORS 		 = 0;
	const bool DISPLAY_SOLUTION_CELL_BORDERS = 1;
	const bool DISPLAY_LINES 				 = 1;
	const bool DISPLAY_SOLUTIONS 			 = 1;
	const bool DISPLAY_COUNTOUR_LINES 		 = 1;

	const std::string filename_BoxPosition = "hough_tree_files/hough_tree_BoxPosition_event_9445.root";
	const std::string filename_RPhi = "hough_tree_files/hough_tree_RPhi_event_9445.root";
	const std::string filename_SolutionPair = "hough_tree_files/hough_tree_SolutionPair_event_9445.root";

	const std::string treename_BoxPosition = "tree";
	const std::string treename_RPhi = "tree";
	const std::string treename_SolutionPair = "tree";

    const float Phi_size = Phi_max - Phi_min;
    const float Phi_min_hist      =  Phi_min-Phi_size*0.05;
	const float Phi_max_hist      =  Phi_max+Phi_size*0.05;

    const float qOverPt_size = qOverPt_max - qOverPt_min;
	const float qOverPt_min_hist  =  qOverPt_min - qOverPt_size*0.05;
	const float qOverPt_max_hist  =  qOverPt_max + qOverPt_size*0.05;

	delete gROOT -> FindObject("hist");
	delete gROOT -> FindObject("c1");


	// histogram declaration
	const int n_bins  =  1000;

	TH2F *hist   =   new TH2F("hist", ";#varphi [rad];q/p_{T} [GeV^{-1}]", n_bins, Phi_min_hist, Phi_max_hist, n_bins, qOverPt_min_hist, qOverPt_max_hist);
	TCanvas *c1  =   new TCanvas("c1", "c1", 12000, 12000);
	gPad 	-> 	SetLeftMargin(0.15);
	gStyle  ->  SetOptStat(0);
	hist    ->  Draw();


	// file hough_tree_BoxPosition.root
	std::unique_ptr<TFile> file_BoxPosition(TFile::Open(filename_BoxPosition.c_str()));
	if ( file_BoxPosition == nullptr ) {
        throw std::runtime_error("Can't open input file: " + filename_BoxPosition);
    }
    std::unique_ptr<TTree> tree_BoxPosition(file_BoxPosition->Get<TTree>(treename_BoxPosition.c_str()));
	Int_t nentries_BoxPosition = (Int_t)tree_BoxPosition->GetEntries();

    if ( tree_BoxPosition == nullptr ) {
        throw std::runtime_error("Can't access tree in the ROOT file: " + treename_BoxPosition);
    }
    std::cout << "... Accessed input tree: " << treename_BoxPosition << " in "  << filename_BoxPosition << ", number of entries: " << nentries_BoxPosition << std::endl;

	float Phi_begin;
	float Phi_end;
	float qOverPt_begin;
	float qOverPt_end;

	float Phi_begin_minsize;
	float Phi_end_minsize;
	float qOverPt_begin_minsize;
	float qOverPt_end_minsize;

	tree_BoxPosition  ->  SetBranchAddress("Phi_begin",&Phi_begin);
	tree_BoxPosition  ->  SetBranchAddress("Phi_end",&Phi_end);
	tree_BoxPosition  ->  SetBranchAddress("qOverPt_begin",&qOverPt_begin);
	tree_BoxPosition  ->  SetBranchAddress("qOverPt_end",&qOverPt_end);


	// file hough_tree_RPhi.root
	std::unique_ptr<TFile> file_RPhi(TFile::Open(filename_RPhi.c_str()));
	if ( file_RPhi == nullptr ) {
        throw std::runtime_error("Can't open input file: " + filename_RPhi);
    }
    std::unique_ptr<TTree> tree_RPhi(file_RPhi->Get<TTree>(treename_RPhi.c_str()));
    Int_t nentries_RPhi = (Int_t)tree_RPhi->GetEntries();

	if ( tree_RPhi == nullptr ) {
        throw std::runtime_error("Can't access tree in the ROOT file: " + treename_RPhi);
    }
    std::cout << "... Accessed input tree: " << treename_RPhi << " in "  << filename_RPhi << ", number of entries: " << nentries_RPhi << std::endl;

	float radius;
	float phi;

	tree_RPhi -> SetBranchAddress("radius", &radius);
	tree_RPhi -> SetBranchAddress("phi", &phi);


	// file hough_tree_SolutionPair.root
	std::unique_ptr<TFile> file_SolutionPair(TFile::Open(filename_SolutionPair.c_str()));
	if ( file_SolutionPair == nullptr ) {
        throw std::runtime_error("Can't open input file: " + filename_SolutionPair);
    }
    std::unique_ptr<TTree> tree_SolutionPair(file_SolutionPair->Get<TTree>(treename_SolutionPair.c_str()));
	Int_t nentries_SolutionPair = (Int_t)tree_SolutionPair->GetEntries();

    if ( tree_SolutionPair == nullptr ) {
        throw std::runtime_error("Can't access tree in the ROOT file: " + treename_SolutionPair);
    }
    std::cout << "... Accessed input tree: " << treename_SolutionPair << " in "  << filename_SolutionPair << ", number of entries: " << nentries_SolutionPair << "\n" << std::endl;

	float Phi_solution;
	float qOverPt_solution;
	float xLeftSolution;
	float xRightSolution;
	float yLeftSolution;
	float yRightSolution;
	int divisionLevel;

	tree_SolutionPair  ->  SetBranchAddress("Phi_solution", &Phi_solution);
	tree_SolutionPair  ->  SetBranchAddress("qOverPt_solution", &qOverPt_solution);
	tree_SolutionPair  ->  SetBranchAddress("xLeftSolution", &xLeftSolution);
	tree_SolutionPair  ->  SetBranchAddress("xRightSolution", &xRightSolution);
	tree_SolutionPair  ->  SetBranchAddress("yLeftSolution", &yLeftSolution);
	tree_SolutionPair  ->  SetBranchAddress("yRightSolution", &yRightSolution);
    tree_SolutionPair  ->  SetBranchAddress("divisionLevel", &divisionLevel);


	// calculate the minimal size of the solution cell
	if (DISPLAY_CELLS_COLORS){
		float min_cell_size  = 100;		// the smallest area of a single cells, to be changed later

		for (Int_t index_minsize = 0; index_minsize < nentries_BoxPosition; ++index_minsize) {
			tree_BoxPosition -> GetEntry(index_minsize);
			if ((Phi_end - Phi_begin) * (qOverPt_end - qOverPt_begin) < min_cell_size)
			{
				min_cell_size = (Phi_end - Phi_begin) * (qOverPt_end - qOverPt_begin);
			}
		}
		const int significant_digits = 10;
		std::cout << "Size of the smallest (solution) cell: " << TMath::Nint(min_cell_size * TMath::Power(10, significant_digits)) / TMath::Power(10, significant_digits) << std::endl;


		// declare color used for cells, colors id copier from ColorBrewer2.0
		std::vector<std::string> shades;
		Int_t numShades = 28;
		int stepSize = 255 / (numShades - 1);

		for (int i = 0; i < numShades; ++i) {
			int shadeValue = i * stepSize;
			std::stringstream ss;
			ss << std::setw(2) << std::setfill('0') << std::hex << shadeValue;
			std::string hexValue = ss.str();
			std::string colorCode = "#00" + hexValue + "00";
			shades.push_back(colorCode);
		}
		shades.at(0) = "#e31a1c";	// distinct color for the solution cell


		// drawig colorful cells
		double size_ratio;
		int size_pow2;
		float epsilon =  0.25;			// added to float size_pow2 so that int(size_pow2) is always power of 2

		for (Int_t index_cell = 0; index_cell < nentries_BoxPosition; ++index_cell) {
			tree_BoxPosition->GetEntry(index_cell);

			size_ratio = int(((Phi_end-Phi_begin)*(qOverPt_end-qOverPt_begin))/min_cell_size + epsilon);
			size_pow2 = int(std::log2(size_ratio));

			if (Phi_begin < Phi_min && Phi_end < Phi_min) continue;
			if (Phi_begin > Phi_max && Phi_end > Phi_max) continue;
			if (qOverPt_begin < qOverPt_min && qOverPt_end < qOverPt_min) continue;
			if (qOverPt_begin > qOverPt_max && qOverPt_end > qOverPt_max) continue;
			if (Phi_begin < Phi_min) Phi_begin = Phi_min;
			if (Phi_end > Phi_max) Phi_end = Phi_max;
			if (qOverPt_begin < qOverPt_min) qOverPt_begin = qOverPt_min;
			if (qOverPt_end > qOverPt_max) qOverPt_end = qOverPt_max;

			TBox *box = new TBox(Phi_begin, qOverPt_begin, Phi_end, qOverPt_end);
			box -> SetFillColor(TColor::GetColor((shades.at(size_pow2)).c_str()));
			box -> Draw("same");

			// Enables seeting each step plotted on the histogram
			//std::cout << "Press Enter to continue...";
    		//std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			//c1 -> Update();
		}
	}


	// Draw lines around the main accumulator area
	const Int_t initial_division_levels = 1;
	if (DISPLAY_COUNTOUR_LINES){
		TLine *line_bottom = new TLine(Phi_min, qOverPt_min, Phi_max, qOverPt_min);
		TLine *line_top    = new TLine(Phi_min, qOverPt_max, Phi_max, qOverPt_max);
		TLine *line_left   = new TLine(Phi_min, qOverPt_min, Phi_min, qOverPt_max);
		TLine *line_right  = new TLine(Phi_max, qOverPt_min, Phi_max, qOverPt_max);

		line_bottom  ->  SetLineColor(kRed);
		line_bottom  ->  SetLineWidth(3);
		line_bottom  ->  Draw("same");

		line_top  ->  SetLineColor(kRed);
		line_top  ->  SetLineWidth(3);
		line_top  ->  Draw("same");

		line_left  ->  SetLineColor(kRed);
		line_left  ->  SetLineWidth(3);
		line_left  ->  Draw("same");

		line_right  ->  SetLineColor(kRed);
		line_right  ->  SetLineWidth(3);
		line_right  ->  Draw("same");
	}


	// display lines based on r and phi parameters
	if (DISPLAY_LINES){

		const float A_const = 3e-4;

		float y_Left;
    	float y_Right;
    	float x_Left;
    	float x_Right;

		for(Int_t index_solutions = 0; index_solutions < nentries_RPhi; ++index_solutions){
			tree_RPhi -> GetEntry(index_solutions);

			y_Left   = qOverPt_min;
			y_Right  = qOverPt_max;

			x_Left   =  A_const * radius * y_Left + phi;
			x_Right  =  A_const * radius * y_Right + phi;

			if(x_Left > Phi_min && x_Right < Phi_max)
			{
				TLine *line = new TLine(x_Left, y_Left, x_Right, y_Right);
				line  ->  SetLineColor(kBlack);
				line  ->  SetLineWidth(1);
				line  ->  Draw("same");

			} else if(x_Left < Phi_min && x_Right < Phi_max && x_Right > Phi_min)
			{
				x_Left = Phi_min;
				y_Left = (x_Left - phi)/(A_const * radius);

				TLine *line = new TLine(x_Left, y_Left, x_Right, y_Right);
				line  ->  SetLineColor(kBlack);
				line  ->  SetLineWidth(1);
				line  ->  Draw("same");
			} else if(x_Left > Phi_min && x_Left < Phi_max && x_Right > Phi_max)
			{
				x_Right = Phi_max;
				y_Right = (x_Right - phi)/(A_const * radius);

				TLine *line = new TLine(x_Left, y_Left, x_Right, y_Right);
				line  ->  SetLineColor(kBlack);
				line  ->  SetLineWidth(1);
				line  ->  Draw("same");
			} else {

				x_Left = Phi_min;
				y_Left = (x_Left - phi)/(A_const * radius);

				x_Right = Phi_max;
				y_Right = (x_Right - phi)/(A_const * radius);

				TLine *line = new TLine(x_Left, y_Left, x_Right, y_Right);
				line  ->  SetLineColor(kBlack);
				line  ->  SetLineWidth(1);
				line  ->  Draw("same");
			}
		}
	}


	// draw solutions cell for each r, phi pair
	int count_solutions_in_ares = 0;
    int divLevel_min =  100;
    int divLevel_max = -100;
    int divLevel_avg =  0;
	if (DISPLAY_SOLUTION_CELL_BORDERS){

		for(Int_t index_solution_cell = 0; index_solution_cell < nentries_SolutionPair; ++index_solution_cell){

			tree_SolutionPair -> GetEntry(index_solution_cell);
			if (Phi_solution > Phi_min && Phi_solution < Phi_max && qOverPt_solution > qOverPt_min && qOverPt_solution < qOverPt_max){
				//TMarker *point = new TMarker(Phi_solution, qOverPt_solution, 8);

				++count_solutions_in_ares;
				//point -> Draw("same");
				//point -> SetMarkerColor(kGreen);
				//point -> SetMarkerSize(0.8);

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
	}


	// Draw solutions values
	if (DISPLAY_SOLUTIONS){
		for(Int_t j=0; j<nentries_SolutionPair; j++){
			tree_SolutionPair -> GetEntry(j);

			if (Phi_solution > Phi_min && Phi_solution < Phi_max && qOverPt_solution > qOverPt_min && qOverPt_solution < qOverPt_max){
				TMarker *point = new TMarker(Phi_solution, qOverPt_solution, 8);
				point -> Draw("same");
				point -> SetMarkerColor(kGreen);
				point -> SetMarkerSize(4);
			}
		}
	}

    cout << ".. In the selected range of phi and q/pt " << count_solutions_in_ares << " solutions are present." << endl;
    cout << ".. Min divisio level: " << divLevel_min << ", max: " << divLevel_max << ", average division level: " << float(divLevel_avg) / count_solutions_in_ares << ".\n" << endl;

	//hist  -> GetYaxis() -> SetTitleOffset(1.);
	c1 -> SaveAs("output/Accumulator_Plot.pdf");

}