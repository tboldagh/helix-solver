void Create_Root_File(){

	// for box coordinates
	TFile hfile_box("hough_tree_BoxPosition.root","RECREATE","ROOT file containing sorted coordinates of accumulator cells");
	TTree tree_box("tree", "tree");
  	tree_box.ReadFile("data_BoxPosition.csv", "Phi_begin:qOverPt_begin:Phi_end:qOverPt_end:divLevel");

	tree_box.Fill();
	tree_box.Write();
	hfile_box.Close();

	// for sets of solutions
	TFile hfile_solution("hough_tree_SolutionPair.root","RECREATE","ROOT file containing sorted coordinates of accumulator cells");
	TTree tree_solution("tree", "tree");
	tree_solution.ReadFile("data_SolutionPair.csv", "qOverPt_solution:Phi_solution");

	tree_solution.Fill();
	tree_solution.Write();
	hfile_solution.Close();

	// to draw all the lines - radius, phi
	TFile hfile_R_Phi("hough_tree_RPhi.root","RECREATE","ROOT file containing sorted coordinates of accumulator cells");
	TTree tree_R_Phi("tree", "tree");
	tree_R_Phi.ReadFile("data_RPhi.csv", "radius:phi");

	tree_R_Phi.Fill();
	tree_R_Phi.Write();
	hfile_R_Phi.Close();

	// to draw all the lines - x, y positions of both ends
	TFile hfile_LinePosition("hough_tree_LinePosition.root","RECREATE","ROOT file containing sorted coordinates of accumulator cells");
	TTree tree_LinePosition("tree", "tree");
	tree_LinePosition.ReadFile("data_LinePosition.csv", "xLeft:yLeft:xRight:yRight");

	tree_LinePosition.Fill();
	tree_LinePosition.Write();
	hfile_LinePosition.Close();
}
