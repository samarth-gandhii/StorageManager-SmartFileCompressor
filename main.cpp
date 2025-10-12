#include "scanner.h"
#include "duplicates.h"
#include "optimizer.h"
#include "utils.h"
#include "summary.h"
#include "huffman.h"
#include "utils.h"
#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <map>
#include <filesystem>

void displayHeader()
{
    std::cout << "===================================\n";
    std::cout << "    SMART STORAGE MANAGER\n";
    std::cout << "  File Optimization & Management\n";
    std::cout << "===================================\n";
}

void displayMenu()
{
    std::cout << "\nMENU:\n";
    std::cout << "1. Scan Directory\n";
    std::cout << "2. Find & Handle Duplicates\n";
    std::cout << "3. Optimize Files (Ranking System)\n";
    std::cout << "4. Compress a File (Huffman)\n";
    std::cout << "5. View Summary Report\n";
    std::cout << "6. Exit Program\n";
    std::cout << "-----------------------------------\n";
}

void displayScanResults(const ScanResult &result)
{
    std::cout << "\nSCAN RESULTS:\n";
    std::cout << "Files Found: " << result.files.size() << "\n";
    std::cout << "Total Space: " << std::fixed << std::setprecision(1)
              << result.totalSpace << " MB\n";
    std::cout << "Used Space:  " << std::fixed << std::setprecision(1)
              << result.usedSpace << " MB\n";
    std::cout << "Free Space:  " << std::fixed << std::setprecision(1)
              << result.freeSpace << " MB\n";
}

void displayFileList(const std::vector<FileInfo> &files)
{
    std::cout << "\nAvailable Files:\n";
    for (size_t i = 0; i < files.size(); ++i)
    {
        std::cout << (i + 1) << ". " << files[i].name
                  << " (" << formatSizeMB(files[i].size) << " MB)\n";
    }
}

void waitForInput()
{
    std::cout << "\nPress Enter to continue...";
    std::cin.ignore();
    std::cin.get();
}

