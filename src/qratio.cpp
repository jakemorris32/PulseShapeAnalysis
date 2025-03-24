#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"
#include "TF1.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <algorithm>
#include <TClass.h>

#include "TCanvas.h"
#include "TLegend.h"

void qratio(const char* fileLocation1, const char* fileLocation2,
			const char* Q1BranchAddress, const char* Q2BranchAddress,
			bool plot = false,
			bool write = true,
			int nBins = 500, double lowRange = -1, double highRange = -1
			)

{
	std::cout << plot << std::endl;
	// Open file1 and get tree
	TFile* file1 = TFile::Open(fileLocation1, "READ");
	if (!file1 || file1->IsZombie()) {
		std::cerr << "Error opening file1: " << fileLocation1 << std::endl;
		return;
	}
	TTree* tree1 = dynamic_cast<TTree*>(file1->Get("adjustedTree"));
	if (!tree1) {
		std::cerr << "Error getting tree from file1" << std::endl;
		file1->Close();
		return;
	}
	
	// Open file2 and get tree
	TFile* file2 = TFile::Open(fileLocation2, "READ");
	if (!file2 || file2->IsZombie()) {
		std::cerr << "Error opening file2: " << fileLocation2 << std::endl;
		file1->Close();
		return;
	}
	TTree* tree2 = dynamic_cast<TTree*>(file2->Get("adjustedTree"));
	if (!tree2) {
		std::cerr << "Error getting tree from file2" << std::endl;
		file1->Close();
		file2->Close();
		return;
	}
	
	double Q1, Q2;
	
	tree1->SetBranchAddress(Q1BranchAddress, &Q1);
	tree1->SetBranchAddress(Q2BranchAddress, &Q2);
	Long64_t nEntries1 = tree1->GetEntries();

	tree2->SetBranchAddress(Q1BranchAddress, &Q1);
	tree2->SetBranchAddress(Q2BranchAddress, &Q2);
	Long64_t nEntries2 = tree2->GetEntries();

	std::vector<double> ratios1, ratios2;
	ratios1.reserve(nEntries1);
	ratios2.reserve(nEntries2);

	// Calculate Q2/Q1 ratios for file1
	for (Long64_t i = 0; i < nEntries1; ++i) {
		tree1->GetEntry(i);
		if (Q1 != 0.0) {
			double r = Q2 / Q1;
			if (std::isfinite(r)) {
				ratios1.push_back(r);
			}
		}
	}
	
	// Calculate Q2/Q1 ratios for file2
	for (Long64_t i = 0; i < nEntries2; ++i) {
		tree2->GetEntry(i);
		if (Q1 != 0.0) {
			double r = Q2 / Q1;
			if (std::isfinite(r)) {
				ratios2.push_back(r);
			}
		}
	}

	// Determine range if not provided
	if (lowRange < 0 || highRange < 0) {
		if (!ratios1.empty() && !ratios2.empty()) {
			std::vector<double> allRatios = ratios1;	
			allRatios.insert(allRatios.end(), ratios2.begin(), ratios2.end());
			std::sort(allRatios.begin(), allRatios.end());
			// Calculate 5th and 95th percentile
			size_t i5 = static_cast<size_t>(0.05 * allRatios.size());
			size_t i95 = static_cast<size_t>(0.95 * allRatios.size());
			double p5 = allRatios[i5];
			double p95 = allRatios[i95];
			// Set range with 20% padding
			lowRange = p5;
			highRange = p95;
			double padding = 0.20 * (highRange - lowRange);
			lowRange -= padding;
			highRange += padding;
		}
		std::cout << "Range determined [" << lowRange << ", " << highRange << "]" << std::endl;
	}

	// Create histograms with the determined range
	TH1D* h1 = new TH1D("h1", "", nBins, lowRange, highRange);
	TH1D* h2 = new TH1D("h2", "", nBins, lowRange, highRange);
	
	// Fill histograms with the calculated ratios
	for (double r : ratios1) {
		h1->Fill(r);
	}
	
	for (double r : ratios2) {
		h2->Fill(r);
	}

	// Fit Gaussians to calculate parameters
	TF1* g1 = new TF1("g1", "gaus", lowRange, highRange);
	g1->SetParameters(h1->GetMaximum(), h1->GetMean(), h1->GetRMS());
	h1->Fit(g1, "Q"); // Q = quiet mode
	
	TF1* g2 = new TF1("g2", "gaus", lowRange, highRange);
	g2->SetParameters(h2->GetMaximum(), h2->GetMean(), h2->GetRMS());
	h2->Fit(g2, "Q"); // Q = quiet mode
	
	double mean1 = g1->GetParameter(1);
	double sigma1 = g1->GetParameter(2);
	double fwhm1 = 2.355 * sigma1;
	
	double mean2 = g2->GetParameter(1);
	double sigma2 = g2->GetParameter(2);
	double fwhm2 = 2.355 * sigma2;

	double fom = (mean1 - mean2) / (fwhm1 + fwhm2);

	if (write == true){
		std::ofstream txtOut;
		txtOut.open("output.txt", std::ios::app);  // Fixed duplicate line
		txtOut << fom << std::endl;
		txtOut.close();
	}

	if (plot == true){
		// Plot both histograms
		std::cout << "Plotting histograms..." << std::endl;
		h1->SetLineColor(kRed);
		g1->SetLineColor(kRed);
		h2->SetLineColor(kBlue);
		g2->SetLineColor(kBlue);
		
		TCanvas* c1 = new TCanvas("c1", "Q Ratio Comparison", 800, 600);
		h1->SetStats(0); // Remove statistics box from h1
		h1->Draw();
		h2->Draw("SAME");
		
		// Draw fit functions
		g1->Draw("SAME");
		g2->Draw("SAME");
	
		c1->SaveAs("QPLOT.png");
		
	}



		// Clean up
	delete h1;
	delete h2;
	delete g1;
	delete g2;
	file1->Close();
	file2->Close();
}
