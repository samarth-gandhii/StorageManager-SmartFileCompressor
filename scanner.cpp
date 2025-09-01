#include "scanner.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <thread>
#include <mutex>
#include <atomic>

#ifdef _WIN32
#include <windows.h>
#include <fileapi.h>
#elif defined(__linux__)
#include <sys/stat.h>
#include <sys/types.h>
#endif

// FileMetadata Implementation
FileMetadata::FileMetadata(const fs::path& path) : filepath(path) {
    try {
        if (fs::exists(path)) {
            size = fs::is_regular_file(path) ? fs::file_size(path) : 0;
            last_write_time = fs::last_write_time(path);
            extension = path.extension().string();
            file_type = fs::status(path).type();
            permissions = fs::status(path).permissions();
            is_hidden = FileScanner::is_hidden_file(path);
            is_symlink = fs::is_symlink(path);
            mime_type = FileScanner::detect_mime_type(path);
        }
    } catch (const fs::filesystem_error&) {
        // Initialize with default values on error
        size = 0;
        file_type = fs::file_type::unknown;
        permissions = fs::perms::unknown;
        is_hidden = false;
        is_symlink = false;
    }
}

bool FileMetadata::is_regular_file() const noexcept {
    return file_type == fs::file_type::regular;
}

bool FileMetadata::is_directory() const noexcept {
    return file_type == fs::file_type::directory;
}

std::string FileMetadata::get_size_string() const {
    return FileScanner::format_file_size(size);
}

