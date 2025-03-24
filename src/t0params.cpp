#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TLegend.h"
#include <iostream>
#include <cmath>

void plot(){

    TFile* file0 = TFile::Open("degrees_10.root", "READ");
    TTree* tree = dynamic_cast<TTree*>(file0->Get("tree"));

    const int nSamples = 6000;
    int nEntries = tree->GetEntries();
    double pulse20[nSamples];
    double pulse10[nSamples];
    double pulse05[nSamples];
    double pulse03[nSamples];

    tree->SetBranchAddress("t0aligned_cfd0.20", pulse20);
    tree->SetBranchAddress("t0aligned_cfd0.10", pulse10);
    tree->SetBranchAddress("t0aligned_cfd0.05", pulse05);
    tree->SetBranchAddress("t0aligned_cfd0.03", pulse03);

    double sum20[nSamples] = {0};
    double sum10[nSamples] = {0};
    double sum05[nSamples] = {0};
    double sum03[nSamples] = {0};

    for (int i = 0; i < nEntries; ++i) {
        tree->GetEntry(i);
        for (int j = 0; j < nSamples; ++j) {
            sum20[j] += pulse20[j];
            sum10[j] += pulse10[j];
            sum05[j] += pulse05[j];
            sum03[j] += pulse03[j];
        }
    }

    double avg20[nSamples], avg10[nSamples], avg05[nSamples], avg03[nSamples];
    for (int j = 0; j < nSamples; ++j) {
        avg20[j] = sum20[j] / nEntries;
        avg10[j] = sum10[j] / nEntries;
        avg05[j] = sum05[j] / nEntries;
        avg03[j] = sum03[j] / nEntries;
    }

    double xVals[nSamples];
    for (int i = 0; i < nSamples; ++i) {
        xVals[i] = i;
    }

    TCanvas* c1 = new TCanvas("c1", "Average Waveforms", 1600, 1200);

    TGraph* g20 = new TGraph(nSamples, xVals, avg20);
    TGraph* g10 = new TGraph(nSamples, xVals, avg10);
    TGraph* g05 = new TGraph(nSamples, xVals, avg05);
    TGraph* g03 = new TGraph(nSamples, xVals, avg03);


    g20->SetLineColor(kAzure + 1);
    g10->SetLineColor(kOrange + 1);
    g05->SetLineColor(kTeal + 2);
    g03->SetLineColor(kViolet -3);
    
    g20->SetLineWidth(4);
    g10->SetLineWidth(4);
    g05->SetLineWidth(4);
    g03->SetLineWidth(4);

    g20->Draw("AL");
    g20->GetXaxis()->SetLimits(900, 1100);
    g20->GetHistogram()->SetAxisRange(900, 1100, "X");

    g10->Draw("L SAME");
    g05->Draw("L SAME");
    g03->Draw("L SAME");
    
    TLegend *leg = new TLegend(0.7, 0.7, 0.9, 0.9);
    leg->SetBorderSize(0);
    leg->SetFillStyle(0);
    leg->AddEntry(g20, "CFD Fraction: 0.20", "l");
    leg->AddEntry(g10, "CFD Fraction: 0.10", "l");
    leg->AddEntry(g05, "CFD Fraction: 0.05", "l");
    leg->AddEntry(g03, "CFD Fraction: 0.03", "l");
    leg->Draw();
    
    c1->SaveAs("average_waveforms_zoom.png");

    file0->Close();

}