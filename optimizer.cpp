#include "optimizer.h"
#include <iostream>
#include <algorithm>
#include <filesystem>

namespace fs = std::filesystem;

optimizer::optimizer(double size) : totalSpace(size) {}


void optimizer::optimizeFiles(std::vector<FileInfo> &files)
{
    // Step 1: Rank files using knapsack
    std::vector<FileInfo> ranked = rankFilesKnapsack(files);

    std::cout << "Optimized file ranking (Knapsack):\n";
    for (size_t i = 0; i < ranked.size(); ++i)
    {
        std::cout << i + 1 << ". " << ranked[i].name
                  << " (" << ranked[i].size / (1024.0 * 1024.0) << " MB)\n";
    }

    // Step 2: User choice
    int choice;
    std::cout << "\nEnter the rank of the file you want to proceed with (0 to exit): ";
    std::cin >> choice;

    if (choice <= 0 || choice > (int)ranked.size())
    {
        std::cout << "Exiting...\n";
        return;
    }

    FileInfo &selected = ranked[choice - 1];

    // Step 3: If text file, show both delete + compress
    if (shouldCompress(selected))
    {
        std::cout << "File: " << selected.name << " is a text file.\n";
        std::cout << "Options:\n1. Delete\n2. Compress\nChoice: ";
        int action;
        std::cin >> action;
        if (action == 1)
        {
            deleteFile(selected);
        }
        else if (action == 2)
        {
            compressFile(selected);
        }
    }
    // Step 4: If not text file, only show delete
    else
    {
        std::cout << "File: " << selected.name << " is NOT a text file.\n";
        std::cout << "Options:\n1. Delete\nChoice: ";
        int action;
        std::cin >> action;
        if (action == 1)
        {
            deleteFile(selected);
        }
    }
}

// Checks if file can be compressed (text-based extensions)
bool optimizer::shouldCompress(const FileInfo &file)
{
    std::string ext = fs::path(file.name).extension().string();
    return (ext == ".txt" || ext == ".log" || ext == ".csv");
}

// Deletes file from system
void optimizer::deleteFile(FileInfo &file)
{
    try
    {
        fs::remove(file.path);
        std::cout << "Deleted: " << file.name << "\n";
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error deleting " << file.name << ": " << e.what() << "\n";
    }
}

// Compresses file (stub, hook for Huffman compression)
void optimizer::compressFile(FileInfo &file)
{
    std::cout << "Compressing: " << file.name << " ...\n";
    // TODO: Call Huffman compression here
}


std::vector<FileInfo> optimizer::rankFilesKnapsack(const std::vector<FileInfo> &files)
{
    int n = files.size();
    // Assuming totalSpace is a class member of optimizer and correctly defined.
    long long W = totalSpace;

    // DP table: dp[i][w] = max value with first i items and weight w.
    std::vector<std::vector<long long>> dp(n + 1, std::vector<long long>(W + 1, 0));

    for (int i = 1; i <= n; i++) {
        long long days = files[i - 1].lastModified;
        long long val = files[i - 1].size * days; // Value = size * days since last modified
        long long wt = files[i - 1].size;

        for (long long w = 0; w <= W; w++) {
            if (wt <= w) {
                dp[i][w] = std::max(dp[i - 1][w], dp[i - 1][w - wt] + val);
            } else {
                dp[i][w] = dp[i - 1][w];
            }
        }
    }

    // Reconstruct chosen and remaining files
    long long w = W;
    std::vector<FileInfo> chosenFiles;
    std::vector<FileInfo> remainingFiles; // This will hold the files not chosen
    std::vector<bool> isChosen(n, false);

    for (int i = n; i > 0 && w >= 0; i--) {
        long long wt = files[i - 1].size;
        if (dp[i][w] != dp[i - 1][w]) {
            chosenFiles.push_back(files[i - 1]);
            w -= wt;
            isChosen[i - 1] = true;
        }
    }

    // Identify the files not chosen by the knapsack algorithm
    for (int i = 0; i < n; ++i) {
        if (!isChosen[i]) {
            remainingFiles.push_back(files[i]);
        }
    }

    // Sort chosen by size (largest first for presentation)
    std::sort(chosenFiles.begin(), chosenFiles.end(),
              [](const FileInfo &a, const FileInfo &b) {
                  return a.size > b.size;
              });

    // Sort remaining files by size (largest first for presentation)
    std::sort(remainingFiles.begin(), remainingFiles.end(),
              [](const FileInfo &a, const FileInfo &b) {
                  return a.size > b.size;
              });

     for (int i = 0; i < n; ++i) {
        
            remainingFiles.push_back(chosenFiles[i]);
        
    }

    return chosenFiles;
}


// std::vector<FileInfo> optimizer::rankFilesKnapsack(const std::vector<FileInfo> &files)
// {
//     int n = files.size();
//     int W = static_cast<int>(totalSpace);

//     std::vector<std::vector<double>> dp(n + 1, std::vector<double>(W + 1, 0));

//     // For file "value", we can assume "size saved" = file.size (greedy)
//     // Or later, weight = size, value = compression gain
//     for (int i = 1; i <= n; i++)
//     {
//         int wt = static_cast<int>(files[i - 1].size);
//         long long val = files[i - 1].lastModified; // value = size saved

//         for (int w = 0; w <= W; w++)
//         {
//             if (wt <= w)
//                 dp[i][w] = std::max(dp[i - 1][w],
//                                     dp[i - 1][w - wt] + val);
//             else
//                 dp[i][w] = dp[i - 1][w];
//         }
//     }

//     // Reconstruct chosen files
//     int w = W;
//     std::vector<FileInfo> chosen;
//     for (int i = n; i > 0 && w >= 0; i--)
//     {
//         if (dp[i][w] != dp[i - 1][w])
//         {
//             chosen.push_back(files[i - 1]);
//             w -= static_cast<int>(files[i - 1].size);
//         }
//     }

//     // Sort chosen by size (largest first for presentation)
//     std::sort(chosen.begin(), chosen.end(),
//               [](const FileInfo &a, const FileInfo &b)
//               {
//                   return a.size > b.size;
//               });

//     return chosen;
// }

// Main optimizer