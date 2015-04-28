/*
 * This file is part of LSA.
 *
 * Copyright © 2014, 2015 Mark Karpov
 *
 * LSA is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * LSA is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LSA_H
#define LSA_H

#include <stdlib.h>    /* standard stuff */
#include <stdio.h>     /* printf */
#include <stdint.h>    /* intN_t things */
#include <getopt.h>    /* getopt_long */
#include <math.h>      /* round */
#include <sys/stat.h>  /* stat */
#include <dirent.h>    /* scan directories */
#include <unistd.h>    /* getcwd, sysconf */
#include <string.h>    /* strcpy, strcat */
#include <audiofile.h> /* http://www.68k.org/~michael/audiofile/ */
#include <pthread.h>   /* create and manage posix threads */
#include <xmmintrin.h> /* for SSE intrinsics */
#include <emmintrin.h> /* for SSE2 intrinsics */

/* some definitions */

#define LSA_VERSION "0.1.1"
#define LSA_LICENSE "LSA — List properties of audio files.\n\n"       \
  "Copyright © 2014, 2015 Mark Karpov\n\n"                            \
  "LSA is free software: you can redistribute it and/or modify it under the\n" \
  "terms of the GNU General Public License as published by the Free\n"  \
  "Software Foundation, either version 3 of the License, or (at your\n" \
  "option) any later version.\n\n"                                      \
  "LSA is distributed in the hope that it will be useful, but WITHOUT ANY\n" \
  "WARRANTY; without even the implied warranty of MERCHANTABILITY or\n" \
  "FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License\n" \
  "for more details.\n\n"                                               \
  "You should have received a copy of the GNU General Public License\n" \
  "along with this program. If not, see <http://www.gnu.org/licenses/>.\n"
#define LSA_HELP "lsa — list properties of audio files\n\n"             \
  "Usage: lsa [OPTIONS] [DIRECTORY]\n\n"                                \
  "Available options:\n"                                                \
  "  --help                  Show this help text\n"                     \
  "  --license               Show license of the program\n"             \
  "  --version               Show version of the program\n"             \
  "  -t,--total              Show totals of some parameters\n"          \
  "  -f,--frames             Show number of frames per file\n"          \
  "  -b,--bitrate            Show bitrate per file\n"                   \
  "  -p,--peak               Show peak per file\n"                      \
  "  -c,--compression        Show compression scheme per file\n"

#define BASENAME_MAX_LEN     256 /* according to definition of `d_name'
                                    field in `struct dirent' */

/* structures */

struct audio_params /* this structure contains various parameters of files
                       that have been analyzed */
{
  AFframecount frames;
  char *name;
  double duration;
  double kbps;
  double peak;
  int channels;
  int compression;
  int format;
  int rate;
  int width;
};

/* some declarations */

extern int op_peak, op_peaks, op_comp;
struct audio_params *analyze_file (char *);

#endif /* LSA_H */
