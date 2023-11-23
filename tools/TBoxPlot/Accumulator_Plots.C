#include <cmath>
#include <iostream>
#include <TCanvas.h>

// Use spacepoints filtering just like in helix-solver
#include "../../include/HelixSolver/Debug.h"
#include "../../include/HelixSolver/ZPhiPartitioning.h"

void Accumulator_Plots(
    const float Phi_min      = -3.14,
	const float Phi_max      =  3.14,
	const float qOverPt_min  = -1.0,
	const float qOverPt_max  =  1.0,

	const uint8_t phi_wedge_index = 5,
	const uint8_t eta_wedge_index = 8
	){

	// define settings for generated plot
	// general
	const bool DISPLAY_COUNTOUR_LINES 		 = 1;
	const bool DISPAY_PHI_REGION_SAFE_ZONE   = 0;

	// lines
	const bool DISPLAY_LINES 				    = 1;
	const bool DISPLAY_ONLY_LINES_IN_REGION     = 1;
	const bool DISPLAY_NEIGHBORING_REGION_LINES = 0;

	// truth solutions
	const bool DISPLAY_TRUTH_SOLUTIONS		     = 1;
	const bool DISPLAY_ONLY_TRUTH_IN_REGION      = 0;
	const bool DISPLAY_NEIGHBOURING_REGION_TRUTH = 0;

	// Hough solutions
	const bool DISPLAY_SOLUTIONS 			         = 1;
	const bool DISPLAY_ONLY_SOLUTIONS_IN_REGION      = 1;
	const bool DISPLAY_NEIGHBOURING_REGION_SOLUTIONS = 0;

	const bool DISPLAY_SOLUTION_CELL_BORDERS = 1;

	// draw colorful boxes - do not use for pileup
	const bool DISPLAY_CELLS_COLORS = 0;

	// Names and location of used files
	const std::string filename_BoxPosition = "hough_tree_files/hough_tree_BoxPosition.root";
	const std::string filename_RPhi = "hough_tree_files/hough_tree_RPhi.root";
	const std::string filename_SolutionPair = "hough_tree_files/hough_tree_SolutionPair.root";
	const std::string filename_particles_initial = "../../../data/ODD_ttbar_pu200_200ev/particles_initial.root";

	const std::string treename_BoxPosition = "tree";
	const std::string treename_RPhi = "tree";
	const std::string treename_SolutionPair = "tree";
	const std::string treename_particles_initial = "particles";

	uint8_t particles_initial_event_id = 0;

    const float Phi_size = Phi_max - Phi_min;
    const float Phi_min_hist      =  Phi_min-Phi_size*0.05;
	const float Phi_max_hist      =  Phi_max+Phi_size*0.05;

    const float qOverPt_size = qOverPt_max - qOverPt_min;
	const float qOverPt_min_hist  =  qOverPt_min - qOverPt_size*0.05;
	const float qOverPt_max_hist  =  qOverPt_max + qOverPt_size*0.05;

	delete gROOT -> FindObject("hist");
	delete gROOT -> FindObject("c1");

	// changes to analyse pileup solutions
	const uint8_t n_regions_phi = 8;
	const uint8_t n_regions_eta = 15;

	const float min_phi = -M_PI;
	const float max_phi =  M_PI;
	const float phi_range = max_phi - min_phi;
	const float phi_excess_width = 0.12;
	const float phi_wedge_width_index = phi_range / n_regions_phi;
	const float phi_wedge_width = 0.5 * phi_wedge_width_index + phi_excess_width;

	const float min_eta = -1.5;
	const float max_eta =  1.5;
	const float eta_range = max_eta - min_eta;
	const float eta_excess_width = 0.12;
	const float eta_wedge_width_index = eta_range / n_regions_eta;
	const float eta_wedge_width = 0.5 * eta_wedge_width_index + eta_excess_width;

	// max delta
	const float max_delta_phi = 0.05;
    const float max_delta_eta = 1.2 * eta_wedge_width;
    const float max_delta_q = 0.5;
    const float max_delta_1_over_pt = 0.1;

	// Make sure that seletd index of regions is within the range
	bool use_neighboring_objects{};

	if (DISPLAY_NEIGHBORING_REGION_LINES || DISPLAY_NEIGHBOURING_REGION_TRUTH || DISPLAY_NEIGHBOURING_REGION_SOLUTIONS) use_neighboring_objects = 1;

	float min_phi_range = use_neighboring_objects ? 1 : 0;
	float min_eta_range = use_neighboring_objects ? 1 : 0;

	float max_phi_range = use_neighboring_objects ? n_regions_phi - 2 : n_regions_phi - 1;
	float max_eta_range = use_neighboring_objects ? n_regions_eta - 2 : n_regions_eta - 1;

	if (DISPLAY_ONLY_LINES_IN_REGION){
		if (phi_wedge_index < min_phi_range || phi_wedge_index > max_phi_range){

			std::cerr << ".. Phi index out of range!!" << std::endl;
			return;
		}

		if (eta_wedge_index < min_eta_range || eta_wedge_index > max_eta_range){

			std::cerr << ".. Phi index out of range!!" << std::endl;
			return;
		}
	}

	const float wedge_phi_center = min_phi + phi_wedge_width_index * phi_wedge_index + phi_wedge_width_index / 2.0;
    const float wedge_eta_center = min_eta + eta_wedge_width_index * eta_wedge_index + eta_wedge_width_index / 2.0;

	// neighbour phi regions
	const float wedge_phi_below_center = min_phi + phi_wedge_width_index * (phi_wedge_index-1) + phi_wedge_width_index / 2.0;
	const float wedge_phi_above_center = min_phi + phi_wedge_width_index * (phi_wedge_index+1) + phi_wedge_width_index / 2.0;

	// neighbour eta regions
	const float wedge_eta_below_center = min_eta + eta_wedge_width_index * (eta_wedge_index-1) + eta_wedge_width_index / 2.0;
    const float wedge_eta_above_center = min_eta + eta_wedge_width_index * (eta_wedge_index+1) + eta_wedge_width_index / 2.0;

	const float wedge_z_center = 0;
	const float	wedge_z_width = 200;

	// central region
	Reg phi_reg = Reg(wedge_phi_center, phi_wedge_width);
    Reg z_reg   = Reg(wedge_z_center, wedge_z_width);
    Reg eta_reg = Reg(wedge_eta_center, eta_wedge_width);

	// phi--
	Reg phi_below_reg = Reg(wedge_phi_below_center, phi_wedge_width);

	// phi++
	Reg phi_above_reg = Reg(wedge_phi_above_center, phi_wedge_width);
	
	// eta--
    Reg eta_below_reg = Reg(wedge_eta_below_center, eta_wedge_width);

	// eta++
    Reg eta_above_reg = Reg(wedge_eta_above_center, eta_wedge_width);

	// range of phi and eta for the analyzed region
	std::cout << "\n... Region phi_min = " << phi_reg.center - phi_reg.width << ", phi_max = " << phi_reg.center + phi_reg.width << std::endl;
	std::cout << "... Region eta_min = " << eta_reg.center - eta_reg.width << ", eta_max = " << eta_reg.center + eta_reg.width << "\n" << std::endl;

	// all regions of interest
	// central region + four adjacent
	Wedge wedge = Wedge(phi_reg, z_reg, eta_reg);
	Wedge wedge_left = Wedge(phi_below_reg, z_reg, eta_reg);
	Wedge wedge_right = Wedge(phi_above_reg, z_reg, eta_reg);
	Wedge wedge_bottom = Wedge(phi_reg, z_reg, eta_below_reg);
	Wedge wedge_top = Wedge(phi_reg, z_reg, eta_above_reg);

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
	float z;

	tree_RPhi -> SetBranchAddress("radius", &radius);
	tree_RPhi -> SetBranchAddress("phi", &phi);
	tree_RPhi -> SetBranchAddress("z", &z);


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
	float Phi_wedge;
	float Eta_wedge;
	float xLeftSolution;
	float xRightSolution;
	float yLeftSolution;
	float yRightSolution;
	int divisionLevel;

	tree_SolutionPair  ->  SetBranchAddress("Phi_solution", &Phi_solution);
	tree_SolutionPair  ->  SetBranchAddress("qOverPt_solution", &qOverPt_solution);
	tree_SolutionPair  ->  SetBranchAddress("Phi_wedge", &Phi_wedge);
	tree_SolutionPair  ->  SetBranchAddress("Eta_wedge", &Eta_wedge);
	tree_SolutionPair  ->  SetBranchAddress("xLeftSolution", &xLeftSolution);
	tree_SolutionPair  ->  SetBranchAddress("xRightSolution", &xRightSolution);
	tree_SolutionPair  ->  SetBranchAddress("yLeftSolution", &yLeftSolution);
	tree_SolutionPair  ->  SetBranchAddress("yRightSolution", &yRightSolution);
    tree_SolutionPair  ->  SetBranchAddress("divisionLevel", &divisionLevel);


	// file particles_initial.root
	std::unique_ptr<TFile> file_particles_initial(TFile::Open(filename_particles_initial.c_str()));
	if ( file_particles_initial == nullptr ) {
        throw std::runtime_error("Can't open input file: " + filename_particles_initial);
    }
    std::unique_ptr<TTree> tree_particles_initial(file_particles_initial->Get<TTree>(treename_particles_initial.c_str()));
    Int_t nentries_particles_initial = (Int_t)tree_particles_initial->GetEntries();

	if ( tree_particles_initial == nullptr ) {
        throw std::runtime_error("Can't access tree in the ROOT file: " + treename_particles_initial);
    }
    std::cout << "... Accessed input tree: " << treename_particles_initial << " in "  << filename_particles_initial << ", number of entries: " << nentries_RPhi << std::endl;

	UInt_t event_id_particles_initial;
    std::vector<float> *pt_particles_initial = 0;
    std::vector<float> *phi_particles_initial = 0;
    std::vector<float> *eta_particles_initial = 0;
	std::vector<float> *q_particles_initial = 0;

    tree_particles_initial -> SetBranchAddress("event_id", &event_id_particles_initial);
    tree_particles_initial -> SetBranchAddress("pt", &pt_particles_initial);
    tree_particles_initial -> SetBranchAddress("phi", &phi_particles_initial);
    tree_particles_initial -> SetBranchAddress("eta", &eta_particles_initial);
    tree_particles_initial -> SetBranchAddress("q", &q_particles_initial);


	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////// Mian part of the script - draw chosen objects /////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

			TLine* line = new TLine();

			if(x_Left > Phi_min && x_Right < Phi_max)
			{
				line = new TLine(x_Left, y_Left, x_Right, y_Right);

			} else if(x_Left < Phi_min && x_Right < Phi_max && x_Right > Phi_min)
			{
				x_Left = Phi_min;
				y_Left = (x_Left - phi)/(A_const * radius);

				line = new TLine(x_Left, y_Left, x_Right, y_Right);
			} else if(x_Left > Phi_min && x_Left < Phi_max && x_Right > Phi_max)
			{
				x_Right = Phi_max;
				y_Right = (x_Right - phi)/(A_const * radius);

				line = new TLine(x_Left, y_Left, x_Right, y_Right);
			} else if(x_Left < Phi_min && x_Right > Phi_max){

				x_Left = Phi_min;
				y_Left = (x_Left - phi)/(A_const * radius);

				x_Right = Phi_max;
				y_Right = (x_Right - phi)/(A_const * radius);

				line = new TLine(x_Left, y_Left, x_Right, y_Right);
			} else {

				// do nothing
			}

			// chose which lines migh be displayed if only some are selected
			bool should_include_line = 0;
			if (DISPLAY_NEIGHBORING_REGION_LINES){

				if (wedge.in_wedge_r_phi_z(radius, phi, z) || 
				wedge_left.in_wedge_r_phi_z(radius, phi, z) || 
				wedge_right.in_wedge_r_phi_z(radius, phi, z) ||
				wedge_bottom.in_wedge_r_phi_z(radius, phi, z) ||
				wedge_top.in_wedge_r_phi_z(radius, phi, z)) should_include_line = 1;
			} else {

				if (wedge.in_wedge_r_phi_z(radius, phi, z)) should_include_line = 1;
			}
			
			// display all the lines
			if (DISPLAY_ONLY_LINES_IN_REGION == 0){

				line  ->  SetLineColor(kBlack);
				line  ->  SetLineWidth(1);
				line  ->  SetLineStyle(1);
				line  ->  Draw("same");
			} else { // display only lines within region (+ possibly from adjacent region)

				if (DISPLAY_NEIGHBORING_REGION_LINES){
					// display lines from the region as black, and lines from 
					// four adjacent regions as red

					if (should_include_line){
						if (wedge.in_wedge_r_phi_z(radius, phi, z)){

							line  ->  SetLineColor(kBlack);
							line  ->  SetLineWidth(2);
							line  ->  SetLineStyle(1);
							line  ->  Draw("same");
						} else{

							line  ->  SetLineColor(kRed);
							line  ->  SetLineWidth(1);
							line  ->  SetLineStyle(1);
							line  ->  Draw("same");
						}
					}

				} else {

					if (wedge.in_wedge_r_phi_z(radius, phi, z)){

						line  ->  SetLineColor(kBlack);
						line  ->  SetLineWidth(1);
						line  ->  SetLineStyle(1);
						line  ->  Draw("same");
					}
				}
			}
		}
	}

	
	//////////////////////////
	// display Hough solutions
	//////////////////////////]
	if (DISPLAY_SOLUTIONS){

		for(Int_t index_solution_cell = 0; index_solution_cell < nentries_SolutionPair; ++index_solution_cell){

			tree_SolutionPair -> GetEntry(index_solution_cell);

			if (DISPLAY_ONLY_SOLUTIONS_IN_REGION){	

				if (DISPLAY_NEIGHBOURING_REGION_SOLUTIONS){

					if ((Eta_wedge > (eta_below_reg.center - eta_reg.width) && Eta_wedge < (eta_above_reg.center + eta_reg.width)) 	&& (Phi_wedge > (phi_reg.center - phi_reg.width) && Phi_wedge < (phi_reg.center + phi_reg.width)) ||
						(Eta_wedge > (eta_reg.center - eta_reg.width) && Eta_wedge < (eta_reg.center + eta_reg.width)) 	&& (Phi_wedge > (phi_below_reg.center - phi_reg.width) && Phi_wedge < (phi_above_reg.center + phi_reg.width))){

						if (Phi_solution > Phi_min && Phi_solution < Phi_max && qOverPt_solution > qOverPt_min && qOverPt_solution < qOverPt_max){
		
							TMarker *point = new TMarker(Phi_solution, qOverPt_solution, 8);
							point -> Draw("same");
							point -> SetMarkerColor(kGreen-7);
							point -> SetMarkerSize(4);

							//std::cout << "... hough_phi = " << Phi_solution << " (" << std::fabs(Phi_solution - phi_comparison) << " <? " << "" << ")" << std::endl;
							//std::cout << "... hough_eta = " << Eta_wedge <<  " (" << std::fabs(Eta_wedge - eta_comparison) << ")" << std::endl;
							//std::cout << "... hough_q_over_pt = " << qOverPt_solution << " (" << std::fabs(qOverPt_solution - q_over_pt_comparison) << ")" << std::endl;
							//std::cout << std::endl;
						}
					}
				}

				if ((Eta_wedge > (eta_reg.center - eta_reg.width) && Eta_wedge < (eta_reg.center + eta_reg.width)) && (Phi_solution > (phi_reg.center - phi_reg.width) && Phi_solution < (phi_reg.center + phi_reg.width))){

					if (Phi_solution > Phi_min && Phi_solution < Phi_max && qOverPt_solution > qOverPt_min && qOverPt_solution < qOverPt_max){
	
						TMarker *point = new TMarker(Phi_solution, qOverPt_solution, 8);
						point -> Draw("same");
						point -> SetMarkerColor(kGreen+3);
						point -> SetMarkerSize(2);

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
					}
				}
				
			} else {

				if (Phi_solution > Phi_min && Phi_solution < Phi_max && qOverPt_solution > qOverPt_min && qOverPt_solution < qOverPt_max){

					TMarker *point = new TMarker(Phi_solution, qOverPt_solution, 8);
					point -> Draw("same");
					point -> SetMarkerColor(kGreen);
					point -> SetMarkerSize(2);
				}
			} 
		}
	} 



	////////////////////////////////////
	// display truth particle parameters
	////////////////////////////////////
	float phi_comparison{};
	float eta_comparison{};
	float q_over_pt_comparison{};
	if (DISPLAY_TRUTH_SOLUTIONS){

		tree_particles_initial -> GetEntry(particles_initial_event_id);

		for (uint64_t truth_index = 0; truth_index < pt_particles_initial -> size(); ++truth_index){

			float pt_truth  = pt_particles_initial  -> at(truth_index);
			float phi_truth = phi_particles_initial -> at(truth_index);
			float eta_truth = eta_particles_initial -> at(truth_index);
			float q_truth   = q_particles_initial   -> at(truth_index);
			float qOverPt_truth = q_truth / pt_truth;

			if (DISPLAY_ONLY_TRUTH_IN_REGION){	

				if (DISPLAY_NEIGHBOURING_REGION_TRUTH){

					if ((eta_truth > (eta_below_reg.center - eta_reg.width) && eta_truth < (eta_above_reg.center + eta_reg.width)) 	&& (phi_truth > (phi_reg.center - phi_reg.width) && phi_truth < (phi_reg.center + phi_reg.width)) ||
						(eta_truth > (eta_reg.center - eta_reg.width) && eta_truth < (eta_reg.center + eta_reg.width)) 	&& (phi_truth > (phi_below_reg.center - phi_reg.width) && phi_truth < (phi_above_reg.center + phi_reg.width))){

						if (phi_truth > Phi_min && phi_truth < Phi_max && qOverPt_truth > qOverPt_min && qOverPt_truth < qOverPt_max){
		
							TMarker *point = new TMarker(phi_truth, qOverPt_truth, 8);
							point -> Draw("same");
							point -> SetMarkerColor(kBlue-7);
							point -> SetMarkerSize(4);

							std::cout << "... truth_phi = " << phi_truth << std::endl;
							std::cout << "... truth_eta = " << eta_truth <<  std::endl;
							std::cout << "... truth_q_over_pt = " << qOverPt_truth << std::endl;
							std::cout << std::endl;

							phi_comparison = phi_truth;
							eta_comparison = eta_truth;
							q_over_pt_comparison = qOverPt_truth;
						}
					}
				}

				if ((eta_truth > (eta_reg.center - eta_reg.width) && eta_truth < (eta_reg.center + eta_reg.width)) && (phi_truth > (phi_reg.center - phi_reg.width) && phi_truth < (phi_reg.center + phi_reg.width))){

					if (phi_truth > Phi_min && phi_truth < Phi_max && qOverPt_truth > qOverPt_min && qOverPt_truth < qOverPt_max){
	
						TMarker *point = new TMarker(phi_truth, qOverPt_truth, 8);
						point -> Draw("same");
						point -> SetMarkerColor(kBlue+3);
						point -> SetMarkerSize(2);
					}
				}
			
			
			float qOverPt_truth = q_truth / pt_truth;

			float qOverPt_truth = q_truth / pt_truth;
				
			} else {

				if (phi_truth > Phi_min && phi_truth < Phi_max && qOverPt_truth > qOverPt_min && qOverPt_truth < qOverPt_max){

					TMarker *point = new TMarker(phi_truth, qOverPt_truth, 8);
					point -> Draw("same");
					point -> SetMarkerColor(kBlue);
					point -> SetMarkerSize(2);
				}
			} 
		}
	}


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
		}
	}


	//////////////////////////////////////////////
	// draw lines around the main accumulator area
	//////////////////////////////////////////////
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


	//////////////////////////////////
	// draw safe zones for one regions
	//////////////////////////////////
	if (DISPAY_PHI_REGION_SAFE_ZONE){
		TLine *line_vertical_left   = new TLine(phi_reg.center - phi_reg.width, qOverPt_min, phi_reg.center - phi_reg.width, qOverPt_max);
		TLine *line_vertical_right  = new TLine(phi_reg.center + phi_reg.width, qOverPt_min, phi_reg.center + phi_reg.width, qOverPt_max);

		line_vertical_left  ->  SetLineColor(kRed);
		line_vertical_left  ->  SetLineWidth(4);
		line_vertical_left  ->  Draw("same");

		line_vertical_right  ->  SetLineColor(kRed);
		line_vertical_right  ->  SetLineWidth(4);
		line_vertical_right  ->  Draw("same");
	}


	////////////////////////
	// last comments
	////////////////////////
	//hist  -> GetYaxis() -> SetTitleOffset(1.);
	c1 -> SaveAs("output/Accumulator_Plot.pdf");

}