#include <string>
#include <fstream>
#include <iostream>
#include "TSystem.h"   // added for gSystem->Load
#include "TString.h"   // added for Form

void gatematrix(){

    gSystem->Load("qdc_c.so");
    gSystem->Load("qratio_c.so");

    // Define the number of steps (gate combinations) for each parameter
    const int numT1 = 51;  // number of t1 values
    const int numT2 = 51;  // number of t2 values

    // Define the range boundaries for t1 and t2
    const int t1_min = 1100;
    const int t1_max = 6100;
    const int t2_min = 1100;
    const int t2_max = 6100;

    std::cout << "Starting gatematrix loop with " << numT1 * numT2 
              << " iterations (expected ~60 minutes total)..." << std::endl;
    for (int i = 0; i < numT1; i++) {
        // Compute t1 value by linear interpolation over the range
        int t1 = t1_min + i * (t1_max - t1_min) / (numT1 - 1);
        for (int j = 0; j < numT2; j++) {
            // Compute t2 value by linear interpolation over the range
            int t2 = t2_min + j * (t2_max - t2_min) / (numT2 - 1);
            if (t1 >= t2) {
                std::ofstream txtOut("output.txt", std::ios::app);
                txtOut << t1 << " " << t2 << " 0" << std::endl;
                continue;  // skip t1 >= t2
            }
            // Run QDC analysis for both root files
            qdc("/shared/storage/physnp/jm2912/degrees_10_adjusted.root", t1, t2);
            qdc("/shared/storage/physnp/jm2912/degrees_30_adjusted.root", t1, t2);

            // Append t1 and t2 parameters to output file
            {
                std::ofstream txtOut("output.txt", std::ios::app);
                txtOut << t1 << " " << t2 << " ";
            }
            // Create branch names based on current t1 and t2 values
            std::string Q1BranchName = Form("Q1_%d_%d_val", t1, t2);
            std::string Q2BranchName = Form("Q2_%d_%d_val", t1, t2);

            // Run the qratio analysis with the generated branch names
            qratio("/shared/storage/physnp/jm2912/degrees_10_adjusted.root",
                   "/shared/storage/physnp/jm2912/degrees_30_adjusted.root",
                   Q1BranchName.c_str(), Q2BranchName.c_str());
        }
    }
    std::cout << "Gatematrix loop complete." << std::endl;
}
