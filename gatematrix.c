#include <string>
#include <fstream>
#include <iostream>

void gatematrix(){

    gSystem->Load("qdc_c.so");
    gSystem->Load("qratio_c.so");

    for (int t1 = 1100; t1 <= 3100 ; t1+= 600){
        for (int t2 = 5500; t2 <= 6100; t2+= 600){
            qdc("/shared/storage/physnp/jm2912/degrees_10_adjusted.root", t1, t2);
            qdc("/shared/storage/physnp/jm2912/degrees_30_adjusted.root", t1, t2);

            std::ofstream txtOut;
            txtOut.open("output.txt", std::ios::app);
            txtOut << t1 << " " << t2 << " ";
            txtOut.close();

            std::string Q1BranchName = Form("Q1_%d_%d_val", t1, t2);
            std::string Q2BranchName = Form("Q2_%d_%d_val", t1, t2);
            qratio("/shared/storage/physnp/jm2912/degrees_10_adjusted.root", "/shared/storage/physnp/jm2912/degrees_30_adjusted.root", Q1BranchName.c_str(), Q2BranchName.c_str());
        }
    }

}
