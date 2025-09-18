#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include <vector>
#include <string>
#include "scanner.h"

class optimizer
{
public:
    optimizer(double size);

    void optimizeFiles(std::vector<FileInfo> &files);

private:
    double totalSpace;

    // New: DP-based knapsack ranking
    std::vector<FileInfo> rankFilesKnapsack(const std::vector<FileInfo> &files);

    // Only need shouldCompress (for text files)
    bool shouldCompress(const FileInfo &file);

    // File operations
    void deleteFile(FileInfo &file);
    void compressFile(FileInfo &file); // Will internally call Huffman
};

#endif
