#include "scanner.h"
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace fs = std::filesystem;

std::string formatTime(fs::file_time_type ftime) {
    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        ftime - fs::file_time_type::clock::now()
        + std::chrono::system_clock::now()
    );
    std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&cftime), "%F %T");
    return ss.str();
}

// Updated to return ScanResult
ScanResult scanDirectory(const std::string& directory) {
    ScanResult result;  // Contains files + disk info

    for (const auto& entry : fs::recursive_directory_iterator(directory)) {
        if (fs::is_regular_file(entry)) {
            FileInfo info;
            info.path = entry.path().string();
            info.size = fs::file_size(entry);
            info.lastModified = formatTime(fs::last_write_time(entry));

            // Extract extension (type)
            info.type = entry.path().extension().string();
            if (info.type.empty()) info.type = "unknown";

            result.files.push_back(info);
        }
    }

    // Get disk info
    auto space = fs::space(directory);
  result.totalSpace = space.capacity / (1024.0 * 1024.0);
result.freeSpace  = space.free / (1024.0 * 1024.0);
result.usedSpace  = (space.capacity - space.available) / (1024.0 * 1024.0);

    return result;  // ✅ Return ScanResult, not files
}

