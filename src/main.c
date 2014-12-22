/*
 * LSA - program to list properties of audio files.
 *
 * Copyright (c) 2014 Mark Karpov
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "lsa.h"

/* global variables */

long sepPos,  /* this value is set from 'main', it's index of the first char
                 of base name part of full name of file */
  itemsTotal, /* total number of files found in target directory */
  prcIndex; /* index of file to process */
struct dirent **items; /* these structures hold information about files in
                          target directory that are suitable for
                          processing */
pthread_mutex_t lock;  /* mutex lock */
struct audioParams **outputs; /* vector of pointers to structures that
                                 contain descriptions for individual
                                 files */
int opHelp, opLicense, opVersion, opTotal, opFrames, opPeak, opPeaks,
  opCompression; /* command line options (flags) */

/* structures & constants */

struct option options[] = /* structures for getopt_long */
  { { "help"       , no_argument, &opHelp       , 1 },
    { "license"    , no_argument, &opLicense    , 1 },
    { "version"    , no_argument, &opVersion    , 1 },
    { "total"      , no_argument, &opTotal      , 1 },
    { "frames"     , no_argument, &opFrames     , 1 },
    { "peak"       , no_argument, &opPeak       , 1 },
    { "compression", no_argument, &opCompression, 1 },
    { NULL         , 0          , NULL          , 0 } };

const char *sExts[] = /* extensions of supported file formats */
  { "aiff",  /* AIFF */
    "aiffc", /* AIFF-C */
    "wav",   /* WAVE */
    "snd",   /* NeXT .snd */
    "au",    /* Sun .au */
    "sf",    /* Berkley/IRCAM/CARL Sound File */
    "avr",   /* Audio Visual Research */
    "iff",   /* Amiga IFF/8SVX */
    "smp",   /* Sample Vision */
    "voc",   /* Creative Voice File */
    "caf",   /* Core Audio Format */
    "flac"   /* Free Lossless Audio Code */ };

/* declarations */

static void *runThread (void *);
static const char *getExt(const char *);
static int extFilter (const struct dirent *);
static int cmpStrp (const void *, const void *);
static char decodeFormat (int);
static void decomposeTime (long, int *, int *, int *);
static char *decodeCompression (int);

/* main */

