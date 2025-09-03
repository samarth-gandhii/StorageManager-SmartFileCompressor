#include "scanner.h"
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace fs = std::filesystem;

std::string formatTime(std::filesystem::file_time_type ftime) {
    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        ftime - fs::file_time_type::clock::now()
        + std::chrono::system_clock::now()
    );
    std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&cftime), "%F %T");
    return ss.str();
}

std::vector<FileInfo> scanDirectory(const std::string& directory) {
    std::vector<FileInfo> files;

    for (const auto& entry : fs::recursive_directory_iterator(directory)) {
        if (fs::is_regular_file(entry)) {
            FileInfo info;
            info.path = entry.path().string();
            info.size = fs::file_size(entry);
            info.lastModified = formatTime(fs::last_write_time(entry));

            // Extract extension (type)
            info.type = entry.path().extension().string();
            if (info.type.empty()) {
                info.type = "unknown";
            }

            files.push_back(info);
        }
    }

    return files;
}
