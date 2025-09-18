#ifndef DUPLICATES_H
#define DUPLICATES_H

#include "scanner.h"
#include <vector>
#include <unordered_map>

std::unordered_map<std::string, std::vector<FileInfo>>
findDuplicates(const std::vector<FileInfo>& files);

void handleDuplicates(std::vector<FileInfo> &files);


#endif