int main()
{
    displayHeader();

    // State variables
    ScanResult initialScan, currentState;
    std::vector<FileInfo> files;
    optimizer *opt = nullptr;

    bool isScanned = false;
    bool duplicatesHandled = false;
    bool isOptimized = false;
    int totalFilesProcessed = 0;

    int choice;

    while (true)
    {
        displayMenu();
        std::cout << "Enter your choice (1-6): ";
        std::cin >> choice;
        std::cin.ignore();

        switch (choice)
        {
        case 1:
        {
            std::cout << "\n[DIRECTORY SCANNER]\n";
            std::cout << "Enter directory path to scan: ";
            std::string directory;
            std::getline(std::cin, directory);

            std::cout << "Scanning directory...\n";

            initialScan = scanDirectory(directory);
            currentState = initialScan;
            files = initialScan.files;

            if (files.empty())
            {
                std::cout << "ERROR: No files found or directory inaccessible!\n";
                waitForInput();
                break;
            }

            if (opt != nullptr)
                delete opt;
            opt = new optimizer(initialScan.totalSpace);

            displayScanResults(initialScan);

            std::cout << "\nFile Types Found:\n";
            std::map<std::string, int> typeCount;
            for (const auto &file : files)
            {
                typeCount[file.type]++;
            }

            for (const auto &pair : typeCount)
            {
                std::cout << "  " << pair.first << ": " << pair.second << " files\n";
            }

            isScanned = true;
            duplicatesHandled = isOptimized = false;
            totalFilesProcessed = 0;

            std::cout << "Directory scan completed successfully.\n";
            waitForInput();
            break;
        }

        case 2:
        {
            if (!isScanned)
            {
                std::cout << "ERROR: Please scan a directory first (Option 1).\n";
                waitForInput();
                break;
            }

            std::cout << "\n[DUPLICATE FINDER]\n";
            std::cout << "Analyzing files for duplicates...\n";

            ScanResult beforeDuplicates = currentState;

            handleDuplicates(files);

            currentState.files = files;
            currentState.usedSpace = 0;
            for (const auto &file : files)
            {
                currentState.usedSpace += file.size / (1024.0 * 1024.0);
            }
            currentState.freeSpace = currentState.totalSpace - currentState.usedSpace;

            int filesRemoved = beforeDuplicates.files.size() - files.size();
            totalFilesProcessed += filesRemoved;

            if (filesRemoved > 0)
            {
                Summary::printStep("Duplicate Removal", beforeDuplicates, currentState, filesRemoved);
            }
            else
            {
                std::cout << "No duplicates found.\n";
            }

            duplicatesHandled = true;
            std::cout << "Duplicate analysis completed.\n";
            waitForInput();
            break;
        }

        case 3:
        {
            if (!duplicatesHandled)
            {
                std::cout << "ERROR: Please handle duplicates first (Option 2).\n";
                waitForInput();
                break;
            }

            if (files.empty())
            {
                std::cout << "ERROR: No files available for optimization.\n";
                waitForInput();
                break;
            }

            std::cout << "\n[FILE OPTIMIZER]\n";
            std::cout << "Using Advanced Knapsack Algorithm for Ranking\n";

            ScanResult beforeOptimization = currentState;

            opt->optimizeFiles(files);

            currentState.files = files;
            currentState.usedSpace = 0;
            for (const auto &file : files)
            {
                currentState.usedSpace += file.size / (1024.0 * 1024.0);
            }
            currentState.freeSpace = currentState.totalSpace - currentState.usedSpace;

            int filesOptimized = beforeOptimization.files.size() - files.size();
            totalFilesProcessed += filesOptimized;

            if (filesOptimized > 0)
            {
                Summary::printStep("File Optimization", beforeOptimization, currentState, filesOptimized);
            }

            isOptimized = true;
            std::cout << "File optimization completed.\n";
            waitForInput();
            break;
        }

        case 4:
        {
            if (!isScanned)
            {
                std::cout << "ERROR: Please scan a directory first (Option 1).\n";
                waitForInput();
                break;
            }

            if (files.empty())
            {
                std::cout << "ERROR: No files available for compression.\n";
                waitForInput();
                break;
            }

            std::cout << "\n[HUFFMAN COMPRESSOR]\n";
            displayFileList(files);

            std::cout << "\nEnter the number of the file to compress: ";
            int fileNum;
            std::cin >> fileNum;
            std::cin.ignore();

            if (fileNum < 1 || fileNum > static_cast<int>(files.size()))
            {
                std::cout << "ERROR: Invalid file number.\n";
                waitForInput();
                break;
            }

            FileInfo &selectedFile = files[fileNum - 1];
            std::string inputPath = selectedFile.path;
            std::string outputPath = inputPath + ".huff";

            std::cout << "Compressing: " << selectedFile.name << "\n";
            std::cout << "Original size: " << formatSizeMB(selectedFile.size) << " MB\n";

            try
            {
                compressFile(inputPath, outputPath);
                
                if (std::filesystem::exists(outputPath))
                {
                    uintmax_t compressedSize = std::filesystem::file_size(outputPath);
                    double compressionRatio = ((double)(selectedFile.size - compressedSize) / selectedFile.size) * 100;

                    std::cout << "SUCCESS: Compression completed!\n";
                    std::cout << "Compressed size: " << formatSizeMB(compressedSize) << " MB\n";
                    std::cout << "Compression ratio: " << std::fixed << std::setprecision(1)
                              << compressionRatio << "%\n";
                    std::cout << "Compressed file: " << outputPath << "\n";

                    std::cout << "\nDelete original file? (y/n): ";
                    char deleteChoice;
                    std::cin >> deleteChoice;
                    std::cin.ignore();

                    if (deleteChoice == 'y' || deleteChoice == 'Y')
                    {
                        std::filesystem::remove(inputPath);
                        std::cout << "Original file deleted.\n";

                        selectedFile.path = outputPath;
                        selectedFile.name = selectedFile.name + ".huff";
                        selectedFile.size = compressedSize;
                    }
                }
                else
                {
                    std::cout << "ERROR: Compressed file not created.\n";
                }
            }
            catch (const std::exception &e)
            {
                std::cout << "ERROR during compression: " << e.what() << "\n";
            }

            waitForInput();
            break;
        }

        case 5:
        {
            if (!isOptimized)
            {
                std::cout << "ERROR: Please complete optimization first (Option 3).\n";
                waitForInput();
                break;
            }

            std::cout << "\n[COMPLETE SUMMARY REPORT]\n";
            std::cout << "=========================================\n";

            double spaceSaved = initialScan.usedSpace - currentState.usedSpace;
            double percentageSaved = (spaceSaved / initialScan.usedSpace) * 100;

            Summary::printFinal(initialScan, currentState, totalFilesProcessed);

            std::cout << "\nOPTIMIZATION STATISTICS:\n";
            std::cout << "Space Saved: " << std::fixed << std::setprecision(2)
                      << spaceSaved << " MB (" << percentageSaved << "%)\n";
            std::cout << "Files Processed: " << totalFilesProcessed << " files\n";
            std::cout << "Files Remaining: " << files.size() << " files\n";

            if (spaceSaved > 100)
            {
                std::cout << "Result: Excellent space optimization achieved!\n";
            }
            else if (spaceSaved > 10)
            {
                std::cout << "Result: Good optimization results.\n";
            }
            else
            {
                std::cout << "Result: Storage was already well optimized.\n";
            }

            waitForInput();
            break;
        }

        case 6:
        {
            std::cout << "\nThank you for using Smart Storage Manager!\n";
            if (opt != nullptr)
                delete opt;
            return 0;
        }

        default:
        {
            std::cout << "ERROR: Invalid choice. Please enter 1-6.\n";
            waitForInput();
            break;
        }
        }
    }

    if (opt != nullptr)
        delete opt;
    return 0;
}
