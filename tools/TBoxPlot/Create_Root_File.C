void Create_Root_File(){

	// for box coordinates
	ifstream file_BoxPosition("data_BoxPosition.csv");
	if (file_BoxPosition.peek() != std::ifstream::traits_type::eof()){
	TFile hfile_box("hough_tree_BoxPosition.root","RECREATE","ROOT file containing sorted coordinates of accumulator cells");
	TTree tree_box("tree", "tree");
  	tree_box.ReadFile("data_BoxPosition.csv", "Phi_begin:qOverPt_begin:Phi_end:qOverPt_end:divLevel/I");

	tree_box.Fill();
	tree_box.Write();
	hfile_box.Close();
	}

	// for sets of solutions
	ifstream file_SolutionPair("data_SolutionPair.csv");
	if (file_SolutionPair.peek() != std::ifstream::traits_type::eof()){
	TFile hfile_solution("hough_tree_SolutionPair.root","RECREATE","ROOT file containing sorted coordinates of accumulator cells");
	TTree tree_solution("tree", "tree");
	tree_solution.ReadFile("data_SolutionPair.csv", "qOverPt_solution:Phi_solution:xLeftSolution:yLeftSolution:xRightSolution:yRightSolution:divisionLevel/I");

	tree_solution.Fill();
	tree_solution.Write();
	hfile_solution.Close();
	}

	// to draw all the lines - radius, phi
	ifstream file_RPhi("data_RPhi.csv");
	if (file_RPhi.peek() != std::ifstream::traits_type::eof()){
	TFile hfile_R_Phi("hough_tree_RPhi.root","RECREATE","ROOT file containing sorted coordinates of accumulator cells");
	TTree tree_R_Phi("tree", "tree");
	tree_R_Phi.ReadFile("data_RPhi.csv", "radius:phi");

	tree_R_Phi.Fill();
	tree_R_Phi.Write();
	hfile_R_Phi.Close();
	}

	// to get all the events ised in the procedure
	ifstream file_Events("data_Events.csv");
	if (file_Events.peek() != std::ifstream::traits_type::eof()){
	TFile hfile_Events("hough_events.root","RECREATE","ROOT file containing sorted coordinates of accumulator cells");
	TTree tree_Events("tree", "tree");
	tree_Events.ReadFile("data_Events.csv", "event_id/I");

	tree_Events.Fill();
	tree_Events.Write();
	hfile_Events.Close();
	}

	// to get mean and standard deviation for each cell
	ifstream file_Mean("data_MeanStdev.csv");
	if (file_Mean.peek() != std::ifstream::traits_type::eof()){
	TFile hfile_Mean("hough_mean_stdev.root","RECREATE","ROOT file containing sorted coordinates of accumulator cells");
	TTree tree_Mean("tree", "tree");
	tree_Mean.ReadFile("data_MeanStdev.csv", "mean_phi:stdev_phi:qOverPt_mean:qOverPt_stdev");

	tree_Mean.Fill();
	tree_Mean.Write();
	hfile_Mean.Close();
	}
}
