#include "scanner.h"
#include "duplicates.h"
#include "optimizer.h"
#include "utils.h"
#include "summary.h"
#include <iostream>

int main() {
    std::cout << "======= Storage Optimizer =======\n";
    std::cout << "Enter directory to scan: ";
    std::string directory;
    std::cin >> directory;

    // Step 1: Scan
    ScanResult scanResult = scanDirectory(directory);
    std::cout << "Scanned " << scanResult.files.size() << " files\n";
    std::cout << "Disk: Total " << scanResult.totalSpace << " MB | Used " 
              << scanResult.usedSpace << " MB | Free " 
              << scanResult.freeSpace << " MB\n";

    // Step 2: Handle duplicates
    findDuplicates(scanResult.files);
    handleDuplicates(scanResult.files);

    // Step 3: Optimize files (Knapsack ranking, delete/compress)
    optimizer opt(scanResult.totalSpace);
    opt.optimizeFiles(scanResult.files);

    std::cout << "\n=== Process Completed ===\n";
    return 0;
}
