#include "summary.h"
#include <iostream>
#include <iomanip>

void Summary::printStep(const std::string& stepName,
                        const ScanResult& before,
                        const ScanResult& after,
                        int filesAffected) {
    std::cout << "\n[" << stepName << "]\n";
    std::cout << "Used Before : " << std::fixed << std::setprecision(2) << before.usedSpaceMB << " MB\n";
    std::cout << "Used After  : " << std::fixed << std::setprecision(2) << after.usedSpaceMB << " MB\n";
    std::cout << "Files Processed: " << filesAffected << "\n";
    std::cout << "-----------------------------------\n";
}

void Summary::printFinal(const ScanResult& start,
                         const ScanResult& end,
                         int totalFilesProcessed) {
    std::cout << "\n====== FINAL SUMMARY ======\n";
    std::cout << "Initial Used : " << std::fixed << std::setprecision(2) << start.usedSpaceMB << " MB\n";
    std::cout << "Final Used   : " << end.usedSpaceMB << " MB\n";
    std::cout << "Total Files Processed: " << totalFilesProcessed << "\n";
    std::cout << "===========================\n";
}
