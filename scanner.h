#ifndef SCANNER_H
#define SCANNER_H

#include <string>
#include <cstdint> 
#include <vector>

struct FileInfo {
    std::string name;
    std::string path;
    uintmax_t size;
    long long lastModified;
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

