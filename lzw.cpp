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
#include "lzw.h"
#include "file.h"

LZWDecoder::LZWDecoder() = default;

constexpr int END_OF_STREAM_MARKER = 0x101;
constexpr int RESET_DICTIONARY_MARKER = 0x100;
constexpr int MAX_CODE_ID_BIT_LENGTH = 12;

std::vector<uint8_t> LZWDecoder::decode(const std::string &srcFilename) {
    resetState();
    decodedBuffer.clear();
    srcFile = openFileForRead(srcFilename);
    inputSize = getFileSize(srcFile);
    inputBuf = new unsigned char [inputSize];
    srcFile.read((char *)inputBuf, inputSize);
    curBitPosition = 0;

    int nextCodeId;
    while (nextCodeId = getNextCodeFromInput(), nextCodeId != END_OF_STREAM_MARKER) {
        if (nextCodeId == RESET_DICTIONARY_MARKER) {
            resetState();
            nextCodeId = getNextCodeFromInput();
            Sequence newSequence = {(uint8_t)nextCodeId};
            writeSequenceToFile(newSequence);
            previousEmittedSequence = newSequence;
        } else {
            if(isCodeIdInDictionary(nextCodeId)) {
                auto sequence = dictionary[nextCodeId];
                writeSequenceToFile(sequence);
                Sequence newSequence = previousEmittedSequence;
                newSequence.emplace_back(sequence[0]);
                addSequenceToDictionary(newSequence);
                previousEmittedSequence = sequence;
            } else {
                if (!previousEmittedSequence.empty()) {
                    Sequence newSequence = previousEmittedSequence;
                    newSequence.emplace_back(previousEmittedSequence[0]);
                    addSequenceToDictionary(newSequence);
                    writeSequenceToFile(newSequence);
                    previousEmittedSequence = newSequence;
                }
            }
        }
    }

    delete inputBuf;
    srcFile.close();
    return decodedBuffer;
}

bool LZWDecoder::decode(const std::string &srcFilename, const std::string &outFilename) {
    auto outFile = openFileForWrite(outFilename);

    decode(srcFilename);

    for (int i = 0; i < decodedBuffer.size(); i++) {
        uint8_t value = decodedBuffer[i];
        outFile.write(reinterpret_cast<const char *>(&value), 1);
    }

    return true;
}

void LZWDecoder::resetState() {
    dictionary.clear();
    for (int i = 0; i < 256; i++) {
        dictionary[i] = {(uint8_t)i};
    }
    nextAvailableCodeId = 0x102;
    currentCodeIdBitLength = 9;
    codeIdBitLengthChange = 0x200;
    previousEmittedSequence.clear();
}

static const unsigned short lzwCodeIdBitMaskTbl[] = {0x1ff, 0x3ff, 0x7ff, 0xfff};

int LZWDecoder::getNextCodeFromInput() {
    int byteOffset = curBitPosition / 8;
    int byteRemainder = curBitPosition % 8;
    curBitPosition += currentCodeIdBitLength;

    unsigned short word = inputBuf[byteOffset] + ((unsigned short)inputBuf[byteOffset + 1] << 8);
    unsigned char byte = inputBuf[byteOffset + 2];

    for(; byteRemainder != 0; byteRemainder--) {
        word = word >> 1 | (unsigned short)(byte & 1) << 0xf;
        byte = byte >> 1;
    }
    int codeId = word & lzwCodeIdBitMaskTbl[currentCodeIdBitLength - 9];
//    printf("codeId: %d bitlength: %d\n", codeId, currentCodeIdBitLength);
    return codeId;
}

bool LZWDecoder::isCodeIdInDictionary(int codeId) {
    return dictionary.find(codeId) != dictionary.end();
}

void LZWDecoder::writeSequenceToFile(Sequence &sequence) {
    for (auto &value : sequence) {
        decodedBuffer.emplace_back(value);
//        outFile.write(reinterpret_cast<const char *>(&value), 1);
    }
}

