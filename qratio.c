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
			int nBins = 500, double lowRange = -1, double highRange = -1,
			bool plot = false)

{
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
	
	// Find a range focused on the main peak if not specified
	if (lowRange < 0 || highRange < 0) {
		// Create temporary histograms with very wide range
		TH1D* tempH1 = new TH1D("tempH1", "", 1000, 0.0, 3.0);
		TH1D* tempH2 = new TH1D("tempH2", "", 1000, 0.0, 3.0);
		
		// Fill temporary histograms
		tree1->SetBranchAddress(Q1BranchAddress, &Q1);
		tree1->SetBranchAddress(Q2BranchAddress, &Q2);
		Long64_t nEntries1 = tree1->GetEntries();
		
		for (Long64_t i = 0; i < nEntries1; i++) {
			tree1->GetEntry(i);
			if (Q1 != 0) {
				double ratio = Q2 / Q1;
				if (std::isfinite(ratio) && ratio > 0 && ratio < 3.0) {
					tempH1->Fill(ratio);
				}
			}
		}
		
		tree2->SetBranchAddress(Q1BranchAddress, &Q1);
		tree2->SetBranchAddress(Q2BranchAddress, &Q2);
		Long64_t nEntries2 = tree2->GetEntries();
		
		for (Long64_t i = 0; i < nEntries2; i++) {
			tree2->GetEntry(i);
			if (Q1 != 0) {
				double ratio = Q2 / Q1;
				if (std::isfinite(ratio) && ratio > 0 && ratio < 3.0) {
					tempH2->Fill(ratio);
				}
			}
		}
		
		// Find bins with maximum content (peaks)
		int maxBin1 = tempH1->GetMaximumBin();
		int maxBin2 = tempH2->GetMaximumBin();
		
		// Get x-values at these bins (peak positions)
		double peakPos1 = tempH1->GetXaxis()->GetBinCenter(maxBin1);
		double peakPos2 = tempH2->GetXaxis()->GetBinCenter(maxBin2);
		
		// Set range to be around the peak positions
		// Use a fixed width around each peak
		double width = 0.3; // adjust based on expected width of your distributions
		
		double low1 = peakPos1 - width;
		double high1 = peakPos1 + width;
		double low2 = peakPos2 - width;
		double high2 = peakPos2 + width;
		
		// Take the wider range to ensure both distributions are visible
		if (lowRange < 0) lowRange = std::min(low1, low2);
		if (highRange < 0) highRange = std::max(high1, high2);
		
		std::cout << "Range focused around peaks: [" << lowRange << ", " << highRange << "]" << std::endl;
		std::cout << "Peak positions: " << peakPos1 << ", " << peakPos2 << std::endl;
		
		// Clean up
		delete tempH1;
		delete tempH2;
	}
	
	// Create histograms with the determined range
	TH1D* h1 = new TH1D("h1", "", nBins, lowRange, highRange);
	TH1D* h2 = new TH1D("h2", "", nBins, lowRange, highRange);
	
	// Fill histograms using the calculated ratios
	tree1->SetBranchAddress(Q1BranchAddress, &Q1);
	tree1->SetBranchAddress(Q2BranchAddress, &Q2);
	Long64_t nEntries1 = tree1->GetEntries();
	
	for (Long64_t i = 0; i < nEntries1; i++) {
		tree1->GetEntry(i);
		if (Q1 != 0) {
			double ratio = Q2 / Q1;
			if (std::isfinite(ratio)) { // Check for NaN or Inf
				h1->Fill(ratio);
			}
		}
	}
	
	tree2->SetBranchAddress(Q1BranchAddress, &Q1);
	tree2->SetBranchAddress(Q2BranchAddress, &Q2);
	Long64_t nEntries2 = tree2->GetEntries();
	
	for (Long64_t i = 0; i < nEntries2; i++) {
		tree2->GetEntry(i);
		if (Q1 != 0) {
			double ratio = Q2 / Q1;
			if (std::isfinite(ratio)) { // Check for NaN or Inf
				h2->Fill(ratio);
			}
		}
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
	std::ofstream txtOut;
	txtOut.open("output.txt", std::ios::app);  // Fixed duplicate line
	txtOut << fom << std::endl;
	txtOut.close();
	
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
	
		TLegend* legend = new TLegend(0.7, 0.7, 0.9, 0.9);
		legend->AddEntry(h1, "10 degrees", "l");
		legend->AddEntry(h2, "30 degrees", "l");
		legend->Draw();
	
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
