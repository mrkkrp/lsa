# LSA - List properties of audio files

```
[mark@arch 1978-Shiny-Beast-(Bat-Chain-Puller)]$ lsa -tbp
rate   B  f # mm:ss kbps peak     file
 44100 16 s 2 03:51  932 0.896606 01. The Floppy Boot Stomp.flac
 44100 16 s 2 04:49  959 0.818634 02. Tropical Hot Dog Night.flac
 44100 16 s 2 03:38  952 0.910522 03. Ice Rose.flac
 44100 16 s 2 03:43  827 0.868317 04. Harry Irene.flac
 44100 16 s 2 03:14  919 0.901825 05. You Know You're A Man.flac
 44100 16 s 2 05:27  883 0.953827 06. Bat Chain Puller.flac
 44100 16 s 2 05:04  877 1.000000 07. When I See Mommy I Feel Like A Mummy.flac
 44100 16 s 2 04:07  915 1.000000 08. Owed T'Alex.flac
 44100 16 s 2 03:24  884 0.936371 09. Candle Mambo.flac
 44100 16 s 2 05:03  785 0.833496 10. Love Lies.flac
 44100 16 s 2 04:25  886 0.963104 11. Suction Prints.flac
 44100 16 s 2 00:40  654 0.381744 12. Apes-Ma.flac
              47:25  887 1.000000 12 files
```

This is a minimal, lightweight, console program to list various parameters
of audio files. It works like the `ls` command displaying parameters of
files in current (or specified) directory. This program is written to work
with collection of files as a whole.

## Requirements

LSA requires CPU with SSE and SSE2 (this means that only if you have pretty
old CPU it won't work). Also, your OS must support these intrinsics.

## Supported Formats

LSA is built on top of the Audio File library, and so it inherits its list
of supported file formats:

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

The program is currently capable of displing the following parameters:

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

1. Install Audio File library;

2. Chances are you already have GCC and GNU make, but if you don't, get them;

3. Download and untar git repository of LSA, or clone it:

   ```
   $ git clone https://github.com/mrkkrp/lsa.git
   ```

4. Go to the root directory of the repository and execute:

   ```
   $ make
   # sh install.sh
   ```

5. Done (you can use `uninstall.sh` to uninstall the program).

## License

Copyright © 2014–2017 Mark Karpov

Distributed under GNU GRL, version 3.
