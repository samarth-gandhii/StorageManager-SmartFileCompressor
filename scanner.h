#pragma once

#include <filesystem>
#include <vector>
#include <string>
#include <optional>
#include <chrono>
#include <unordered_set>
#include <functional>

namespace fs = std::filesystem;

/**
 * @brief Structure to hold comprehensive file metadata
 */
struct FileMetadata {
    fs::path filepath;
    std::uintmax_t size;
    fs::file_time_type last_write_time;
    fs::file_time_type last_access_time;
    std::string extension;
    std::string mime_type;
    fs::file_type file_type;
    fs::perms permissions;
    bool is_hidden;
    bool is_symlink;
    
    // Constructor for easy initialization
    FileMetadata() = default;
    FileMetadata(const fs::path& path);
    
    // Helper methods
    bool is_regular_file() const noexcept;
    bool is_directory() const noexcept;
    std::string get_size_string() const;
    std::string get_last_modified_string() const;
};

/**
 * @brief Configuration options for file scanning
 */
struct ScanOptions {
    bool include_hidden_files = false;
    bool follow_symlinks = false;
    bool scan_recursively = true;
    std::uintmax_t max_file_size = std::numeric_limits<std::uintmax_t>::max();
    std::uintmax_t min_file_size = 0;
    
    // File type filters
    std::unordered_set<std::string> allowed_extensions;
    std::unordered_set<std::string> excluded_extensions;
    std::unordered_set<std::string> excluded_directories{"$RECYCLE.BIN", "System Volume Information", ".git", ".svn", "node_modules"};
    
    // Progress callback function type
    using ProgressCallback = std::function<bool(const fs::path&, size_t)>;
    ProgressCallback progress_callback = nullptr;
};

/**
 * @brief Statistics collected during scanning
 */
struct ScanStatistics {
    size_t total_files_scanned = 0;
    size_t total_directories_scanned = 0;
    size_t files_skipped = 0;
    size_t errors_encountered = 0;
    std::uintmax_t total_size_scanned = 0;
    std::chrono::milliseconds scan_duration{0};
    
    // Error details
    std::vector<std::pair<fs::path, std::string>> error_details;
    
    void print_summary() const;
};

/**
 * @brief High-performance file system scanner with modern C++ features
 */
class FileScanner {
public:
    FileScanner() = default;
    explicit FileScanner(const ScanOptions& options);
    
    // Main scanning methods
    std::vector<FileMetadata> scan_directory(const fs::path& root_path);
    std::vector<FileMetadata> scan_multiple_directories(const std::vector<fs::path>& paths);
    
    // Metadata extraction
    std::optional<FileMetadata> get_file_metadata(const fs::path& file_path) const;
    
    // Configuration
    void set_scan_options(const ScanOptions& options) noexcept;
    const ScanOptions& get_scan_options() const noexcept;
    
    // Statistics
    const ScanStatistics& get_last_scan_statistics() const noexcept;
    void reset_statistics() noexcept;
    
    // Utility methods
    static std::string detect_mime_type(const fs::path& file_path);
    static bool is_hidden_file(const fs::path& file_path);
    static std::string format_file_size(std::uintmax_t size);
    
private:
    ScanOptions options_;
    mutable ScanStatistics stats_;
    
    // Internal scanning methods
    void scan_directory_recursive(const fs::path& dir_path, std::vector<FileMetadata>& results);
    void scan_directory_iterative(const fs::path& dir_path, std::vector<FileMetadata>& results);
    
    // Filtering and validation
    bool should_skip_file(const fs::directory_entry& entry) const;
    bool should_skip_directory(const fs::path& dir_path) const;
    bool passes_size_filter(std::uintmax_t file_size) const noexcept;
    bool passes_extension_filter(const std::string& extension) const;
    
    // Error handling
    void handle_filesystem_error(const fs::filesystem_error& e, const fs::path& path) const;
    void log_error(const fs::path& path, const std::string& error_msg) const;
    
    // Performance helpers
    void reserve_capacity(std::vector<FileMetadata>& container, const fs::path& root) const;
    
    // Platform-specific optimizations
    #ifdef _WIN32
    std::optional<fs::file_time_type> get_last_access_time_windows(const fs::path& path) const;
    #endif
    
    #ifdef __linux__
    std::optional<fs::file_time_type> get_last_access_time_linux(const fs::path& path) const;
    #endif
};

/**
 * @brief RAII wrapper for timing operations
 */
class ScanTimer {
public:
    explicit ScanTimer(std::chrono::milliseconds& duration);
    ~ScanTimer();
    
private:
    std::chrono::steady_clock::time_point start_time_;
    std::chrono::milliseconds& duration_ref_;
};

// Convenience type aliases
using FileList = std::vector<FileMetadata>;
using ScanResult = std::pair<FileList, ScanStatistics>;

// Global utility functions
namespace scanner_utils {
    /**
     * @brief Check if a file extension indicates a compressible file type
     */
    bool is_compressible_extension(const std::string& extension);
    
    /**
     * @brief Get appropriate MIME type based on file extension
     */
    std::string get_mime_type_from_extension(const std::string& extension);
    
    /**
     * @brief Calculate estimated compression ratio for a file type
     */
    double estimate_compression_ratio(const std::string& extension);
    
    /**
     * @brief Format duration for human-readable output
     */
    std::string format_duration(std::chrono::milliseconds duration);
}
