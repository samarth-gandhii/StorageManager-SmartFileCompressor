#include "duplicates.h"
#include <iostream>
#include <vector>
#include <map>
#include <fstream> // <-- add this for std::ifstream
#include <string>
#include <algorithm>
#include <filesystem> // for deleting files

std::string hash(const std::string &filepath)
{

    std::ifstream files(filepath, std::ios::binary);
    if (!files)
    {
        return ""; // or throw an error
    }

    unsigned long long hash = 0;
    char buffer[4096];
    while (files.read(buffer, sizeof(buffer)) || files.gcount() > 0)
    {
        std::streamsize bytesRead = files.gcount();
        for (std::streamsize i = 0; i < bytesRead; ++i)
        {
            hash = hash * 31 + static_cast<unsigned char>(buffer[i]);
        }
    }

    // Convert hash to hex string
    char hexStr[17];
    snprintf(hexStr, sizeof(hexStr), "%016llx", hash);
    return std::string(hexStr);
}

std::unordered_map<std::string, std::vector<FileInfo>>
findDuplicates(const std::vector<FileInfo> &files)
{

    std::unordered_map<std::string, std ::vector<FileInfo>> groups;

    for (const auto &f : files)
    {
        groups[f.hash].push_back(f);
    }
    return groups;
}
