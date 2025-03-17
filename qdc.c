#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"
#include <iostream>
#include <string>  // Add string header
#include <cmath>

void qdc(const char* fileLocation, int t1, int t2)
{
    TFile* file0 = TFile::Open(fileLocation, "UPDATE");
        TTree* tree = dynamic_cast<TTree*>(file0->Get("adjustedTree"));
    
    const int nSamples = 10000;              // length of the waveform array
    double pulse[nSamples]; 

    tree->SetBranchAddress("t0aligned_cfd0.10", pulse);

    // Use std::string for branch names
    std::string Q1BranchName = Form("Q1_%d_%d_val", t1, t2);
    std::string Q2BranchName = Form("Q2_%d_%d_val", t1, t2);
    
    // Print branch names for debugging
    std::cout << "Creating branches: " << Q1BranchName << " and " << Q2BranchName << std::endl;
    
    double Q1val = 0.0, Q2val = 0.0;  // Initialize both values
    
    // Use c_str() to convert std::string to const char*
    TBranch* Q1Branch = tree->Branch(Q1BranchName.c_str(), &Q1val, Form("%s/D", Q1BranchName.c_str()));
    TBranch* Q2Branch = tree->Branch(Q2BranchName.c_str(), &Q2val, Form("%s/D", Q2BranchName.c_str()));
    
    int t0 = 1000;
    double Q1 = 0.0;
    double Q2 = 0.0;

    Long64_t nEntries = tree->GetEntries();
    for (Long64_t i = 0; i < nEntries; ++i) {
        tree->GetEntry(i);

        Q1 = 0.0;
        Q2 = 0.0;

        for (int j = t0; j < t1; ++j) {
            Q1 += pulse[j];
        }

        for (int j = t0; j < t2; ++j) {
            Q2 += pulse[j];
        }
        Q1val = Q1;
        Q2val = Q2;
        Q1Branch->Fill();
        Q2Branch->Fill();
    }

    tree->Write();
    file0->Close();
    
    std::cout << "QDC calculation completed for t1=" << t1 << " and t2=" << t2 << std::endl;
}