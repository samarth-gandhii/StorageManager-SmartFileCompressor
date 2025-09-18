#include "optimizer.h"
#include <iostream>
#include <algorithm>
#include <filesystem>

namespace fs = std::filesystem;

optimizer::optimizer(double size) : totalSpace(size) {}

void optimizer::optimizeFiles(std::vector<FileInfo>& files) {
    std::vector<FileInfo> ranked = rankFilesKnapsack(files);
    
    if (ranked.empty()) {
        std::cout << "No files available for optimization." << std::endl;
        return;
    }
    
    std::cout << "\n=== Optimized file ranking (Knapsack) ===" << std::endl;
    for (size_t i = 0; i < ranked.size(); ++i) {
        std::cout << i + 1 << ". " << ranked[i].name 
                  << " (" << ranked[i].size / (1024.0 * 1024.0) << " MB)" << std::endl;
    }
    
    int choice;
    while (true) {
        std::cout << "\nEnter the rank of the file you want to proceed with (1-" 
                  << ranked.size() << ", 0 to exit): ";
        std::cin >> choice;
        
        if (choice == 0) {
            std::cout << "Exiting..." << std::endl;
            return;
        }
        
        if (choice < 1 || choice > static_cast<int>(ranked.size())) {
            std::cout << "Invalid choice. Please enter a number between 1 and " 
                      << ranked.size() << std::endl;
            continue;
        }
        
        break;
    }
    
    FileInfo selected = ranked[choice - 1];
    
    if (shouldCompress(selected)) {
        std::cout << "File " << selected.name << " is a text file." << std::endl;
        std::cout << "Options:\n1. Delete\n2. Compress" << std::endl;
        int action;
        std::cin >> action;
        
        if (action == 1) {
            deleteFile(selected);
        } else if (action == 2) {
            compressFile(selected);
        } else {
            std::cout << "Invalid option." << std::endl;
        }
    } else {
        std::cout << "File " << selected.name << " is NOT a text file." << std::endl;
        std::cout << "Options:\n1. Delete" << std::endl;
        int action;
        std::cin >> action;
        
        if (action == 1) {
            deleteFile(selected);
        } else {
            std::cout << "Invalid option." << std::endl;
        }
    }
}

bool optimizer::shouldCompress(const FileInfo& file) {
    std::string ext = fs::path(file.name).extension().string();
    return (ext == ".txt" || ext == ".log" || ext == ".csv" || ext == ".cpp" || ext == ".h");
}

void optimizer::deleteFile(FileInfo& file) {
    try {
        fs::remove(file.path);
        std::cout << "Deleted: " << file.name << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error deleting " << file.name << ": " << e.what() << std::endl;
    }
}

void optimizer::compressFile(FileInfo& file) {
    std::cout << "Compressing " << file.name << "..." << std::endl;
    // This is handled in main.cpp case 4 now
}

// FIXED RANKING FUNCTION
std::vector<FileInfo> optimizer::rankFilesKnapsack(const std::vector<FileInfo>& files) {
    int n = files.size();
    std::cout << "\nDEBUG: Ranking " << n << " files" << std::endl;
    std::cout << "Total Space: " << totalSpace << " MB" << std::endl;
    
    if (n == 0) return {};
    
    // Convert totalSpace from MB to KB for reasonable capacity
    int W = static_cast<int>(totalSpace * 1024); // MB to KB
    if (W > 100000) W = 100000; // Cap at 100GB in KB to prevent memory issues
    
    std::vector<std::vector<double>> dp(n + 1, std::vector<double>(W + 1, 0));
    
    // Show first few files for debugging
    for (size_t i = 0; i < std::min((size_t)3, files.size()); ++i) {
        std::cout << "File " << i << ": " << files[i].name 
                  << " Size: " << files[i].size / (1024.0 * 1024.0) << " MB"
                  << " Age: " << files[i].lastModified << " days" << std::endl;
    }
    
    // Improved value and weight calculation
    for (int i = 1; i <= n; ++i) {
        // Convert file size to KB for consistent units
        int wt = static_cast<int>(files[i - 1].size / 1024); // Size in KB
        if (wt == 0) wt = 1; // Minimum weight of 1KB
        
        // Better value function: combine multiple factors
        double sizeScore = files[i - 1].size / (1024.0 * 1024.0); // Size in MB
        double ageScore = (files[i - 1].lastModified + 1) * 0.1;  // Age factor (newer = lower score)
        
        // Type-based scoring
        double typeScore = 1.0;
        if (files[i - 1].type == ".tmp" || files[i - 1].type == ".log") {
            typeScore = 3.0; // Higher priority for temp/log files
        } else if (files[i - 1].type == ".bak" || files[i - 1].type == ".old") {
            typeScore = 2.5; // High priority for backup files
        } else if (files[i - 1].type == ".cache") {
            typeScore = 2.0; // Medium priority for cache files
        }
        
        double val = sizeScore * typeScore + ageScore; // Combined value
        
        for (int w = 0; w <= W; ++w) {
            if (wt <= w) {
                dp[i][w] = std::max(dp[i - 1][w], dp[i - 1][w - wt] + val);
            } else {
                dp[i][w] = dp[i - 1][w];
            }
        }
    }
    
    // Reconstruct solution (which files were selected)
    int w = W;
    std::vector<FileInfo> chosen;
    std::vector<bool> selected(n, false);
    
    for (int i = n; i > 0 && w > 0; i--) {
        if (dp[i][w] != dp[i - 1][w]) {
            chosen.push_back(files[i - 1]);
            selected[i - 1] = true;
            w -= static_cast<int>(files[i - 1].size / 1024);
        }
    }
    
    // Reverse to show in selection order (preserve knapsack ranking)
    std::reverse(chosen.begin(), chosen.end());
    
    std::cout << "Knapsack selected " << chosen.size() << " files for optimization" << std::endl;
    
    return chosen;
}
