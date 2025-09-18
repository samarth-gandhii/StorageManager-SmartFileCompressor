#include "duplicates.h"
#include <iostream>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <string>
#include <algorithm>
#include <filesystem>

std::string hash(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file) {
        return "";
    }
    
    unsigned long long hash = 0;
    char buffer[4096];
    while (file.read(buffer, sizeof(buffer)) || file.gcount() > 0) {
        std::streamsize bytesRead = file.gcount();
        for (std::streamsize i = 0; i < bytesRead; ++i) {
            hash = hash * 31 + static_cast<unsigned char>(buffer[i]);
        }
    }
    
    char hexStr[17];
    snprintf(hexStr, sizeof(hexStr), "%016llx", hash);
    return std::string(hexStr);
}

std::unordered_map<std::string, std::vector<FileInfo>> findDuplicates(const std::vector<FileInfo>& files) {
    std::unordered_map<std::string, std::vector<FileInfo>> groups;
    
    for (const auto& f : files) {
        std::string fileHash = hash(f.path);
        groups[fileHash].push_back(f);
    }
    
    return groups;
}

void handleDuplicates(std::vector<FileInfo>& files) {
    auto groups = findDuplicates(files);
    
    int groupNum = 1;
    for (const auto& pair : groups) {
        if (pair.second.size() > 1) {
            std::cout << "=== Duplicate group " << groupNum << " ===" << std::endl;
            for (const auto& file : pair.second) {
                std::filesystem::path p(file.path);
                std::cout << "- " << p.filename().string() << " (" << file.path << ")" << std::endl;
            }
            
            std::cout << "Do you want to delete duplicates? (y/n): ";
            char choice;
            std::cin >> choice;
            
            if (choice == 'y' || choice == 'Y') {
                for (size_t i = 1; i < pair.second.size(); ++i) {
                    std::filesystem::remove(pair.second[i].path);
                    std::cout << "Deleted: " << pair.second[i].path << std::endl;
                }
            }
            groupNum++;
        }
    }
}