std::string FileMetadata::get_last_modified_string() const {
    auto time_t = std::chrono::file_clock::to_time_t(last_write_time);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

// ScanStatistics Implementation
void ScanStatistics::print_summary() const {
    std::cout << "\n=== Scan Summary ===" << std::endl;
    std::cout << "Files scanned: " << total_files_scanned << std::endl;
    std::cout << "Directories scanned: " << total_directories_scanned << std::endl;
    std::cout << "Files skipped: " << files_skipped << std::endl;
    std::cout << "Errors encountered: " << errors_encountered << std::endl;
    std::cout << "Total size scanned: " << FileScanner::format_file_size(total_size_scanned) << std::endl;
    std::cout << "Scan duration: " << scanner_utils::format_duration(scan_duration) << std::endl;
    
    if (!error_details.empty()) {
        std::cout << "\nErrors encountered:" << std::endl;
        for (const auto& [path, error] : error_details) {
            std::cout << "  " << path << ": " << error << std::endl;
        }
    }
}

// FileScanner Implementation
FileScanner::FileScanner(const ScanOptions& options) : options_(options) {}

std::vector<FileMetadata> FileScanner::scan_directory(const fs::path& root_path) {
    ScanTimer timer(stats_.scan_duration);
    reset_statistics();
    
    std::vector<FileMetadata> results;
    
    try {
        if (!fs::exists(root_path)) {
            log_error(root_path, "Path does not exist");
            return results;
        }
        
        if (!fs::is_directory(root_path)) {
            // If it's a single file, just process it
            if (auto metadata = get_file_metadata(root_path)) {
                results.push_back(*metadata);
                stats_.total_files_scanned = 1;
                stats_.total_size_scanned = metadata->size;
            }
            return results;
        }
        
        // Pre-allocate vector capacity for better performance
        reserve_capacity(results, root_path);
        
        // Choose scanning strategy based on options
        if (options_.scan_recursively) {
            scan_directory_recursive(root_path, results);
        } else {
            scan_directory_iterative(root_path, results);
        }
        
    } catch (const fs::filesystem_error& e) {
        handle_filesystem_error(e, root_path);
    } catch (const std::exception& e) {
        log_error(root_path, std::string("Unexpected error: ") + e.what());
    }
    
    return results;
}

std::vector<FileMetadata> FileScanner::scan_multiple_directories(const std::vector<fs::path>& paths) {
    std::vector<FileMetadata> combined_results;
    
    for (const auto& path : paths) {
        auto results = scan_directory(path);
        combined_results.insert(combined_results.end(), 
                              std::make_move_iterator(results.begin()),
                              std::make_move_iterator(results.end()));
    }
    
    return combined_results;
}

std::optional<FileMetadata> FileScanner::get_file_metadata(const fs::path& file_path) const {
    try {
        if (!fs::exists(file_path)) {
            return std::nullopt;
        }
        
        auto metadata = std::make_optional<FileMetadata>();
        metadata->filepath = file_path;
        metadata->extension = file_path.extension().string();
        metadata->file_type = fs::status(file_path).type();
        metadata->permissions = fs::status(file_path).permissions();
        metadata->is_hidden = is_hidden_file(file_path);
        metadata->is_symlink = fs::is_symlink(file_path);
        metadata->mime_type = detect_mime_type(file_path);
        
        if (fs::is_regular_file(file_path)) {
            metadata->size = fs::file_size(file_path);
            metadata->last_write_time = fs::last_write_time(file_path);
            
            // Try to get last access time (platform specific)
            #ifdef _WIN32
            if (auto atime = get_last_access_time_windows(file_path)) {
                metadata->last_access_time = *atime;
            } else {
                metadata->last_access_time = metadata->last_write_time;
            }
            #elif defined(__linux__)
            if (auto atime = get_last_access_time_linux(file_path)) {
                metadata->last_access_time = *atime;
            } else {
                metadata->last_access_time = metadata->last_write_time;
            }
            #else
            metadata->last_access_time = metadata->last_write_time;
            #endif
        } else {
            metadata->size = 0;
            metadata->last_write_time = fs::last_write_time(file_path);
            metadata->last_access_time = metadata->last_write_time;
        }
        
        return metadata;
        
    } catch (const fs::filesystem_error& e) {
        stats_.errors_encountered++;
        return std::nullopt;
    }
}

void FileScanner::scan_directory_recursive(const fs::path& dir_path, std::vector<FileMetadata>& results) {
    try {
        auto options = fs::directory_options::skip_permission_denied;
        if (options_.follow_symlinks) {
            // Default behavior follows symlinks
        } else {
            // Add option to not follow symlinks if available
        }
        
        for (const auto& entry : fs::recursive_directory_iterator(dir_path, options)) {
            try {
                if (should_skip_file(entry)) {
                    stats_.files_skipped++;
                    continue;
                }
                
                if (entry.is_directory()) {
                    stats_.total_directories_scanned++;
                    if (should_skip_directory(entry.path())) {
                        continue;
                    }
                }
                
                if (auto metadata = get_file_metadata(entry.path())) {
                    if (passes_size_filter(metadata->size) && 
                        passes_extension_filter(metadata->extension)) {
                        
                        results.emplace_back(std::move(*metadata));
                        stats_.total_files_scanned++;
                        stats_.total_size_scanned += metadata->size;
                        
                        // Call progress callback if provided
                        if (options_.progress_callback) {
                            if (!options_.progress_callback(entry.path(), results.size())) {
                                // User requested cancellation
                                break;
                            }
                        }
                    } else {
                        stats_.files_skipped++;
                    }
                }
                
            } catch (const fs::filesystem_error& e) {
                handle_filesystem_error(e, entry.path());
            }
        }
        
    } catch (const fs::filesystem_error& e) {
        handle_filesystem_error(e, dir_path);
    }
}

void FileScanner::scan_directory_iterative(const fs::path& dir_path, std::vector<FileMetadata>& results) {
    try {
        for (const auto& entry : fs::directory_iterator(dir_path)) {
            try {
                if (should_skip_file(entry)) {
                    stats_.files_skipped++;
                    continue;
                }
                
                if (auto metadata = get_file_metadata(entry.path())) {
                    if (passes_size_filter(metadata->size) && 
                        passes_extension_filter(metadata->extension)) {
                        
                        results.emplace_back(std::move(*metadata));
                        stats_.total_files_scanned++;
                        stats_.total_size_scanned += metadata->size;
                    } else {
                        stats_.files_skipped++;
                    }
                }
                
            } catch (const fs::filesystem_error& e) {
                handle_filesystem_error(e, entry.path());
            }
        }
    } catch (const fs::filesystem_error& e) {
        handle_filesystem_error(e, dir_path);
    }
}

bool FileScanner::should_skip_file(const fs::directory_entry& entry) const {
    try {
        // Skip hidden files if not requested
        if (!options_.include_hidden_files && is_hidden_file(entry.path())) {
            return true;
        }
        
        // Skip symlinks if not requested
        if (!options_.follow_symlinks && entry.is_symlink()) {
            return true;
        }
        
        // Skip if it's a directory that should be excluded
        if (entry.is_directory() && should_skip_directory(entry.path())) {
            return true;
        }
        
        return false;
        
    } catch (const fs::filesystem_error&) {
        return true; // Skip files we can't access
    }
}

bool FileScanner::should_skip_directory(const fs::path& dir_path) const {
    std::string dir_name = dir_path.filename().string();
    return options_.excluded_directories.find(dir_name) != options_.excluded_directories.end();
}

bool FileScanner::passes_size_filter(std::uintmax_t file_size) const noexcept {
    return file_size >= options_.min_file_size && file_size <= options_.max_file_size;
}

bool FileScanner::passes_extension_filter(const std::string& extension) const {
    // Convert to lowercase for case-insensitive comparison
    std::string lower_ext = extension;
    std::transform(lower_ext.begin(), lower_ext.end(), lower_ext.begin(), ::tolower);
    
    // If allowed extensions are specified, check if this extension is allowed
    if (!options_.allowed_extensions.empty()) {
        return options_.allowed_extensions.find(lower_ext) != options_.allowed_extensions.end();
    }
    
    // Check if this extension is excluded
    return options_.excluded_extensions.find(lower_ext) == options_.excluded_extensions.end();
}

void FileScanner::handle_filesystem_error(const fs::filesystem_error& e, const fs::path& path) const {
    stats_.errors_encountered++;
    log_error(path, e.what());
}

void FileScanner::log_error(const fs::path& path, const std::string& error_msg) const {
    stats_.error_details.emplace_back(path, error_msg);
}

void FileScanner::reserve_capacity(std::vector<FileMetadata>& container, const fs::path& root) const {
    try {
        // Estimate number of files by doing a quick count
        size_t estimated_files = 0;
        for ([[maybe_unused]] const auto& entry : fs::recursive_directory_iterator(
                root, fs::directory_options::skip_permission_denied)) {
            estimated_files++;
            if (estimated_files > 10000) break; // Don't spend too much time estimating
        }
        container.reserve(std::min(estimated_files, size_t(10000)));
    } catch (...) {
        container.reserve(1000); // Default reservation
    }
}

void FileScanner::set_scan_options(const ScanOptions& options) noexcept {
    options_ = options;
}

const ScanOptions& FileScanner::get_scan_options() const noexcept {
    return options_;
}

const ScanStatistics& FileScanner::get_last_scan_statistics() const noexcept {
    return stats_;
}

void FileScanner::reset_statistics() noexcept {
    stats_ = ScanStatistics{};
}

// Static utility methods
std::string FileScanner::detect_mime_type(const fs::path& file_path) {
    return scanner_utils::get_mime_type_from_extension(file_path.extension().string());
}

bool FileScanner::is_hidden_file(const fs::path& file_path) {
    std::string filename = file_path.filename().string();
    
    #ifdef _WIN32
    // On Windows, check file attributes
    DWORD attributes = GetFileAttributesW(file_path.c_str());
    if (attributes != INVALID_FILE_ATTRIBUTES) {
        return (attributes & FILE_ATTRIBUTE_HIDDEN) != 0;
    }
    #endif
    
    // Unix-style hidden files (starting with .)
    return !filename.empty() && filename[0] == '.';
}

std::string FileScanner::format_file_size(std::uintmax_t size) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB", "PB"};
    int unit_index = 0;
    double size_d = static_cast<double>(size);
    
    while (size_d >= 1024.0 && unit_index < 5) {
        size_d /= 1024.0;
        unit_index++;
    }
    
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << size_d << " " << units[unit_index];
    return ss.str();
}

