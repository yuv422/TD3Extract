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
#include <map>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include "file.h"
#include "lzwdecode.h"

short calcHash1(const std::string &filename, short seed) {
    int hash = 0;
    for (int i = (int)filename.length() - 1; i >= 0; i--) {
        hash = hash * seed;
        hash += (short)filename[i];
    }
    return (short)(hash & 0xffff);
}

short calcHash2(const std::string &filename) {
    int hash = 0;
    for (int i = 0; i < filename.length() - 1; i++) {
        hash = hash + i * filename[i];
    }
    return (short)(hash & 0xffff);
}

int calcFilenameHash(const std::string &filename) {
    short h1 = calcHash1(filename, 0x101);
    short h2 = calcHash2(filename);
    return ((int)h1 << 16) + h2;
}

#pragma pack(1)
struct DataArchiveFileStruct {
    unsigned int id;
    short archiveFileId;
    unsigned int offset;
    unsigned int size;
};

struct PlayDisk {
    std::vector<std::string> cars;
    std::vector<std::string> scenes;
};

char engineFilenames[][13] = {
        "COMPASS.LZ",
        "WATER.LZ",
        "WATEREGA.LZ",
        "CHASE.LZ",
        "BROKE.LZ",
        "BROKEGA.LZ",
        "ACCOCOLR.BIN",
        "ACCO.LZ",
        "TITLCOLR.BIN",
        "TITLE2.LZ",
        "TITLE1.LZ",
        "TITL2COL.BIN",
        "TITLEANI.LZ",
        "TITLELET.LZ",
        "TITLEL2.LZ",
        "TITLECAR.LZ",
        "CREDCOLR.BIN",
        "CREDITC.LZ",
        "CREDITB.LZ",
        "CREDITA.LZ",
        "TOPCOLR.BIN",
        "TOPSCORC.LZ",
        "TOPSCORB.LZ",
        "TOPSCORA.LZ",
        "SELCOLR.BIN",
        "OTWCOL.BIN",
        "THEME.MUS",
        "COPCOLR.BIN",
        "COPB.LZ",
        "COPA.LZ",
        "COPSEQ.LZ",
        "KEYCOLR.BIN",
        "KEYS.LZ",
        "MASTERQ.BIN",
        "DIFFCOLR.BIN",
        "DETAIL1.LZ",
        "DETAIL2.LZ",
        "SELECT.LZ",
        "DIFFLEVA.LZ",
        "DIFFLEVB.LZ",
        "DIFFLEVC.LZ",
        "SSBJ.LZ",
        "SCENETTT.BIN",
        "NEWWAVE.MUS",
        "SCENETTO.BIN",
        "SCENETTP.BIN",
        "SCENETTA.DAT",
        "SCENETT1.DAT"
};

char carFilenameSuffixes[][13] = {
        "SIC.BIN",
        ".SIC",
        ".SID",
        "SC.BIN",
        "FL1.LZ",
        "FL2.LZ",
        ".BIC",
        ".ICN",
        "1.BOT",
        "2.BOT",
        "L.BOT",
        "R.BOT",
        ".TOP",
        ".ETC",
        "COL.BIN"
};

char sceneFilenameSuffixes[][13] = {
        ".ICN",
        ".SIC",
        "1.ALZ",
        "1.BLZ",
        "1.COL",
        "1.DAT",
        "2.ALZ",
        "2.BLZ",
        "2.COL",
        "3.ALZ",
        "3.BLZ",
        "3.COL",
        "4.ALZ",
        "4.BLZ",
        "4.COL",
        "5.ALZ",
        "5.BLZ",
        "5.COL",
        "A.DAT",
        "A.MUS",
        "B.DAT",
        "B.MUS",
        "C.DAT",
        "C.MUS",
        "D.DAT",
        "E.DAT",
        "O.BIN",
        "P.BIN",
        "T.BIN"
};



std::ifstream openTD3ExeForRead() {
    return openFileForRead("TD3.EXE");
}

PlayDisk loadPlayDisk() {
    auto fp = openFileForRead("PLAYDISK.DAT");
    fp.seekg(0xae); // num cars position
    uint8_t numCars;
    fp.read((char *)&numCars, 1);
    uint8_t numScenes;
    fp.read((char *)&numScenes, 1);

    PlayDisk playDisk;

    fp.seekg(0x12); // start of car name table.
    for (int i = 0; i < numCars; i++) {
        char name[6];
        fp.read(name, 6);
        playDisk.cars.emplace_back(name);
    }

    fp.seekg(0x66); // start of scene name table.
    for (int i = 0; i < numScenes; i++) {
        char name[8];
        fp.read(name, 8);
        playDisk.scenes.emplace_back(name);
    }
    fp.close();
    return playDisk;
}

std::vector<DataArchiveFileStruct> readFileInfoTbl(std::ifstream &fp, int startOffset, int numRecords) {
    std::vector<DataArchiveFileStruct> infoTable(numRecords);
    fp.seekg(startOffset);
    for (int i = 0; i < numRecords; i++) {
        fp.read((char *)&infoTable[i], sizeof(DataArchiveFileStruct));
    }
    return infoTable;
}



std::string getoutputFilename(const std::map<unsigned int, std::string> &filenames, const DataArchiveFileStruct &fileInfo) {
    auto matchedFilename = filenames.find(fileInfo.id);
    if (matchedFilename != filenames.end()) {
        return matchedFilename->second;
    } else {
        std::stringstream stream;
        stream << std::hex << fileInfo.id;
        return stream.str();
    }
}

