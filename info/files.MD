Images
------
Images are RLE encoded and compressed with LZW. There are no dimensions stored
with the image data. Image widths and in some cases packed RLE size is hardcoded
in the EXE. :-(  

Car Images
----------

All car image files have the Car ID suffix. eg. `CDIAB`

| filename | width | palette file | description                        |
|----------|-------|--------------|------------------------------------|
| *.TOP    | 320   | *COL.BIN     | Top of in car HUD gfx              |
| *1.BOT   | 320   | *COL.BIN     | Dash image top                     |
| *2.BOT   | 320   | *COL.BIN     | Dash image bottom                  |
| *L.BOT   | 168   | *COL.BIN     | Dash Steering wheel rotated left   |
| *R.BOT   | 168   | *COL.BIN     | Dash Steering wheel rotated right  |
| *.ETC    | 72    | *COL.BIN     | Gear stick. wheel center indicator |
| *FL1.LZ  | 208   | *SC.BIN      | Statistics frame + initial stats   |
| *FL2.LZ  | 208   | *SC.BIN      | Additional statistics              |
| *.BIC    | 112   | *SC.BIN      | Car select image. Top left         |
| *.ICN    | 208   | *SC.BIN      | Car select image. Top right        |
| *.SID    | 112   | *SC.BIN      | Car select image. Bottom left      |
| *.SIC    | 72    | *SIC.BIN ^1  | Selected car icon on main menu     |

`^1`: This palette is appended to end of main menu palette.