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

    options:
       x - Extract game files
       p - Patch TD3.EXE to use extracted files
```
