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

void func_hist(const char* fileLocation1, const char* fileLocation2,
			    const char* BranchAddress,  
			    bool plot = false,
			    int nBins = 500, double lowRange = 1000, double highRange = 1200,
				const char* plotName = "func_hist.png"
			    ) {

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
	
	double p1, p2;
	
	tree1->SetBranchAddress(BranchAddress, &p1);
	Long64_t nEntries1 = tree1->GetEntries();

	tree2->SetBranchAddress(BranchAddress, &p2);
	Long64_t nEntries2 = tree2->GetEntries();


	TH1D* h1 = new TH1D("h1", "", nBins, lowRange, highRange);
	TH1D* h2 = new TH1D("h2", "", nBins, lowRange, highRange);
	
	// Fill histograms with the p1 and p2
    for (Long64_t i = 0; i < nEntries1; ++i) {
        tree1->GetEntry(i);
        h1->Fill(p1);
    }

    for (Long64_t i = 0; i < nEntries2; ++i) {
        tree2->GetEntry(i);
        h2->Fill(p2);
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

	if (plot == true){
		// Plot both histograms
		std::cout << "Plotting histograms..." << std::endl;
		h1->SetTitle(BranchAddress);
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
	
		c1->SaveAs(plotName);
		
	}

	// Clean up
	delete h1;
	delete h2;
	delete g1;
	delete g2;
	file1->Close();
	file2->Close();
}

void plot_all_params() {
	// Plot Amp, Tau, Amp1, Tau1, Amp2, Tau2
	func_hist("/shared/storage/physnp/jm2912/degrees_10_adjusted.root",
			  "/shared/storage/physnp/jm2912/degrees_30_adjusted.root",
			  "Amp", true, 500, 0, 1000, "Amp.png");
	func_hist("/shared/storage/physnp/jm2912/degrees_10_adjusted.root",
			  "/shared/storage/physnp/jm2912/degrees_30_adjusted.root",
			  "Tau", true, 500, 1000, 1200, "Tau.png");
	func_hist("/shared/storage/physnp/jm2912/degrees_10_adjusted.root",
			  "/shared/storage/physnp/jm2912/degrees_30_adjusted.root",
			  "Amp1", true, 500, 0, 1000, "Amp1.png");
	func_hist("/shared/storage/physnp/jm2912/degrees_10_adjusted.root",
			  "/shared/storage/physnp/jm2912/degrees_30_adjusted.root",
			  "Tau1", true, 500, 1000, 1200, "Tau1.png");
	func_hist("/shared/storage/physnp/jm2912/degrees_10_adjusted.root",
			  "/shared/storage/physnp/jm2912/degrees_30_adjusted.root",
			  "Amp2", true, 500, 0, 1000, "Amp2.png");
	func_hist("/shared/storage/physnp/jm2912/degrees_10_adjusted.root",
			  "/shared/storage/physnp/jm2912/degrees_30_adjusted.root",
			  "Tau2", true, 500, 1000, 1200, "Tau2.png");
}