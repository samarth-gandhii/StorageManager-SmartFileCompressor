#include "summary.h"
#include "utils.h"
#include <iostream>
#include <iomanip>

void Summary::printStep(const std::string& stepName, const ScanResult& before, 
                       const ScanResult& after, int filesProcessed) {
    std::cout << "\n=== " << stepName << " Summary ===\n";
    std::cout << "Files processed: " << filesProcessed << "\n";
    std::cout << "Before: " << before.files.size() << " files, " 
              << std::fixed << std::setprecision(2) << before.usedSpace << " MB\n";
    std::cout << "After:  " << after.files.size() << " files, " 
              << std::fixed << std::setprecision(2) << after.usedSpace << " MB\n";
    std::cout << "Space saved: " << (before.usedSpace - after.usedSpace) << " MB\n";
}

void Summary::printFinal(const ScanResult& initial, const ScanResult& final, 
                        int totalFilesProcessed) {
    std::cout << "\n[COMPLETE SUMMARY REPORT]\n";
    std::cout << "=========================================\n";
    std::cout << "======= FINAL SUMMARY =======\n";
    std::cout << "Initial Used : " << std::fixed << std::setprecision(2) 
              << initial.usedSpace << " MB\n";
    std::cout << "Final Used   : " << std::fixed << std::setprecision(2) 
              << final.usedSpace << " MB\n";
    std::cout << "Total Files Processed: " << totalFilesProcessed << "\n";
    std::cout << "============================\n";
}
