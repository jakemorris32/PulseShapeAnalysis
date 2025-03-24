#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"
#include "TF1.h"  // Properly include TF1
#include "TFitResult.h"  // Add TFitResult header
#include "TFitResultPtr.h"  // Add TFitResultPtr header
#include <iostream>
#include <cmath>

void single_exp(const char* fileName) {

    TFile* file = TFile::Open(fileName, "Update");
    if (!file || file->IsZombie()) {
        std::cerr << "Error opening file: " << fileName << std::endl;
        return;
    }
    
    TTree* tree = dynamic_cast<TTree*>(file->Get("adjustedTree"));
    if (!tree) {
        std::cerr << "Error retrieving tree from file" << std::endl;
        file->Close();
        return;
    }

    const Int_t nSamples = 10000;
    Double_t pulse[nSamples];
    tree->SetBranchAddress("t0aligned_cfd0.10", pulse);

    Double_t Amp, Tau;

    TBranch *brAmp = tree->Branch("Amp", &Amp, "Amp/D");
    TBranch *brTau = tree->Branch("Tau", &Tau, "Tau/D");
    
    TF1* fitFunc = new TF1("fitFunc", "[0]*exp(-x/[1])", 0, nSamples);

    Long64_t nEntries = tree->GetEntries();
    for (Long64_t i = 0; i < nEntries; i++) {
        tree->GetEntry(i);

        // Find the pulse peak
        int peakIndex = 0;
        double peakVal = pulse[0];
        for (int j = 1; j < nSamples; j++) {
            if (pulse[j] > peakVal) {
                peakVal = pulse[j];
                peakIndex = j;
            }
        }

        // Create histogram for tail
        int N_tail = nSamples - peakIndex;
        TH1D hTail("hTail", "Pulse decay tail", N_tail, 0, N_tail);
        for (int k = 0; k < N_tail; ++k) {  // Fixed syntax error in loop
            // ROOT bins start at 1, bin 0 is underflow
            hTail.SetBinContent(k+1, pulse[peakIndex + k]);
        }

        // Fit Single Exponential
        fitFunc->SetParameters(pulse[peakIndex], N_tail/5.0);
        fitFunc->SetRange(0, N_tail);
        
        // Use R for the specified range and Q for quiet mode
        TFitResultPtr fitResult = hTail.Fit(fitFunc, "QRS");
        
        // Check if fit succeeded
        if (fitResult->IsValid()) {
            Amp = fitFunc->GetParameter(0);
            Tau = fitFunc->GetParameter(1);
        } else {
            // Default values if fit fails
            Amp = pulse[peakIndex];
            Tau = -1.0; // Indicating fit failure
            if (i % 500 == 0) {
                std::cout << "Event " << i << ": Fit failed" << std::endl;
            }
        }

        // Print Tau and Amp every 500 events
        if (i % 500 == 0 && fitResult->IsValid()) {
            std::cout << "Event " << i << ": Tau = " << Tau << ", Amp = " << Amp << std::endl;
        }

        brAmp->Fill();
        brTau->Fill();
    }

    tree->Write("", TObject::kOverwrite);
    file->Close();
    delete fitFunc; // Clean up the fit function
    
    std::cout << "Fit parameters added to tree for " << nEntries << " events.\n";
}

void double_exp(const char* fileName) {
    TFile* file = TFile::Open(fileName, "Update");
    TTree* tree = dynamic_cast<TTree*>(file->Get("adjustedTree"));
    
    const Int_t nSamples = 10000;
    Double_t pulse[nSamples];
    tree->SetBranchAddress("t0aligned_cfd0.10", pulse);

    // Parameters for double exponential fit
    Double_t Amp1, Tau1, Amp2, Tau2;

    // Create branches for the parameters
    TBranch *brAmp1 = tree->Branch("Amp1", &Amp1, "Amp1/D");
    TBranch *brTau1 = tree->Branch("Tau1", &Tau1, "Tau1/D");
    TBranch *brAmp2 = tree->Branch("Amp2", &Amp2, "Amp2/D");
    TBranch *brTau2 = tree->Branch("Tau2", &Tau2, "Tau2/D");
    
    // Define the double exponential function: [0]*exp(-x/[1]) + [2]*exp(-x/[3])
    TF1* fitFunc = new TF1("fitFunc", "[0]*exp(-x/[1]) + [2]*exp(-x/[3])", 0, nSamples);

    Long64_t nEntries = tree->GetEntries();
    for (Long64_t i = 0; i < nEntries; i++) {
        tree->GetEntry(i);

        // Find the pulse peak
        int peakIndex = 0;
        double peakVal = pulse[0];
        for (int j = 1; j < nSamples; j++) {
            if (pulse[j] > peakVal) {
                peakVal = pulse[j];
                peakIndex = j;
            }
        }

        // Create histogram for tail
        int N_tail = nSamples - peakIndex;
        TH1D hTail("hTail", "Pulse decay tail", N_tail, 0, N_tail);
        for (int k = 0; k < N_tail; ++k) {
            // ROOT bins start at 1, bin 0 is underflow
            hTail.SetBinContent(k+1, pulse[peakIndex + k]);
        }

        // Set initial parameters for double exponential fit
        // Assuming the first component has higher amplitude but shorter decay
        fitFunc->SetParameters(pulse[peakIndex] * 0.7, N_tail/10.0,  // Fast component
                               pulse[peakIndex] * 0.3, N_tail/3.0);   // Slow component
        
        // Set parameter limits to ensure physical results
        fitFunc->SetParLimits(0, 0, pulse[peakIndex] * 2); // Amp1
        fitFunc->SetParLimits(1, 1, N_tail);               // Tau1
        fitFunc->SetParLimits(2, 0, pulse[peakIndex] * 2); // Amp2
        fitFunc->SetParLimits(3, 1, N_tail * 2);           // Tau2
        
        fitFunc->SetRange(0, N_tail);
        
        // Use R for the specified range, S for saving fit info, and Q for quiet mode
        TFitResultPtr fitResult = hTail.Fit(fitFunc, "QRS");
        
        // Check if fit succeeded
        if (fitResult->IsValid()) {
            Amp1 = fitFunc->GetParameter(0);
            Tau1 = fitFunc->GetParameter(1);
            Amp2 = fitFunc->GetParameter(2);
            Tau2 = fitFunc->GetParameter(3);
            
            // Sort components by time constant (Tau1 < Tau2)
            if (Tau1 > Tau2) {
                std::swap(Amp1, Amp2);
                std::swap(Tau1, Tau2);
            }
        } else {
            // Default values if fit fails
            Amp1 = pulse[peakIndex] * 0.7;
            Tau1 = -1.0; // Indicating fit failure
            Amp2 = pulse[peakIndex] * 0.3;
            Tau2 = -1.0;
            if (i % 500 == 0) {
                std::cout << "Event " << i << ": Double exponential fit failed" << std::endl;
            }
        }

        // Print parameters every 500 events
        if (i % 500 == 0 && fitResult->IsValid()) {
            std::cout << "Event " << i << ": Tau1 = " << Tau1 
                      << ", Amp1 = " << Amp1 
                      << ", Tau2 = " << Tau2 
                      << ", Amp2 = " << Amp2 << std::endl;
        }

        brAmp1->Fill();
        brTau1->Fill();
        brAmp2->Fill();
        brTau2->Fill();
    }

    tree->Write("", TObject::kOverwrite);
    file->Close();
    delete fitFunc; // Clean up the fit function
    
    std::cout << "Double exponential fit parameters added to tree for " << nEntries << " events.\n";
}