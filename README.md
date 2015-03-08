# LSA - List properties of audio files

```
[mark@arch 1978-Shiny-Beast-(Bat-Chain-Puller)]$ lsa -tbp
rate   B  f # mm:ss kbps peak     file
 44100 16 s 2 03:51  932 0.896606 01. The Floppy Boot Stomp.flac
 44100 16 s 2 04:48  958 0.818634 02. Tropical Hot Dog Night.flac
 44100 16 s 2 03:37  952 0.910550 03. Ice Rose.flac
 44100 16 s 2 03:42  826 0.868317 04. Harry Irene.flac
 44100 16 s 2 03:14  919 0.901852 05. You Know You're A Man.flac
 44100 16 s 2 05:27  883 0.953856 06. Bat Chain Puller.flac
 44100 16 s 2 05:03  876 1.000000 07. When I See Mommy I Feel Like A Mummy.flac
 44100 16 s 2 04:06  914 1.000000 08. Owed T'Alex.flac
 44100 16 s 2 03:24  883 0.936399 09. Candle Mambo.flac
 44100 16 s 2 05:03  785 0.833496 10. Love Lies.flac
 44100 16 s 2 04:25  886 0.963134 11. Suction Prints.flac
 44100 16 s 2 00:40  654 0.381744 12. Apes-Ma.flac
              47:25  872 1.000000 12 files
```

This is minimal, lightweight, console, Unix-style program to list various
parameters of audio files. It works like `ls` command displaying parameters
of files in current (or specified) directory. This program is written to
work with collection of files as a whole.

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
* duration in minutes, seconds, and hours (if necessary) per file;
* total duration of all files in actual directory;
* number of frames per file;
* total number of frames of all files in actual directory;
* bit-rate per file in kbps;
* average bit-rate for all files;
* peak [0..1] per file;
* maximum peak among all files in actual directory;
* compression scheme.

## Installation

See `INSTALL.md`.

## License

Copyright (c) 2014, 2015 Mark Karpov

Distributed under GNU GRL, version 3.