#ifdef _WIN32
std::optional<fs::file_time_type> FileScanner::get_last_access_time_windows(const fs::path& path) const {
    WIN32_FILE_ATTRIBUTE_DATA fileData;
    if (GetFileAttributesExW(path.c_str(), GetFileExInfoStandard, &fileData)) {
        FILETIME ft = fileData.ftLastAccessTime;
        // Convert FILETIME to file_time_type
        ULARGE_INTEGER ull;
        ull.LowPart = ft.dwLowDateTime;
        ull.HighPart = ft.dwHighDateTime;
        
        // Convert to time_point
        auto tp = std::chrono::file_clock::from_sys(
            std::chrono::sys_time<std::chrono::nanoseconds>(
                std::chrono::nanoseconds((ull.QuadPart - 116444736000000000ULL) * 100)
            )
        );
        return tp;
    }
    return std::nullopt;
}
#endif

#ifdef __linux__
std::optional<fs::file_time_type> FileScanner::get_last_access_time_linux(const fs::path& path) const {
    struct stat file_stat;
    if (stat(path.c_str(), &file_stat) == 0) {
        auto tp = std::chrono::file_clock::from_sys(
            std::chrono::sys_time<std::chrono::seconds>(
                std::chrono::seconds(file_stat.st_atime)
            )
        );
        return tp;
    }
    return std::nullopt;
}
#endif

