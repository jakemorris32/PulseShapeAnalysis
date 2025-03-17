#include "TFile.h"
#include "TTree.h"
#include "TGraph.h"
#include "TCanvas.h"
#include <iostream>
#include <cmath>

void diagnostic()
{
    // Open the ROOT file
    TFile* file = TFile::Open("degrees_10.root", "READ");
    if (!file || file->IsZombie()) {
        std::cerr << "Error opening file" << std::endl;
        return;
    }
    
    TTree* tree = dynamic_cast<TTree*>(file->Get("tree"));
    if (!tree) {
        std::cerr << "Error getting tree" << std::endl;
        file->Close();
        return;
    }

    // Get aligned waveforms
    static const int nSamples = 10000;
    double t0_aligned[nSamples];
    tree->SetBranchAddress("t0aligned_cfd0.30", t0_aligned);
    
    // For averaging
    double avgWaveform[nSamples] = {0};
    int eventCount = 0;
    
    // For differential analysis
    double diffValues[nSamples-1] = {0};

    // Process all entries
    Long64_t nEntries = tree->GetEntries();
    for (Long64_t i = 0; i < nEntries; ++i) {
        tree->GetEntry(i);
        eventCount++;
        
        // Sum for average
        for (int j = 0; j < nSamples; ++j) {
            avgWaveform[j] += t0_aligned[j];
        }
    }
    
    // Calculate average
    for (int j = 0; j < nSamples; ++j) {
        avgWaveform[j] /= eventCount;
    }
    
    // Calculate first derivative to find discontinuities
    for (int j = 0; j < nSamples-1; ++j) {
        diffValues[j] = avgWaveform[j+1] - avgWaveform[j];
    }
    
    // Find maximum derivative (biggest discontinuity)
    int maxDiffIdx = 0;
    double maxDiffVal = 0;
    for (int j = 990; j < 1010; ++j) {
        if (fabs(diffValues[j]) > maxDiffVal) {
            maxDiffVal = fabs(diffValues[j]);
            maxDiffIdx = j;
        }
    }
    
    std::cout << "Maximum discontinuity found at index " << maxDiffIdx 
              << " with value " << diffValues[maxDiffIdx] << std::endl;
    
    // Output values around the bump
    std::cout << "Values around the bump:" << std::endl;
    for (int j = maxDiffIdx-5; j <= maxDiffIdx+5; ++j) {
        std::cout << "Index " << j << ": " << avgWaveform[j] 
                  << ", diff: " << diffValues[j] << std::endl;
    }
    
    // Draw the results
    TCanvas* c = new TCanvas("c", "Waveform Diagnostics", 1200, 800);
    c->Divide(2, 1);
    
    // Plot average waveform
    c->cd(1);
    double xValues[nSamples];
    for (int j = 0; j < nSamples; ++j) xValues[j] = j;
    
    TGraph* avgGraph = new TGraph(nSamples, xValues, avgWaveform);
    avgGraph->SetTitle("Average Waveform");
    avgGraph->GetXaxis()->SetRangeUser(980, 1020);
    avgGraph->Draw("AL");
    
    // Plot differential
    c->cd(2);
    TGraph* diffGraph = new TGraph(nSamples-1, xValues, diffValues);
    diffGraph->SetTitle("First Derivative");
    diffGraph->GetXaxis()->SetRangeUser(980, 1020);
    diffGraph->Draw("AL");
    
    c->SaveAs("waveform_diagnostic.png");
    
    file->Close();
}
