#ifndef UTILS_H
#define UTILS_H

#include <string>
#include<fstream>
#include <cstdint> 
#include <vector>
// Format bytes to MB string with 2 decimals
std::string formatSizeMB(uintmax_t bytes);

// Ask Yes/No safely
bool askYesNo(const std::string& msg);

#endif
