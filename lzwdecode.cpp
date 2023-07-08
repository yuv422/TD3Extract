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
#include "lzwdecode.h"
#include "file.h"

LZWDecoder::LZWDecoder() = default;

constexpr int END_OF_STREAM_MARKER = 0x101;
constexpr int RESET_DICTIONARY_MARKER = 0x100;
constexpr int MAX_CODE_ID_BIT_LENGTH = 12;

bool LZWDecoder::decode(const std::string &srcFilename, const std::string &outFilename) {
    resetState();
    srcFile = openFileForRead(srcFilename);
    outFile = openFileForWrite(outFilename);
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
    outFile.close();
    return false;
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
    return word & lzwCodeIdBitMaskTbl[currentCodeIdBitLength - 9];
}

bool LZWDecoder::isCodeIdInDictionary(int codeId) {
    return dictionary.find(codeId) != dictionary.end();
}

void LZWDecoder::writeSequenceToFile(Sequence &sequence) {
    for (auto &value : sequence) {
        outFile.write(reinterpret_cast<const char *>(&value), 1);
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
