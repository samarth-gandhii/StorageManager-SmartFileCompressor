#include "summary.h"
#include <iostream>
#include <iomanip>

void Summary::printStep(const std::string& stepName,
                        const ScanResult& before,
                        const ScanResult& after,
                        int filesAffected) {
    std::cout << "\n[" << stepName << "]\n";
    std::cout << "Used Before: " << std::fixed << std::setprecision(2) << before.usedSpace / (1024.0 * 1024.0) << " MB" << std::endl;
    std::cout << "Used After: " << std::fixed << std::setprecision(2) << after.usedSpace / (1024.0 * 1024.0) << " MB" << std::endl;
    std::cout << "Files Processed: " << filesAffected << std::endl;
    std::cout << "-----------------------------------" << std::endl;
}

void Summary::printFinal(const ScanResult& start,
                         const ScanResult& end,
                         int totalFilesProcessed) {
    std::cout << "\n====== FINAL SUMMARY ======\n";
    std::cout << "Initial Used : " << std::fixed << std::setprecision(2) << start.usedSpace / (1024.0 * 1024.0) << " MB\n";
    std::cout << "Final Used   : " << std::fixed << std::setprecision(2) << end.usedSpace / (1024.0 * 1024.0) << " MB\n";
    std::cout << "Total Files Processed: " << totalFilesProcessed << "\n";
    std::cout << "===========================\n";
}
