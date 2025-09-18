#include "huffman.h"
#include <iostream>
#include <fstream>
#include <queue>
#include <unordered_map>
#include <vector>

struct Node {
    char ch;
    int freq;
    Node* left = nullptr;
    Node* right = nullptr;
    Node(char c, int f) : ch(c), freq(f) {}
};

struct Compare {
    bool operator()(Node* a, Node* b) {
        return a->freq > b->freq;
    }
};

Node* buildTree(std::unordered_map<char, int>& freq) {
    std::priority_queue<Node*, std::vector<Node*>, Compare> pq;
    for (const auto& p : freq) {
        pq.push(new Node(p.first, p.second));
    }
    while (pq.size() > 1) {
        Node* left = pq.top(); pq.pop();
        Node* right = pq.top(); pq.pop();
        Node* parent = new Node('\0', left->freq + right->freq);
        parent->left = left; parent->right = right;
        pq.push(parent);
    }
    return pq.top();
}

void generateCodes(Node* root, const std::string& code, std::unordered_map<char, std::string>& codes) {
    if (!root) return;
    if (!root->left && !root->right) {
        codes[root->ch] = code.empty() ? "0" : code;
    } else {
        generateCodes(root->left, code + "0", codes);
        generateCodes(root->right, code + "1", codes);
    }
}

void writeCompressedData(std::ifstream& in, const std::unordered_map<char, std::string>& codes, std::ofstream& out) {
    std::string buffer;
    char ch;
    while (in.get(ch)) {
        buffer += codes.at(ch);
        while (buffer.length() >= 8) {
            unsigned char byte = 0;
            for (int i = 0; i < 8; ++i) {
                if (buffer[i] == '1') byte |= (1 << (7 - i));
            }
            out.put(byte);
            buffer = buffer.substr(8);
        }
    }
    if (!buffer.empty()) {
        buffer.append(8 - buffer.length(), '0');
        unsigned char byte = 0;
        for (int i = 0; i < 8; ++i) {
            if (buffer[i] == '1') byte |= (1 << (7 - i));
        }
        out.put(byte);
    }
}

void compressFile(const std::string& inputFile, const std::string& outputFile) {
    std::ifstream in(inputFile);
    std::unordered_map<char, int> freq;
    char ch;
    while (in.get(ch)) freq[ch]++;
    in.close();

    Node* root = buildTree(freq);
    std::unordered_map<char, std::string> codes;
    generateCodes(root, "", codes);

    std::ofstream out(outputFile, std::ios::binary);
    int mapSize = freq.size();
    out.write(reinterpret_cast<char*>(&mapSize), sizeof(int));
    for (const auto& p : freq) {
        out.write(&p.first, sizeof(char));
        out.write(reinterpret_cast<const char*>(&p.second), sizeof(int));
    }

    in.open(inputFile);
    writeCompressedData(in, codes, out);
    in.close();
    out.close();
    std::cout << "File compressed to " << outputFile << std::endl;
}

void decompressFile(const std::string& inputFile, const std::string& outputFile) {
    std::ifstream in(inputFile, std::ios::binary);
    std::unordered_map<char, int> freq;

    int mapSize;
    in.read(reinterpret_cast<char*>(&mapSize), sizeof(int));
    for (int i = 0; i < mapSize; ++i) {
        char ch;
        int f;
        in.read(&ch, sizeof(char));
        in.read(reinterpret_cast<char*>(&f), sizeof(int));
        freq[ch] = f;
    }

    Node* root = buildTree(freq);

    int totalChars = 0;
    for (const auto& p : freq) totalChars += p.second;

    std::ofstream out(outputFile);
    Node* current = root;
    unsigned char byte;
    int decodedChars = 0;

    while (in.read(reinterpret_cast<char*>(&byte), 1) && decodedChars < totalChars) {
        for (int i = 7; i >= 0 && decodedChars < totalChars; --i) {
            current = (byte >> i) & 1 ? current->right : current->left;
            if (!current->left && !current->right) {
                out.put(current->ch);
                decodedChars++;
                current = root;
            }
        }
    }
    in.close();
    out.close();
    std::cout << "File decompressed to " << outputFile << std::endl;
}
