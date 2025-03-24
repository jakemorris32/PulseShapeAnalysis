#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"
#include "TLegend.h"
#include <iostream>

void bslAdjust()
{
	// Open the ROOT file and retrieve the TTree
	TFile* sourceFile = TFile::Open("/shared/storage/physnp/sp1357/MPhys_and_BSc/SummerProject17/data_NaI/degrees_10.root", "READ");
		
	TTree* sourceTree = dynamic_cast<TTree*>(sourceFile->Get("tree"));
	if (!sourceTree) {
		std::cerr << "Error getting source tree" << std::endl;
		sourceFile->Close();
		return;
	}

	// Create a new output file
	TFile* outputFile = TFile::Open("/shared/storage/physnp/jm2912/degrees_10_adjusted.root", "RECREATE");
	if (!outputFile || outputFile->IsZombie()) {
		std::cerr << "Error creating output file" << std::endl;
		sourceFile->Close();
		return;
	}
	
	// Create a new tree from scratch instead of cloning
	TTree* newTree = new TTree("adjustedTree", "Tree with baseline-adjusted data");

	// Set up variables and branch addresses
	static const int nSamples = 10000;
	double pd[nSamples];
	double baselineadjusted[nSamples];
	double baselines; 
	
	// Connect to source tree
	sourceTree->SetBranchAddress("pulsedata", pd);

	// Add only the branches we need to the new tree
	newTree->Branch("baselines", &baselines, "baselines/D");
	newTree->Branch("baseline_adjusted", baselineadjusted, "baseline_adjusted[10000]/D");

	// Loop over entries in the source TTree
	Long64_t nEntries = sourceTree->GetEntries();
	std::cout << "Processing " << nEntries << " entries..." << std::endl;
	
	for (Long64_t i = 0; i < nEntries; ++i) {
		sourceTree->GetEntry(i);

		// Compute baseline from the first 100 samples
		double baseline = 0.0;
		int baselineSamples = 100;
		for (int j = 0; j < baselineSamples; ++j) {
			baseline += pd[j];
		}

		baseline /= baselineSamples;

		// Adjust baseline
		for (Long64_t k = 0; k < nSamples; ++k) {
			baselineadjusted[k] = pd[k] - baseline; 
		}

		baselines = baseline;
		
		// Fill the new tree
		newTree->Fill();
		
		// Progress update
		if (i % 1000 == 0) {
			std::cout << "Processed " << i << " entries" << std::endl;
		}
	}

	// Write the new tree to the output file
	outputFile->cd();
	newTree->Write();

	// Clean up
	outputFile->Close();
	sourceFile->Close();
	
	std::cout << "Baseline adjustment completed. Output saved to /shared/storage/physnp/jm1912/degrees_10_adjusted.root" << std::endl;
}