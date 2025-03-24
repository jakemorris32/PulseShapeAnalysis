#include "TFile.h"
#include "TTree.h"
#include "TGraph.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TLine.h"
#include <iostream>

void plot() {

    const int nSamples = 6000;

    // --- Process degrees_10 file ---
    TFile* file1 = TFile::Open("/shared/storage/physnp/jm2912/degrees_10_adjusted.root", "READ");
    TTree* tree1 = dynamic_cast<TTree*>(file1->Get("adjustedTree"));

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

    // --- Process degrees_30 file ---
    TFile* file2 = TFile::Open("/shared/storage/physnp/jm2912/degrees_30_adjusted.root", "READ");
    TTree* tree2 = dynamic_cast<TTree*>(file2->Get("adjustedTree"));

    int nEntries2 = tree2->GetEntries();
    double pulse2[nSamples];
    tree2->SetBranchAddress("t0aligned_cfd0.10", pulse2);

    double sum2[nSamples] = {0};
    for (int i = 0; i < nEntries2; ++i) {
        tree2->GetEntry(i);
        for (int j = 0; j < nSamples; ++j) {
            sum2[j] += pulse2[j];
        }
    }

    double avg2[nSamples];
    for (int j = 0; j < nSamples; ++j) {
        avg2[j] = sum2[j] / nEntries2;
    }

    // --- Prepare the x-axis values ---
    double xVals[nSamples];
    for (int i = 0; i < nSamples; ++i) {
        xVals[i] = i;
    }

    // --- Create canvas and graphs ---
    TCanvas* c1 = new TCanvas("c1", "Average Waveforms", 1600, 1200);

    // Graph for degrees_30 waveform
    TGraph* g1 = new TGraph(nSamples, xVals, avg1);
    g1->SetLineWidth(4);
    // Optionally set a color (e.g. red for degrees_10)
    g1->SetLineColor(kRed);
    g1->SetTitle(""); // Remove the title

    // Graph for degrees_10 waveform
    TGraph* g2 = new TGraph(nSamples, xVals, avg2);
    g2->SetLineWidth(4);
    // Draw it as a dashed line and with a different color (e.g. blue for degrees_30)
    g2->SetLineColor(kBlue);

    // Draw the first graph and then overlay the second
    g1->Draw("AL");
    // Ensure proper x-axis range
    g1->GetXaxis()->SetLimits(0, nSamples);
    g1->GetHistogram()->SetAxisRange(0, nSamples, "X");
    g2->Draw("L SAME");

    TLine* line = new TLine(4300, g1->GetYaxis()->GetXmin(), 4300, g1->GetYaxis()->GetXmax());
    line->SetLineColor(kBlack);
    line->SetLineWidth(4);
    line->SetLineStyle(2); // Dashed line
    line->Draw();

    TLine* line2 = new TLine(5100, g1->GetYaxis()->GetXmin(), 5100, g1->GetYaxis()->GetXmax());
    line2->SetLineColor(kBlack);
    line2->SetLineWidth(4);
    line2->SetLineStyle(2); // Dashed line
    line2->Draw();

    c1->SaveAs("average_plot.png");

    // Close files
    file1->Close();
    file2->Close();
}