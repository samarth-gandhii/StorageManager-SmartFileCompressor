#ifndef SCANNER_H
#define SCANNER_H

#include <string>
#include <vector>
#include <cstdint>  // Add this for uintmax_t

struct FileInfo {
    std::string name;
    std::string path;
    uintmax_t size;      // This line needs to be here
    long long lastModified;
    std::string type;
    std::string hash;
};

struct ScanResult {
    std::vector<FileInfo> files;
    double totalSpace = 0.0;
    double freeSpace = 0.0;
    double usedSpace = 0.0;
};

ScanResult scanDirectory(const std::string& directory);

#endif
