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
    bool shouldDelete(const FileInfo &file);
    bool shouldCompress(const FileInfo &file);
    // double calculateUsedSpace(const std::vector<FileInfo>& files);

    void deleteFile(FileInfo &file);
    void compressFile(FileInfo &file); // Will internally call Huffman
};
#endif
