#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <string>
#include <unordered_map>
#include <fstream>

// Forward declaration for internal Node structure
struct Node;

// Main compression/decompression functions (these match your huffman.cpp)
void compressFile(const std::string& inputFile, const std::string& outputFile);
void decompressFile(const std::string& inputFile, const std::string& outputFile);

// Internal helper functions (optional to declare, but good practice)
Node* buildTree(std::unordered_map<char, int>& freq);
void generateCodes(Node* root, const std::string& code, std::unordered_map<char, std::string>& codes);
void writeCompressedData(std::ifstream& in, const std::unordered_map<char, std::string>& codes, std::ofstream& out);

#endif
