#include "scanner.h"
#include <filesystem>
#include <chrono>
#include <iostream>

namespace fs = std::filesystem;

long long formatTime(fs::file_time_type ftime) {
    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now());
    auto now = std::chrono::system_clock::now();
    auto diff = now - sctp;
    auto days = std::chrono::duration_cast<std::chrono::hours>(diff).count() / 24;
    return days;
}

ScanResult scanDirectory(const std::string& directory) {
    ScanResult result;
    std::error_code ec;
    
    // Reset values to ensure fresh calculation
    result.usedSpace = 0.0;
    result.totalSpace = 0.0;
    result.freeSpace = 0.0;
    result.files.clear();
    
    if (!fs::exists(directory, ec) || ec) {
        std::cerr << "Directory doesn't exist: " << directory << std::endl;
        return result;
    }
    
    // Scan all files
    for (auto it = fs::recursive_directory_iterator(directory, ec); 
         it != fs::recursive_directory_iterator() && !ec; 
         it.increment(ec)) {
        
        if (ec) {
            ec.clear();
            continue;
        }

        const auto& entry = *it;
        if (fs::is_regular_file(entry, ec) && !ec) {
            FileInfo info;
            info.path = entry.path().string();
            info.name = entry.path().filename().string();
            
            info.size = fs::file_size(entry, ec);
            if (ec) {
                info.size = 0;
                ec.clear();
            }
            
            info.lastModified = formatTime(fs::last_write_time(entry, ec));
            if (ec) {
                info.lastModified = 0;
                ec.clear();
            }
            
            info.type = entry.path().extension().string();
            if (info.type.empty()) info.type = "unknown";
            
            result.files.push_back(info);
        }
    }

    // FIXED: Calculate used space properly
    result.usedSpace = 0.0;
    for (const auto& file : result.files) {
        result.usedSpace += file.size / (1024.0 * 1024.0); // Convert bytes to MB
    }

    // Get disk space information
    auto space = fs::space(directory, ec);
    if (!ec) {
        result.totalSpace = space.capacity / (1024.0 * 1024.0);
        result.freeSpace = space.free / (1024.0 * 1024.0);
    }

    return result;
}
