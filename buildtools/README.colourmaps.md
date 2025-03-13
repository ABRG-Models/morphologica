# Generating the colourmap lists

This is the process to generate morph/colourmaps_cet.h and morph/colourmaps_crameri.h.

## Crameri

Get ScientificColourMaps version 8.x from https://www.fabiocrameri.ch/colourmaps/ as ScientificColourMaps8.zip.

Unpack in a directory of your choice.

Compile process_colourtables.cpp in morphologica:

```bash
cd morphologica
mkdir -p build && cd build
cmake .. -DBUILD_UTILS=ON
make process_colourtables
```

Change directory to ScientificColourMaps8 and run process_colourtables. For example, I did:

```bash
cd ~/Downloads/ScientificColourMaps8/
~/src/morphologica/build/buildtools/process_colourtables
```

This generates the following output files:
```
colourmap_colourMapTypeToStr.cpp
colourmap_convert_switch.cpp
colourmap_enum.cpp
colourmap_example.cpp
colourmaps_crameri.h
colourmap_strToColourMapType.cpp
```
The file `colourmaps_crameri.h` goes in `morph/`. The other files need to be spliced into `morph/ColourMaps.h`.

## CET

The process is similar for CET, but the download is different.
From https://colorcet.com/download/index.html download the file `CETperceptual_csv_0_1.zip` which contains 'CSV floating point RGB values in the range 0-1'.
Unpack in a folder which will be called CETperceptual_csv_0_1.
Change directory into the folder and run process_colourtables.

```bash
cd ~/Downloads/CETperceptual_csv_0_1
~/src/morphologica/build/buildtools/process_colourtables
```
