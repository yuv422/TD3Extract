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
#ifndef TD3EXTRACT_LZW_H
#define TD3EXTRACT_LZW_H

#include <fstream>
#include <string>
#include <map>
#include <vector>

typedef std::vector<uint8_t> Sequence;

class LZWDecoder {
private:
    std::ifstream srcFile;
    std::vector<uint8_t> decodedBuffer;
    std::map<int, Sequence> dictionary;
    int nextAvailableCodeId = 0x102;
    int currentCodeIdBitLength = 9;
    int codeIdBitLengthChange = 0x200;

    Sequence previousEmittedSequence;

    unsigned char *inputBuf = nullptr;
    int inputSize = 0;
    int curBitPosition = 0;
public:
    LZWDecoder();
    bool decode(const std::string &srcFilename, const std::string &outFilename);
    std::vector<uint8_t> decode(const std::string &srcFilename);

private:
    void resetState();
    int getNextCodeFromInput();
    bool isCodeIdInDictionary(int codeId);
    void writeSequenceToFile(Sequence &sequence);
    void addSequenceToDictionary(Sequence &sequence);
};

class LZWEncoder {
private:
    std::vector<uint8_t> inputBuffer;
    std::vector<uint8_t> lzwBuffer;
    std::map<Sequence, int> dictionary;
    int nextAvailableCodeId = 0x102;
    int currentCodeIdBitLength = 9;
    int codeIdBitLengthChange = 0x200;

    Sequence previousEmittedSequence;

    int curBitPosition = 0;
    int curInputPosition = 0;
public:
    LZWEncoder() = default;
    std::vector<uint8_t> encode(const std::vector<uint8_t> &data);
    void encode(const std::vector<uint8_t> &data, const std::string &outFilename);

private:
    void resetState();
    bool writeCodeId(int codeId);
    bool isSequenceInDictionary(Sequence &sequence);
    void addSequenceToDictionary(Sequence &sequence);
};

#endif //TD3EXTRACT_LZW_H
