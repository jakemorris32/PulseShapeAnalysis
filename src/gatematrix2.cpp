#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <cstdio>    // For snprintf

// ROOT-specific includes (if needed)
#include "TSystem.h"  // for gSystem->Load()

// Declarations for external functions provided in qdc_c.so and qratio_c.so
extern void qdc(const char* filename, int t1, int t2);
extern void qratio(const char* file1, const char* file2, const char* Q1BranchName, const char* Q2BranchName, bool plot = false, int nBins = 500, double lowRange = -1, double highRange = -1);

// Mutexes for protecting shared resources
std::mutex output_mutex;
std::mutex job_mutex;

// Worker function for processing one (t1, t2) gate combination
void process_gate(int t1, int t2,
                  const std::string &file1,
                  const std::string &file2,
                  std::ofstream &txtOut)
{
    // If t1 is not less than t2, write a default output and return.
    if (t1 >= t2) {
        std::lock_guard<std::mutex> guard(output_mutex);
        txtOut << t1 << " " << t2 << " 0" << std::endl;
        return;
    }

    // Run qdc analysis on both ROOT files.
    qdc(file1.c_str(), t1, t2);
    qdc(file2.c_str(), t1, t2);

    {
        // Write t1 and t2 to output in a thread-safe manner.
        std::lock_guard<std::mutex> guard(output_mutex);
        txtOut << t1 << " " << t2 << " ";
    }

    // Create branch names using thread-safe snprintf instead of ROOT's Form().
    char Q1BranchName[128];
    char Q2BranchName[128];
    std::snprintf(Q1BranchName, sizeof(Q1BranchName), "Q1_%d_%d_val", t1, t2);
    std::snprintf(Q2BranchName, sizeof(Q2BranchName), "Q2_%d_%d_val", t1, t2);

    // Run qratio analysis with the generated branch names.
    qratio(file1.c_str(), file2.c_str(), Q1BranchName, Q2BranchName);

    {
        // Finish the output line (if additional results are appended, modify accordingly).
        std::lock_guard<std::mutex> guard(output_mutex);
        txtOut << std::endl;
    }
}

int main() {
    // Load the necessary shared libraries.
    gSystem->Load("qdc_c.so");
    gSystem->Load("qratio_c.so");

    // Define grid parameters.
    const int numT1 = 51;
    const int numT2 = 51;
    const int t1_min = 1100;
    const int t1_max = 6100;
    const int t2_min = 1100;
    const int t2_max = 6100;

    std::cout << "Starting gatematrix loop with " << numT1 * numT2 
              << " iterations (expected ~60 minutes total)..." << std::endl;

    // Open the output file once in append mode.
    std::ofstream txtOut("output.txt", std::ios::app);
    if (!txtOut.is_open()) {
        std::cerr << "Failed to open output.txt" << std::endl;
        return 1;
    }

    // Precompute step sizes to avoid repeated calculations.
    int stepT1 = (t1_max - t1_min) / (numT1 - 1);
    int stepT2 = (t2_max - t2_min) / (numT2 - 1);

    // Prepare a list of all jobs (each job is a (t1,t2) pair).
    std::vector<std::pair<int, int>> jobs;
    for (int i = 0; i < numT1; i++) {
        int t1 = t1_min + i * stepT1;
        for (int j = 0; j < numT2; j++) {
            int t2 = t2_min + j * stepT2;
            jobs.emplace_back(t1, t2);
        }
    }

    // Determine the maximum number of threads to use.
    unsigned int max_threads = std::thread::hardware_concurrency();
    if (max_threads == 0)
        max_threads = 4;  // Fallback if hardware_concurrency() is undefined.

    size_t job_index = 0;
    // Worker lambda: each thread takes a job from the list until none remain.
    auto worker = [&]() {
        while (true) {
            int current_t1, current_t2;
            {
                std::lock_guard<std::mutex> lock(job_mutex);
                if (job_index >= jobs.size())
                    break;
                current_t1 = jobs[job_index].first;
                current_t2 = jobs[job_index].second;
                ++job_index;
            }
            process_gate(current_t1, current_t2,
                         "/shared/storage/physnp/jm2912/degrees_10_adjusted.root",
                         "/shared/storage/physnp/jm2912/degrees_30_adjusted.root",
                         txtOut);
        }
    };

    // Launch worker threads.
    std::vector<std::thread> threads;
    for (unsigned int i = 0; i < max_threads; i++) {
        threads.emplace_back(worker);
    }

    // Wait for all threads to complete.
    for (auto &t : threads) {
        t.join();
    }

    txtOut.close();
    std::cout << "Gatematrix loop complete." << std::endl;
    return 0;
}