// ScanTimer Implementation
ScanTimer::ScanTimer(std::chrono::milliseconds& duration) 
    : start_time_(std::chrono::steady_clock::now()), duration_ref_(duration) {}

ScanTimer::~ScanTimer() {
    auto end_time = std::chrono::steady_clock::now();
    duration_ref_ = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time_);
}

// Utility namespace implementation
namespace scanner_utils {
    bool is_compressible_extension(const std::string& extension) {
        static const std::unordered_set<std::string> compressible_extensions = {
            ".txt", ".log", ".xml", ".html", ".css", ".js", ".json", ".csv",
            ".cpp", ".h", ".py", ".java", ".c", ".md", ".rst", ".yml", ".yaml"
        };
        
        std::string lower_ext = extension;
        std::transform(lower_ext.begin(), lower_ext.end(), lower_ext.begin(), ::tolower);
        return compressible_extensions.find(lower_ext) != compressible_extensions.end();
    }
    
    std::string get_mime_type_from_extension(const std::string& extension) {
        static const std::unordered_map<std::string, std::string> mime_types = {
            {".txt", "text/plain"},
            {".html", "text/html"},
            {".css", "text/css"},
            {".js", "application/javascript"},
            {".json", "application/json"},
            {".xml", "application/xml"},
            {".pdf", "application/pdf"},
            {".jpg", "image/jpeg"},
            {".jpeg", "image/jpeg"},
            {".png", "image/png"},
            {".gif", "image/gif"},
            {".bmp", "image/bmp"},
            {".mp4", "video/mp4"},
            {".avi", "video/avi"},
            {".mp3", "audio/mpeg"},
            {".wav", "audio/wav"},
            {".zip", "application/zip"},
            {".rar", "application/rar"},
            {".tar", "application/tar"},
            {".gz", "application/gzip"},
            {".exe", "application/octet-stream"},
            {".dll", "application/octet-stream"},
            {".so", "application/octet-stream"}
        };
        
        std::string lower_ext = extension;
        std::transform(lower_ext.begin(), lower_ext.end(), lower_ext.begin(), ::tolower);
        
        auto it = mime_types.find(lower_ext);
        return (it != mime_types.end()) ? it->second : "application/octet-stream";
    }
    
    double estimate_compression_ratio(const std::string& extension) {
        if (is_compressible_extension(extension)) {
            return 0.3; // Text files typically compress to ~30% of original size
        } else if (extension == ".zip" || extension == ".rar" || extension == ".gz") {
            return 1.0; // Already compressed
        } else if (extension == ".jpg" || extension == ".png" || extension == ".mp3" || extension == ".mp4") {
            return 0.95; // Slightly compressible
        }
        return 0.7; // Default compression ratio for binary files
    }
    
    std::string format_duration(std::chrono::milliseconds duration) {
        auto total_ms = duration.count();
        
        if (total_ms < 1000) {
            return std::to_string(total_ms) + " ms";
        } else if (total_ms < 60000) {
            return std::to_string(total_ms / 1000.0) + " seconds";
        } else {
            auto minutes = total_ms / 60000;
            auto seconds = (total_ms % 60000) / 1000.0;
            return std::to_string(minutes) + " min " + std::to_string(seconds) + " sec";
        }
    }
}
