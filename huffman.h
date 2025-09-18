#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <string>

struct HuffmanNode {
    char ch;
    int freq;
    HuffmanNode *left, *right;
    HuffmanNode(char c, int f);
};

HuffmanNode* buildHuffmanTree(const int freq[], int size);
void generateCodes(HuffmanNode* root, std::string path, std::string codes[]);
std::string encode(const std::string &text, std::string codes[]);
std::string decode(const std::string &encoded, HuffmanNode* root);
void deleteTree(HuffmanNode* node);

#endif