void LZWDecoder::addSequenceToDictionary(Sequence &sequence) {
    dictionary[nextAvailableCodeId] = sequence;
    nextAvailableCodeId++;
    if(nextAvailableCodeId >= codeIdBitLengthChange && currentCodeIdBitLength != MAX_CODE_ID_BIT_LENGTH) {
        currentCodeIdBitLength++;
        codeIdBitLengthChange = codeIdBitLengthChange << 1;
    }
}


void LZWEncoder::encode(const std::vector<uint8_t> &data, const std::string &outFilename) {
    auto outFile = openFileForWrite(outFilename);

    auto lzw = encode(data);

    for (int i = 0; i < lzw.size(); i++) {
        uint8_t value = lzw[i];
        outFile.write(reinterpret_cast<const char *>(&value), 1);
    }
    outFile.close();
}

std::vector<uint8_t> LZWEncoder::encode(const std::vector<uint8_t> &data) {
    inputBuffer = data;
    lzwBuffer.clear();
    curBitPosition = 0;
    resetState();
    writeCodeId(RESET_DICTIONARY_MARKER);

    while(curInputPosition < inputBuffer.size()) {
        Sequence sequence;
        sequence.emplace_back(inputBuffer[curInputPosition++]);
        auto newSequence = sequence;
        while (isSequenceInDictionary(newSequence) && curInputPosition < inputBuffer.size()) {
            newSequence.emplace_back(inputBuffer[curInputPosition++]);
            if (isSequenceInDictionary(newSequence)) {
                sequence = newSequence;
            }
        }
        writeCodeId(dictionary[sequence]);
        if (sequence.size() != newSequence.size()) {
            addSequenceToDictionary(newSequence);
            curInputPosition--;
        }
    }

    writeCodeId(END_OF_STREAM_MARKER);

    return lzwBuffer;
}

bool LZWEncoder::isSequenceInDictionary(Sequence &sequence) {
    return dictionary.find(sequence) != dictionary.end();
}

void LZWEncoder::addSequenceToDictionary(Sequence &sequence) {
    dictionary[sequence] = nextAvailableCodeId;
    nextAvailableCodeId++;

    if(nextAvailableCodeId >= codeIdBitLengthChange+1) {
        if (currentCodeIdBitLength == MAX_CODE_ID_BIT_LENGTH) {
            writeCodeId(RESET_DICTIONARY_MARKER);
            resetState();
        } else {
            currentCodeIdBitLength++;
            codeIdBitLengthChange = codeIdBitLengthChange << 1;
        }
    }
}

void LZWEncoder::resetState() {
    dictionary.clear();
    for (int i = 0; i < 256; i++) {
        dictionary[{(uint8_t)i}] = i;
    }
    nextAvailableCodeId = 0x102;
    currentCodeIdBitLength = 9;
    codeIdBitLengthChange = 0x200;
    previousEmittedSequence.clear();
}

bool LZWEncoder::writeCodeId(int codeId) {
//    printf("codeId: %d bitlength: %d\n", codeId, currentCodeIdBitLength);
//    int byteOffset = curBitPosition / 8;
    int byteRemainder = curBitPosition % 8;

    if (byteRemainder == 0) {
        lzwBuffer.emplace_back(codeId & 0xff);
        lzwBuffer.emplace_back(codeId >> 8);
    } else {
        auto byte = lzwBuffer.back();
        lzwBuffer[lzwBuffer.size()-1] = byte | ((codeId & 0xff) << byteRemainder);
        lzwBuffer.emplace_back(codeId >> (8 - byteRemainder));
        if ((8 - byteRemainder) + 8 < currentCodeIdBitLength) {
            lzwBuffer.emplace_back((codeId >> ((8 - byteRemainder) + 8)));
        }
    }
    curBitPosition += currentCodeIdBitLength;

    return false;
}
