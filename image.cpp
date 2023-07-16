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
#include "image.h"
#include "lzw.h"
#include "file.h"
#include "lodepng.h"

bool Image::loadTD3LZImageFile(const std::string &srcFilename, int imageWidth, const std::string &srcPaletteFilename) {
    width = imageWidth;
    delete pixels;
    pixels = nullptr;

    LZWDecoder lzwDecoder;
    auto decodedBuffer = lzwDecoder.decode(srcFilename);
    auto unpackedPixels = unpackRLE(decodedBuffer);

    height = unpackedPixels.size() / width;
    if (unpackedPixels.size() % width != 0) {
        std::cout << "Error: insufficient image data for specified width: " << width << "\n\n";
        exit(1);
    }

    generatePixelBufFromUnpackedRLEData(unpackedPixels);

    loadPalette(srcPaletteFilename);

    return true;
}

bool Image::loadPngFile(const std::string &srcFilename) {
    std::vector<unsigned char> png;
    lodepng::State state;

    unsigned error = lodepng::load_file(png, srcFilename);
    if (!error) error = lodepng_inspect(&width, &height, &state, &png[0], png.size());
    if (error) {
        std::cout << "[loadPngFile] error reading file " << error << ": "<< lodepng_error_text(error) << std::endl;
        return false;
    }

    if (state.info_png.color.colortype != LCT_PALETTE) {
        printf("[loadPngFile] Only indexed PNG files allowed\n");
        return false;
    }

    state.info_raw.colortype = LCT_PALETTE;
    state.info_raw.bitdepth = 8;

    error = lodepng_decode(&pixels, &width, &height, &state, &png[0], png.size());
    if (error) {
        std::cout << "[read_png_file] decoder error " << error << ": "<< lodepng_error_text(error) << std::endl;
        return false;
    }
    return false;
}

bool Image::savePngFile(const std::string &pngFilename) {
    std::vector<unsigned char> png;
    lodepng::State state;
    state.info_raw.colortype = LCT_PALETTE;
    state.info_raw.bitdepth = 8;
    state.info_png.color.colortype = LCT_PALETTE;
    state.info_png.color.bitdepth = 8;
    state.encoder.auto_convert = false;


    for (int i = 0; i < palette.size(); i += 3) {
        // copy the palette into src + dest info. This preserves the palette. Other wise it strips unused colours on save.
        lodepng_palette_add(
                &state.info_raw,
                palette[i],
                palette[i+1],
                palette[i+2],
                0xFF
        );
        lodepng_palette_add(
                &state.info_png.color,
                palette[i],
                palette[i+1],
                palette[i+2],
                0xFF
        );
    }
    unsigned error = lodepng::encode(png, pixels, (unsigned int)width, (unsigned int)height, state);
    if(!error) lodepng::save_file(png, pngFilename);
    if (error) {
        std::cout << "[savePngFile] encoder error " << error << ": "<< lodepng_error_text(error) << std::endl;
        return false;
    }

    return true;
}

std::vector<uint8_t> Image::unpackRLE(std::vector<uint8_t> packedData) {
    std::vector<uint8_t> outBuffer;
    for (int curPos = 0; curPos < packedData.size(); curPos += 2) {
        uint8_t pixelValue = packedData[curPos];
        uint8_t lengthByte = packedData[curPos + 1];

        if (lengthByte & 1) {
            outBuffer.emplace_back(pixelValue);
        }
        lengthByte = lengthByte >> 1;
        for (int i = 0; i < lengthByte; i++) {
            outBuffer.emplace_back(pixelValue);
            outBuffer.emplace_back(pixelValue);
        }
    }
    return outBuffer;
}

void Image::generatePixelBufFromUnpackedRLEData(const std::vector<uint8_t> &unpackedPixels) {
    pixels = new uint8_t [unpackedPixels.size()];

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            pixels[(height - 1 - y) * width + x] = unpackedPixels[y * width + x];
        }
    }
}

const static uint8_t basePal[48] = {
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x28,
        0x00, 0x28, 0x00,
        0x00, 0x28, 0x28,
        0x28, 0x00, 0x00,
        0x28, 0x00, 0x28,
        0x28, 0x14, 0x00,
        0x28, 0x28, 0x28,
        0x14, 0x14, 0x14,
        0x14, 0x14, 0x3c,
        0x14, 0x3c, 0x14,
        0x14, 0x3c, 0x3c,
        0x3c, 0x14, 0x14,
        0x3c, 0x14, 0x3c,
        0x3c, 0x3c, 0x14,
        0x3c, 0x3c, 0x3c
};

void Image::loadPalette(const std::string &srcPaletteFilename) {
    palette.clear();
    palette.reserve(256 * 3);
    auto srcFile = openFileForRead(srcPaletteFilename);

    for (auto c : basePal) {
        c = c << 2;
        palette.emplace_back(c);
    }

    for (int i = 0; i < 336; i++) {
        uint8_t c;
        srcFile.read(reinterpret_cast<char *>(&c), 1);
        c = c << 2;
        palette.emplace_back(c);
    }
    srcFile.close();
}

bool Image::saveLZWFile(const std::string &outFilename) {
    auto flippedPixels = formatPixelsForRLE();
    auto rlePixels = packRLE(flippedPixels);

    LZWEncoder lzwEncoder;
    lzwEncoder.encode(rlePixels, outFilename);
    return true;
}

std::vector<uint8_t> Image::packRLE(std::vector<uint8_t> &unpackedData) {
    std::vector<uint8_t> rleData;
    for (int curPos = 0; curPos < unpackedData.size(); ) {
        auto pixel = unpackedData[curPos];
        curPos++;
        int runLength = 1;
        for(; curPos < unpackedData.size() && unpackedData[curPos] == pixel && runLength < 255; curPos++) {
            runLength++;
        }
        rleData.emplace_back(pixel);
        rleData.emplace_back(runLength);
    }
    return rleData;
}

std::vector<uint8_t> Image::formatPixelsForRLE() {
    std::vector<uint8_t> flippedRows;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            flippedRows.emplace_back(pixels[(height - 1 - y) * width + x]);
        }
    }
    return flippedRows;
}
