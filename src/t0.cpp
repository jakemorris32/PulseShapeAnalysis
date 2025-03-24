#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"
#include <iostream>
#include <cmath>

void t0(const char* fileLocation, double cfdFraction = 0.1)
{
	// Open the ROOT file using the provided file location
	TFile* file = TFile::Open(fileLocation, "UPDATE");
	if (!file || file->IsZombie()) {
		std::cerr << "Error opening file: " << fileLocation << std::endl;
		return;
	}
	
	TTree* tree = dynamic_cast<TTree*>(file->Get("adjustedTree"));
	if (!tree) {
		std::cerr << "Error getting tree" << std::endl;
		file->Close();
		return;
	}

	// Set up variables and branch addresses
	static const int nSamples = 10000;
	double baselineAdjusted[nSamples];
	double t0_value;
	double t0_aligned[nSamples];
	
	// Get the branch with baseline adjusted data
	tree->SetBranchAddress("baseline_adjusted", baselineAdjusted);

	// Create new branches for t0 and t0_aligned
	// Use branch names that include the cfdFraction value to distinguish them
	std::string t0BranchName = Form("t0_cfd%.2f", cfdFraction);
	std::string t0AlignedBranchName = Form("t0aligned_cfd%.2f", cfdFraction);
	
	TBranch* t0Branch = tree->Branch(t0BranchName.c_str(), &t0_value, (t0BranchName+"/D").c_str());
	TBranch* t0AlignedBranch = tree->Branch(t0AlignedBranchName.c_str(), t0_aligned, 
										   (t0AlignedBranchName+"[10000]/D").c_str());

	std::cout << "Using CFD fraction: " << cfdFraction << std::endl;
	std::cout << "Created branches: " << t0BranchName << " and " << t0AlignedBranchName << std::endl;

	// Loop over entries in the TTree
	Long64_t nEntries = tree->GetEntries();
	for (Long64_t i = 0; i < nEntries; ++i) {
		tree->GetEntry(i);

		// Find the maximum amplitude of the pulse
		double maxAmplitude = 0.0;
		int maxIndex = 0;
		for (int j = 0; j < nSamples; ++j) {
			if (fabs(baselineAdjusted[j]) > maxAmplitude) {
				maxAmplitude = fabs(baselineAdjusted[j]);
				maxIndex = j;
			}
		}

		// Determine pulse polarity
		bool isNegativePulse = baselineAdjusted[maxIndex] < 0;
		
		// Calculate threshold for CFD using the provided fraction
		double threshold = cfdFraction * maxAmplitude;
		if (isNegativePulse) threshold = -threshold;
		
		// Find t0 - where the pulse crosses the threshold
		t0_value = -1; // Default value if threshold not found
		
		// Look for threshold crossing before the maximum
		for (int j = 0; j < maxIndex; ++j) {
			if ((isNegativePulse && baselineAdjusted[j] > threshold && baselineAdjusted[j+1] <= threshold) ||
				(!isNegativePulse && baselineAdjusted[j] < threshold && baselineAdjusted[j+1] >= threshold)) {
				// Simply use the first index where we cross threshold
				t0_value = j;
				break;
			}
		}
		
		// Very simple alignment approach - just integer shift
		if (t0_value >= 0) {
			// Initialize with zeros
			for (int j = 0; j < nSamples; ++j) {
				t0_aligned[j] = 0.0;
			}
			
			// Calculate simple integer shift
			int t0_int = static_cast<int>(t0_value); // truncate to integer
			int shift = 1000 - t0_int;  // shift amount to move t0 to index 1000
			
			// Plain shift - copy each value to its new position
			for (int j = 0; j < nSamples; ++j) {
				int sourceIdx = j - shift;
				if (sourceIdx >= 0 && sourceIdx < nSamples) {
					t0_aligned[j] = baselineAdjusted[sourceIdx];
				}
				// else leave as zero
			}
			
			if (i < 5) {
				std::cout << "Event " << i << ": t0=" << t0_value 
						  << ", t0_int=" << t0_int
						  << ", shift=" << shift << std::endl;
			}
		} else {
			// If t0 not found, just copy the original data
			for (int j = 0; j < nSamples; ++j) {
				t0_aligned[j] = baselineAdjusted[j];
			}
		}
		
		// Fill the branches
		t0Branch->Fill();
		t0AlignedBranch->Fill();
	}

	// Write the updated tree
	tree->Write("", TObject::kOverwrite);

	// Close the file
	file->Close();
	
	std::cout << "T0 alignment completed successfully with CFD fraction: " << cfdFraction << std::endl;
}