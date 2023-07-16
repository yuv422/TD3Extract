//
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
#ifndef TD3EXTRACT_IMAGE_H
#define TD3EXTRACT_IMAGE_H

#include <string>
#include <vector>

class Image {
private:
    unsigned int width = 0;
    unsigned int height = 0;
    std::vector<uint8_t> palette;
    uint8_t *pixels = nullptr;

public:
    bool loadTD3LZImageFile(const std::string &srcFilename, int imageWidth, const std::string &srcPaletteFilename);
    bool loadPngFile(const std::string &srcFilename);
    bool savePngFile(const std::string &pngFilename);
    bool saveLZWFile(const std::string &outFilename);

private:
    void loadPalette(const std::string &srcPaletteFilename);
    std::vector<uint8_t> unpackRLE(std::vector<uint8_t> packedData);
    void generatePixelBufFromUnpackedRLEData(const std::vector<uint8_t> &unpackedPixels);

    std::vector<uint8_t> formatPixelsForRLE();
    std::vector<uint8_t> packRLE(std::vector<uint8_t> &unpackedData);
};
#endif //TD3EXTRACT_IMAGE_H
