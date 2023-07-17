# TD3Extract
A tool for extracting data files from the game - Test Drive III "The Passion"

Test Drive III game files are packed in the following archive files.
`DATAA.DAT`, `DATAB.DAT`, `DATAC.DAT`, `C<CAR_ID>.DAT` and `SCENEnn.DAT`

This tool will extract the files back to their original filenames. It can also patch
the original game EXE to allow loading data from the unpacked files instead of the
archived `.DAT` files.

Usage
=====

```
    TD3Extract option

    Options:
      -extractFiles                          : Extract files
      -patchEXE                              : Patch TD3.EXE to use extracted files
      -decompressLZW inLZFile outFile        : Decompress LZW compressed file.
      -unpackRLE inFile outFile              : Decompress RLE compressed file.
      -extractImage inFile width paletteFile : Decompress packed game image file
                                               into PNG file.
      -encodeImage inFile outFile            : Compress PNG image into RLE+LZW
                                               encoded format for use by the game.
```

Engine File Formats
-------------------

More details on files used by the game can be found in the [/info](/info) directory.