#ifndef SCANNER_H
#define SCANNER_H

#include <string>
#include <cstdint> 
#include <vector>

// Struct to hold file metadata
struct FileInfo {
    std::string path;       // Full path
    uintmax_t size;         // File size in bytes
    std::string lastModified; // Last modification time (string formatted)
    std::string type;  
     std::string hash;        // File type (extension)
};

// Scans a directory and returns metadata for all files inside
std::vector<FileInfo> scanDirectory(const std::string& directory);

#endif
