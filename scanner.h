#ifndef SCANNER_H
#define SCANNER_H

#include <string>
#include <cstdint> 
#include <vector>

struct FileInfo {
    std::string path;
    uintmax_t size;
    std::string lastModified;
    std::string type;
    std::string hash;
};

struct ScanResult {
    std::vector<FileInfo> files;
    double totalSpace;
    double freeSpace;
    double usedSpace;
};

// Updated return type
ScanResult scanDirectory(const std::string& directory);

#endif

