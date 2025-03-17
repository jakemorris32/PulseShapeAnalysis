#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"
#include <iostream>
#include <cmath>

void qdc(const char* fileLocation)
{

    TFile* file0 = TFile::Open(fileLocation, "UPDATE");
    TTree* tree = dynamic_cast<TTree*>(file0->Get("adjustedTree"));

    const int nSamples = 10000;              // length of the waveform array
    double pulse[nSamples]; 

    tree->SetBranchAddress("t0aligned_cfd0.10", pulse);

    double Q1val, Q2val = 0.0;
    TBranch* Q1Branch = tree->Branch("Q1_val", &Q1val, "Q1_val/D");
    TBranch* Q2Branch = tree->Branch("Q2_val", &Q2val, "Q2_val/D");


    int t0 = 1000;
    int t1 = 2500;
    int t2 = 4000;
    double Q1 = 0.0;
    double Q2 = 0.0;

    Long64_t nEntries = tree->GetEntries();
    for (Long64_t i =0; i < nEntries; ++i) {
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
}