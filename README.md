# LSA - List properties of audio files

This is minimal, lightweight, console, Unix-style program to list various
parameters of audio files.

## Requirements

LSA requires CPU with SSE and SSE2 (this means that only if you have pretty
old CPU it won't work). Also, your OS must support these intrinsics.

## Supported Formats

LSA is built on top of Audio File library, so it inherits its list of
supported file formats:

* AIFF/AIFF-C (.aiff, .aifc)
* WAVE (.wav)
* NeXT .snd/Sun .au (.snd, .au)
* Berkeley/IRCAM/CARL Sound File (.sf)
* Audio Visual Research (.avr)
* Amiga IFF/8SVX (.iff)
* Sample Vision (.smp)
* Creative Voice File (.voc)
* NIST SPHERE (.wav)
* Core Audio Format (.caf)
* FLAC (.flac)

Supported compression formats:

* G.711 mu-law and A-law
* IMA ADPCM
* Microsoft ADPCM
* FLAC
* ALAC (Apple Lossless Audio Codec)

## Features

The program currently is capable to display the following parameters:

* sample rate;
* sample width;
* sample format (signed or unsigned integer, single or double precision
  floating point);
* number of channels;
* length in minutes, seconds, and hours (if necessary) per file;
* total length of all files in actual directory;
* number of frames per file;
* total number of frames of all files in actual directory;
* peak [0..1] per file;
* maximum peak among all files in actual directory;
* compression scheme.

## Installation

See `INSTALL.md`.

## TODO

* efficiently calculate perceived loudness (as per ITU-R BS.1770-3);
* efficiently calculate loudness range.

## License

Copyright (c) 2014 Mark Karpov

Distributed under GNU GRL.