int main (int argc, char **argv)
{
  /* First, we process command line options with 'getopt_long', see
     documentation for this function to understand what's going on here. */
  int opt;
  while ((opt = getopt_long (argc, argv, "+tfpc", options, NULL)) != -1)
    {
      switch (opt)
        {
        case 't' : opTotal       = 1; break;
        case 'f' : opFrames      = 1; break;
        case 'p' : opPeak        = 1; break;
        case 'c' : opCompression = 1; break;
        }
    }
  /* Some options are informational by their nature and they cancel other
     options, so we just check if user wants to see some info and print it
     if it's the case. */
  if (opHelp)
    {
      printf (LSA_HELP);
      return EXIT_SUCCESS;
    }
  if (opLicense)
    {
      printf (LSA_LICENSE);
      return EXIT_SUCCESS;
    }
  if (opVersion)
    {
      printf ("LSA %s, built %s %s\n", LSA_VERSION, __DATE__, __TIME__);
      return EXIT_SUCCESS;
    }
  /* Find out current working directory, max length of string to hold full
     file name, set 'sepPos'. */
  char *temp = optind < argc ? *(argv + optind) : getcwd (NULL, 0);
  long temp_len = strlen (temp);
  if (!temp_len)
    {
      if (optind >= argc) free (temp);
      return EXIT_FAILURE;
    }
  long wdir_len = sizeof (char) * (temp_len + BASENAME_MAX_LEN + 2);
  /* We add 2: one byte for terminating char and one for possible '/'. */
  char *wdir = malloc (wdir_len);
  strcpy (wdir, temp);
  if (*(temp + temp_len - 1) != '/')
    {
      *(wdir + temp_len) = '/';
      sepPos = temp_len + 1;
      *(wdir + sepPos) = '\0';
    }
  else sepPos = temp_len;
  if (optind >= argc) free (temp);
  /* First of all, we should check if the given directory exist. */
  struct stat sb;
  if (!(stat (wdir, &sb) == 0 && S_ISDIR (sb.st_mode)))
    {
      fprintf (stderr, "'%s' does not exist or it's not a directory\n", wdir);
      free (wdir);
      return EXIT_FAILURE;
    }
  /* Before we do some serious stuff, we need to initialize mutex, we cannot
     work without it. */
  if (pthread_mutex_init(&lock, NULL) != 0)
    {
      free (wdir);
      return EXIT_FAILURE;
    }
  /* Scan working directory, save number of items we can process and items
     themselves in global variables. */
  itemsTotal = scandir (wdir, &items, extFilter, NULL);
  /* Allocate memory for vector of result structures. */
  outputs = malloc (sizeof (struct audioParams *) * itemsTotal);
  /* Get number of cores, start a thread per core, every thread gets string
     with copy of working directory, the string should have enough space to
     'strcat' base names to it later. We also need to allocate enough space
     for a vector of all thread ids. */
  long ncores = sysconf (_SC_NPROCESSORS_ONLN);
  pthread_t *tidv = malloc (sizeof (pthread_t) * ncores);
  long i;
  for (i = 0; i < ncores; i++)
    {
      char *temp = malloc (sizeof (char) * wdir_len);
      strcpy (temp, wdir);
      pthread_create (tidv + i, NULL, runThread, temp);
    }
  /* Wait for all threads to finish using vector of ids. Now free the
     vector, free directory items, and entire vector of these items. Free
     original working directory. Also, destroy mutex. */
  for (i = 0; i < ncores; i++)
    {
      pthread_join (*(tidv + i), NULL);
    }
  pthread_mutex_destroy(&lock);
  free (tidv);
  free (wdir);
  /* Now, it's time to sort our strings and print results. */
  qsort (outputs, itemsTotal, sizeof (struct audioParams *), cmpStrp);
  /* Here we determine if we should display hours + some auxiliary
     calculations for '--total' option. */
  char showHours = 0;
  double totalSecs = 0;
  AFframecount totalFrames = 0;
  float totalPeak = 0;
  for (i = 0; i < itemsTotal; i++)
    {
      struct audioParams *a = *(outputs + i);
      if (a->frames / a->rate > 3600) showHours = 1;
      totalSecs += (double)a->frames / a->rate;
      totalFrames += a->frames;
      if (a->peak > totalPeak) totalPeak = a->peak;
    }
  if (opTotal && totalSecs > 3600) showHours = 1;
  /* Print header of our table. */
  printf ("rate   B  f # ");
  if (showHours) printf ("hh:");
  printf ("mm:ss ");
  if (opFrames) printf ("frames     ");
  if (opPeak) printf ("peak     ");
  if (opCompression) printf ("compression ");
  printf ("file\n");
  /* Print items and free output structures. */
  for (i = 0; i < itemsTotal; i++)
    {
      struct audioParams *p = *(outputs + i);
      if (p)
        {
          int dur_h, dur_m, dur_s;
          decomposeTime(p->frames / p->rate, &dur_h, &dur_m, &dur_s);
          printf ("%6d %-2d %c %d ",
                  p->rate,
                  p->width,
                  decodeFormat (p->format),
                  p->channels);
          if (showHours) printf ("%02d:", dur_h);
          printf ("%02d:%02d ", dur_m, dur_s);
          if (opFrames) printf ("%10ld ", p->frames);
          if (opPeak)
            printf ("%8f ", p->peak);
          if (opCompression)
            printf ("%11s ", decodeCompression (p->compression));
          printf ("%s\n", p->name);
          free (p);
        }
    }
  /* Print totals (optionally). */
  if (opTotal)
    {
      int dur_h, dur_m, dur_s;
      decomposeTime ((int)totalSecs, &dur_h, &dur_m, &dur_s);
      printf ("              ");
      if (showHours) printf ("%02d:", dur_h);
      printf ("%02d:%02d ", dur_m, dur_s);
      if (opFrames) printf ("%10ld ", totalFrames);
      if (opPeak) printf ("%8f ", totalPeak);
      if (opCompression) printf ("            ");
      printf ("%ld files\n", itemsTotal);
    }
  free (outputs);
  /* Free items, freedom must be given to everyone. */
  for (i = 0; i < itemsTotal; i++)
    {
      free (*(items + i));
    }
  free (items);
  return EXIT_SUCCESS;
}

