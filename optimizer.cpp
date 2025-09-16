#include "optimizer.h"
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <fstream>
// #include "huffman.h"

namespace fs = std::filesystem;

void optimizer::optimizeFiles(std::vector<FileInfo> &files)
{
    for (auto &file : files)
    {
        if (shouldDelete(file))
        {

            std::cout << "Do you want to delete " << file.path << "? (y/n)";
            char choice;
            std::cin >> choice;
            if (choice == 'y' || choice == 'Y')
                deleteFile(file);
        }
        else if (shouldCompress(file))
        {
            std::cout << "Do you want to compress " << file.path << "? (y/n): ";
            char choice;
            std::cin >> choice;
            if (choice == 'y' || choice == 'Y')
                compressFile(file);
        }
    }
}
