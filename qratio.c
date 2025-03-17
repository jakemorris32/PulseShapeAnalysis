#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"
#include "TF1.h"
#include <iostream>
#include <fstream>
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
    
    // Fit Gaussians to calculate parameters
    TF1* g1 = new TF1("g1", "gaus", lowRange, highRange);
    g1->SetParameters(h1->GetMaximum(), h1->GetMean(), h1->GetRMS());
    h1->Fit(g1, "Q"); // Q = quiet mode
    
    TF1* g2 = new TF1("g2", "gaus", lowRange, highRange);
    g2->SetParameters(h2->GetMaximum(), h2->GetMean(), h2->GetRMS());
    h2->Fit(g2, "Q"); // Q = quiet mode
    
    // Extract fit parameters
    double mean1 = g1->GetParameter(1);
    double sigma1 = g1->GetParameter(2);
    double fwhm1 = 2.355 * sigma1;
    
    double mean2 = g2->GetParameter(1);
    double sigma2 = g2->GetParameter(2);
    double fwhm2 = 2.355 * sigma2;
   
    // Write results to text file
    std::ofstream txtOut;
    txtOut.open("output.txt", std::ios::app);
    txtOut << mean1 << " " << mean2 << " " << fwhm1 << " " << fwhm2 << std::endl;
    txtOut.close();
    
    // Clean up
    delete h1;
    delete h2;
    delete g1;
    delete g2;
    file1->Close();
    file2->Close();
}