/* functions */

static void *runThread (void *dir)
/* This function describes behavior of an individual thread. It takes new
   item from vector of items (if there's any), updates name of file 'dir',
   calls function 'analyzeFile' with this name and takes result of this
   call. Note that 'analyzeFile' allocates memory for its result structure
   with 'malloc'. Finally this routine copies pointer to result structure to
   'outputs'. */
{
  while (prcIndex < itemsTotal)
    {
      pthread_mutex_lock (&lock);
      long i = prcIndex++;
      pthread_mutex_unlock (&lock);
      *((char *)dir + sepPos) = '\0';
      strcat ((char *)dir, (**(items + i)).d_name);
      *(outputs + i) = analyzeFile ((char *)dir);
      if (*(outputs + i))
        (**(outputs + i)).name = (**(items + i)).d_name;
    }
  free (dir);
  return NULL;
}

static const char *getExt(const char *arg)
/* This function extracts extension from a file name. */
{
    const char *dot = strrchr(arg, '.');
    if (!dot || dot == arg) return "";
    return dot + 1;
}

static int extFilter (const struct dirent *arg)
/* This function filters every item of type 'struct dirent'. It only accepts
   regular files and links and those items must have one of supported
   extensions. */
{
  if (arg->d_type != DT_REG && arg->d_type != DT_LNK) return 0;
  const char *ext = getExt (arg->d_name);
  unsigned int i;
  for (i = 0; i < (sizeof (sExts) / sizeof (sExts[0])); i++)
    {
      if (!strcmp (ext, *(sExts + i))) return 1;
    }
  return 0;
}

static int cmpStrp (const void *a, const void *b)
/* This is wrapper around 'strcmp' to sort output structures with 'qsort'. */
{
  return strcmp((**(struct audioParams **)a).name,
                (**(struct audioParams **)b).name);
}

static char decodeFormat (int arg)
/* The function returns one letter corresponding to code of sample format. */
{
  switch (arg)
    {
    case AF_SAMPFMT_TWOSCOMP : return 's';
    case AF_SAMPFMT_UNSIGNED : return 'u';
    case AF_SAMPFMT_FLOAT    : return 'f';
    case AF_SAMPFMT_DOUBLE   : return 'd';
    }
  return '?';
}

static void decomposeTime (long arg, int *h, int *m, int *s)
/* Extract number of hours, minutes, and seconds from given total number of
   seconds. */
{
  *h = arg / 3600; /* hours */
  arg -= *h * 3600;
  *m = arg / 60; /* minutes */
  arg -= *m * 60;
  *s = arg; /* seconds */
}

static char *decodeCompression (int arg)
/* Return name of compression scheme by its code. */
{
  switch (arg)
    {
    case AF_COMPRESSION_UNKNOWN   : return "unknown";
    case AF_COMPRESSION_NONE      : return "none";
    case AF_COMPRESSION_G722      : return "G.722";
    case AF_COMPRESSION_G711_ULAW : return "G.711 u-law";
    case AF_COMPRESSION_G711_ALAW : return "G.711 a-law";
    case AF_COMPRESSION_G726      : return "G.726";
    case AF_COMPRESSION_G728      : return "G.728";
    case AF_COMPRESSION_DVI_AUDIO : return "DVI audio";
    case AF_COMPRESSION_GSM       : return "GSM";
    case AF_COMPRESSION_FS1016    : return "FS-1016";
    case AF_COMPRESSION_DV        : return "DV";
    case AF_COMPRESSION_MS_ADPCM  : return "MS ADPCM";
    case AF_COMPRESSION_FLAC      : return "FLAC";
    case AF_COMPRESSION_ALAC      : return "ALAC";
    }
  return "unknown";
}
