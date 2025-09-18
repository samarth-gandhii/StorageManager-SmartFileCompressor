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
findDuplicates(std::vector<FileInfo> &files)
{

    std::unordered_map<std::string, std ::vector<FileInfo>> groups;

    for (auto &f : files) {
    f.hash = hash(f.path);
}

    for (const auto &f : files)
    {
        groups[f.hash].push_back(f);
    }
    return groups;
}
void handleDuplicates(std::vector<FileInfo> &files) {
    auto groups = findDuplicates(files);

    // Show duplicates
    int groupNum = 1;
    for (const auto &pair : groups) {
        if (pair.second.size() > 1) {
            std::cout << "Duplicate group " << groupNum++ << ":\n";
            for (const auto &file : pair.second) {
                std::filesystem::path p(file.path);
                std::cout << "   " << p.filename().string()   // just the file name
                          << " (" << file.path << ")\n";   // full path in brackets
            }
        }
    }

    std::cout << "\nDo you want to delete duplicates? (y/n): ";
    char choice;
    std::cin >> choice;

    if (choice == 'y' || choice == 'Y') {
        for (const auto &pair : groups) {
            if (pair.second.size() > 1) {
                // Keep the first file, delete the rest
                for (size_t i = 1; i < pair.second.size(); ++i) {
                    std::filesystem::remove(pair.second[i].path);
                    std::cout << "Deleted: " << pair.second[i].path << "\n";
                }
            }
        }
    }
}


// void handleDuplicates(std::vector<FileInfo> &files) {
//     auto groups = findDuplicates(files);

//     // Show duplicates
//     int groupNum = 1;
//     for (const auto &pair : groups) {
//         if (pair.second.size() > 1) {
//             std::cout << "Duplicate group " << groupNum++ << ":\n";
//             for (const auto &file : pair.second) {
//                 std::cout << "   " << file.name().string()
//                 << "   " << file.path << "\n";
//             }
//         }
//     }

//     std::cout << "\nDo you want to delete duplicates? (y/n): ";
//     char choice;
//     std::cin >> choice;

//     if (choice == 'y' || choice == 'Y') {
//         for (const auto &pair : groups) {
//             if (pair.second.size() > 1) {
//                 // Keep the first file, delete the rest
//                 for (size_t i = 1; i < pair.second.size(); ++i) {
//                     std::filesystem::remove(pair.second[i].path);
//                     std::cout << "Deleted: " << pair.second[i].path << "\n";
//                 }
//             }
//         }
//     }
// }
