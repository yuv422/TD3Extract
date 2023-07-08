/*
MIT License

Copyright (c) 2023 Eric Fry

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include <iostream>
#include "file.h"

std::ifstream openFileForRead(const std::string &file) {
    auto fp = std::ifstream(file, std::ios::binary);
    if (!fp.is_open()) {
        std::cout << "Error: Failed to open " << file << "\n";
        exit(1);
    }
    return fp;
}

std::ofstream openFileForWrite(const std::string &file) {
    auto fp = std::ofstream(file, std::ios::binary);
    if (!fp.is_open()) {
        std::cout << "Error: Failed to open '" << file << "' for writing.\n";
        exit(1);
    }
    return fp;
}

int getFileSize(std::ifstream &file) {
    file.seekg(0, std::ios_base::end);
    int size = (int)file.tellg();
    file.seekg(0, std::ios_base::beg);
    return size;
}
