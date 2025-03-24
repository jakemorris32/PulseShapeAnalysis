#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TLegend.h"
#include <iostream>
#include <cmath>

void plot(){

    TFile* file1 = TFile::Open("/shared/storage/physnp/jm2912/degrees_30_adjusted.root", "READ");

    TTree* tree1 = dynamic_cast<TTree*>(file1->Get("adjustedTree"));

    const int nSamples = 6000;
    int nEntries1 = tree1->GetEntries();
    double pulse1[nSamples];

    tree1->SetBranchAddress("t0aligned_cfd0.10", pulse1);

    double sum1[nSamples] = {0};

    for (int i = 0; i < nEntries1; ++i) {
        tree1->GetEntry(i);
        for (int j = 0; j < nSamples; ++j) {
            sum1[j] += pulse1[j];
        }
    }

    double avg1[nSamples];
    for (int j = 0; j < nSamples; ++j) {
        avg1[j] = sum1[j] / nEntries1;
    }

    double xVals[nSamples];
    for (int i = 0; i < nSamples; ++i) {
        xVals[i] = i;
    }

    TCanvas* c1 = new TCanvas("c1", "Average Waveforms", 1600, 1200);

    TGraph* g1 = new TGraph(nSamples, xVals, avg1);


    
    g1->SetLineWidth(4);

    g1->Draw("AL");
    g1->GetXaxis()->SetLimits(0, 6000);
    g1->GetHistogram()->SetAxisRange(0, 6000, "X");

    
    c1->SaveAs("average_plot10.png");

    file1->Close();

}