void dumpFile(const std::map<unsigned int, std::string> &filenames, const DataArchiveFileStruct &fileInfo, const std::string &dataFilename) {
    std::ifstream archiveFile;
    unsigned char buf[0xffff];

    switch (fileInfo.archiveFileId) {
        case 'a' :  archiveFile = openFileForRead("DATAA.DAT"); break;
        case 'b' :  archiveFile = openFileForRead("DATAB.DAT"); break;
        case 'c' :  archiveFile = openFileForRead("DATAC.DAT"); break;
        case 'd' :
        case 'e' :  archiveFile = openFileForRead(dataFilename); break;
        default: return;
    }

    auto outputFilename = getoutputFilename(filenames, fileInfo);
    auto outFile = openFileForWrite(outputFilename);

    std::cout << "Extracting: " << outputFilename << "\n";

    archiveFile.seekg(fileInfo.offset);
    archiveFile.read((char *)buf, fileInfo.size - 1);
    outFile.write((char *)buf, fileInfo.size - 1);

    outFile.close();
    archiveFile.close();
}

int findOffsetOfFileInfoTable(std::ifstream &td3File) {
    int offset = 0;
    td3File.seekg(0, std::ios_base::end);
    int size = (int)td3File.tellg();

    for (; offset < size - 4; offset++) {
        int id = 0;
        td3File.seekg(offset);
        for (int i = 0; i < 4; i++) {
            char byte;
            td3File.read(&byte, 1);
            id += (int)byte << ((3 - i) * 8);
        }
        if (id == 0xEF0E4D4C) { // The id of the first entry in the table.
            return offset;
        }
    }

    std::cout << "Error: Failed to find start of FileInfoTable in TD3.EXE.\n";
    exit(1);
}

void dumpEngineFiles() {
    auto fp = openTD3ExeForRead();

    std::map<unsigned int, std::string> filenameIdMap;
    for( auto &filename : engineFilenames) {
        filenameIdMap[calcFilenameHash(filename)] = filename;
    }

    auto offset = findOffsetOfFileInfoTable(fp);
    auto fileInfoTable = readFileInfoTbl(fp, offset, 49);

    for (auto &fileInfo : fileInfoTable) {
        dumpFile(filenameIdMap, fileInfo, "");
    }
    fp.close();
}

void dumpCar(const std::string &carFilename) {
    auto listFile = openFileForRead(carFilename + ".LST");
    auto fileInfoTable = readFileInfoTbl(listFile, 0x1d1, 15);
    listFile.close();

    std::map<unsigned int, std::string> filenameIdMap;
    for( auto &suffix : carFilenameSuffixes) {
        auto filename = carFilename + suffix;
        filenameIdMap[calcFilenameHash(filename)] = filename;
    }

    for (auto &fileInfo : fileInfoTable) {
        dumpFile(filenameIdMap, fileInfo, carFilename + ".DAT");
    }
}

void dumpCarFiles(PlayDisk &playDisk) {
    for (auto &car : playDisk.cars) {
        dumpCar(car);
    }
}

void dumpScene(const std::string &sceneFilename) {
    auto listFile = openFileForRead(sceneFilename + ".LST");
    auto fileInfoTable = readFileInfoTbl(listFile, 0x4d0, 29);
    listFile.close();

    std::map<unsigned int, std::string> filenameIdMap;
    for( auto &suffix : sceneFilenameSuffixes) {
        auto filename = sceneFilename + suffix;
        filenameIdMap[calcFilenameHash(filename)] = filename;
    }

    for (auto &fileInfo : fileInfoTable) {
        dumpFile(filenameIdMap, fileInfo, sceneFilename + ".DAT");
    }
}

void dumpSceneFiles(PlayDisk &playDisk) {
    for (auto &scene : playDisk.scenes) {
        dumpScene(scene);
    }
}

void patchExe() {
    auto originalFile = openTD3ExeForRead();
    auto patchedFile = openFileForWrite("TD3_U.EXE");

    originalFile.seekg(0, std::ios_base::end);
    int size = (int)originalFile.tellg();

    char *buf = new char[size];
    originalFile.seekg(0);
    originalFile.read(buf, size);


    auto fileInfoTableOffset = findOffsetOfFileInfoTable(originalFile);

    std::cout << "Found offset of file info table at 0x" << std::hex << fileInfoTableOffset << "\n";
    std::cout << "Patching TD3.EXE -> TD3_U.EXE\n";

    /* Blanking out the ID in the first record of the table.
     * This makes the engine think that no files are packed in archive files.
     * It will then attempt to load the file directly from disk by its filename.
     */
    for (int i = 0; i < 4; i++) {
        buf[fileInfoTableOffset + i] = 0;
    }

    patchedFile.write(buf, size);

    originalFile.close();
    patchedFile.close();

    std::cout << "Done.\n";
    delete[] buf;
}

void printUsage(char **argv) {
    std::cout << "\nUsage: " << argv[0] << " option\n\n";
    std::cout << "Options:\n";
    std::cout << "  x - Extract files\n";
    std::cout << "  p - Patch TD3.EXE to use extracted files\n";
    std::cout << "  d inputLZFile outputFile - decompress LZW compressed file.\n\n";

    exit(1);
}

int main(int argc, char **argv) {
    std::map<unsigned int,std::string> fileNameMap;

    if (argc < 2) {
        printUsage(argv);
    }

    if (argv[1][0] == 'x') {
        auto playdisk = loadPlayDisk();
        dumpEngineFiles();
        dumpCarFiles(playdisk);
        dumpSceneFiles(playdisk);
    } else if (argv[1][0] == 'p') {
        patchExe();
    } else if (argv[1][0] == 'd' && argc >= 4) {
        LZWDecoder lzwDecoder;
        lzwDecoder.decode(argv[2], argv[3]);
    } else {
        printUsage(argv);
    }

    return 0;
}
