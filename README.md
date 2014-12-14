# LSA - List Audio Files

NOTE: this software is not ready to use. This is just layout of README file.

This is minimal, lightweight, console, Unix-style program to list various
parameters of audio files. It can work with most audio-formats that are not
patented. It doesn't work with MPEG and some Apple formats for these are
patented audio formats and shouldn't be used at all.

## Features

* multithread: if several files are given, every file will be processed in
  separate thread.
* the program can list the following parameters of an audio file:
  * name of the file;
  * size of the file;
  * number of samples;
  * length in hours, minutes, and seconds;
  * type of compression;
  * format of samples;
  * number of channels;
  * bit width;
  * sample rate;
  * peaks (per channel).

## Installation

See `INSTALL.md`.

## TODO

* The following info should be also displayed:
  * replay gain;
  * perceived loudness;
  * true peaks (per channel);
  * dynamic range (per channel).
* Add configuration file, so users can change of parameters (?).

## License

Copyright (c) 2014 Mark Karpov

Distributed under GNU GRL.
