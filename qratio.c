#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TF1.h"
#include "TPaveText.h"
#include <iostream>
#include <cmath>

void qratio(const char* fileLocation1, const char* fileLocation2, int nBins = 100, double lowRange = 0.8, double highRange = 1.0)
{
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
    
    double Q1, Q2;
    
    // Set branch addresses and fill histogram for file1
    tree1->SetBranchAddress("Q1_val", &Q1);
    tree1->SetBranchAddress("Q2_val", &Q2);
    TH1D* h1 = new TH1D("h1", "", nBins, lowRange, highRange);
    Long64_t nEntries1 = tree1->GetEntries();
    for (Long64_t i = 0; i < nEntries1; i++){
        tree1->GetEntry(i);
        if (Q1 != 0)
            h1->Fill(Q2 / Q1);
    }
    
    // Set branch addresses and fill histogram for file2
    tree2->SetBranchAddress("Q1_val", &Q1);
    tree2->SetBranchAddress("Q2_val", &Q2);
    TH1D* h2 = new TH1D("h2", "", nBins, lowRange, highRange);
    Long64_t nEntries2 = tree2->GetEntries();
    for (Long64_t i = 0; i < nEntries2; i++){
        tree2->GetEntry(i);
        if (Q1 != 0)
            h2->Fill(Q2 / Q1);
    }
    
    // Fit Gaussian to h1 - with better range and background
    double peakBin1 = h1->GetMaximumBin();
    double peakCenter1 = h1->GetXaxis()->GetBinCenter(peakBin1);
    double peakHeight1 = h1->GetBinContent(peakBin1);
    double peakWidth1 = h1->GetRMS()/2;  // Use narrower width estimate
    
    // Define a narrower fit range around the peak
    double fitRangeMin1 = peakCenter1 - 10*peakWidth1;
    double fitRangeMax1 = peakCenter1 + 10*peakWidth1;
    
    // Make sure fit range stays within histogram bounds
    if (fitRangeMin1 < lowRange) fitRangeMin1 = lowRange;
    if (fitRangeMax1 > highRange) fitRangeMax1 = highRange;
    
    // Use a Gaussian + constant background
    TF1* g1 = new TF1("g1", "gaus(0) + pol0(3)", fitRangeMin1, fitRangeMax1);
    g1->SetParameters(peakHeight1, peakCenter1, peakWidth1, h1->GetMinimum());
    // Limit parameters to reasonable values
    g1->SetParLimits(0, 0, 2*peakHeight1);     // amplitude
    g1->SetParLimits(1, fitRangeMin1, fitRangeMax1); // mean
    g1->SetParLimits(2, peakWidth1/10, peakWidth1*5); // sigma
    
    h1->Fit(g1, "R+"); // R = use fit range, + = add to existing functions
    
    // Fit Gaussian to h2 - with better range and background
    double peakBin2 = h2->GetMaximumBin();
    double peakCenter2 = h2->GetXaxis()->GetBinCenter(peakBin2);
    double peakHeight2 = h2->GetBinContent(peakBin2);
    double peakWidth2 = h2->GetRMS()/2;  // Use narrower width estimate
    
    // Define a narrower fit range around the peak
    double fitRangeMin2 = peakCenter2 - 10*peakWidth2;
    double fitRangeMax2 = peakCenter2 + 10*peakWidth2;
    
    // Make sure fit range stays within histogram bounds
    if (fitRangeMin2 < lowRange) fitRangeMin2 = lowRange;
    if (fitRangeMax2 > highRange) fitRangeMax2 = highRange;
    
    // Use a Gaussian + constant background
    TF1* g2 = new TF1("g2", "gaus(0) + pol0(3)", fitRangeMin2, fitRangeMax2);
    g2->SetParameters(peakHeight2, peakCenter2, peakWidth2, h2->GetMinimum());
    // Limit parameters to reasonable values
    g2->SetParLimits(0, 0, 2*peakHeight2);     // amplitude
    g2->SetParLimits(1, fitRangeMin2, fitRangeMax2); // mean
    g2->SetParLimits(2, peakWidth2/10, peakWidth2*5); // sigma
    
    h2->Fit(g2, "R+"); // R = use fit range, + = add to existing functions
    
    // Extract fit parameters - using only the Gaussian part
    double mean1 = g1->GetParameter(1);
    double sigma1 = g1->GetParameter(2);
    double fwhm1 = 2.355 * sigma1;
    
    double mean2 = g2->GetParameter(1);
    double sigma2 = g2->GetParameter(2);
    double fwhm2 = 2.355 * sigma2;
    
    // Print results
    std::cout << "File 1 (10 degrees):" << std::endl;
    std::cout << "  Mean: " << mean1 << std::endl;
    std::cout << "  Sigma: " << sigma1 << std::endl;
    std::cout << "  FWHM: " << fwhm1 << std::endl;
    
    std::cout << "File 2 (30 degrees):" << std::endl;
    std::cout << "  Mean: " << mean2 << std::endl;
    std::cout << "  Sigma: " << sigma2 << std::endl;
    std::cout << "  FWHM: " << fwhm2 << std::endl;
    
    // Plot both histograms
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
    
    // Add text with fit results
    TPaveText* pt = new TPaveText(0.15, 0.65, 0.45, 0.85, "NDC");
    pt->SetFillColor(0);
    pt->SetBorderSize(1);
    pt->SetTextAlign(12);
    
    pt->AddText(Form("10 degrees - Mean: %.4f", mean1));
    pt->AddText(Form("10 degrees - FWHM: %.4f", fwhm1));
    pt->AddText(Form("30 degrees - Mean: %.4f", mean2));
    pt->AddText(Form("30 degrees - FWHM: %.4f", fwhm2));
    pt->Draw();
    
    TLegend* legend = new TLegend(0.7, 0.7, 0.9, 0.9);
    legend->AddEntry(h1, "10 degrees", "l");
    legend->AddEntry(h2, "30 degrees", "l");
    legend->Draw();
    
    c1->SaveAs("Q_ratio.png");
    
    // Clean up
    file1->Close();
    file2->Close();
}
