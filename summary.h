#ifndef SUMMARY_H
#define SUMMARY_H

#include <string>
#include "scanner.h"  // So we can use ScanResult

class Summary {
public:
    // Print a summary after each step automatically
    static void printStep(const std::string& stepName,
                          const ScanResult& before,
                          const ScanResult& after,
                          int filesAffected);

    // Optional: overall summary (start vs end of project)
    static void printFinal(const ScanResult& start,
                           const ScanResult& end,
                           int totalFilesProcessed);
};

#endif
