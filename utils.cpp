#include "utils.h"
#include <iostream>
#include <iomanip>
#include <sstream>

std::string formatSizeMB(uintmax_t bytes) {
    double mb = bytes / (1024.0 * 1024.0);
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(2) << mb << " MB";
    return ss.str();
}

bool askYesNo(const std::string& msg) {
    char choice;
    std::cout << msg << " (y/n): ";
    std::cin >> choice;
    return (choice == 'y' || choice == 'Y');
}
