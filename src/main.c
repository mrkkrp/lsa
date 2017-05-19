/*
 * This file is part of LSA.
 *
 * Copyright © 2014–2017 Mark Karpov
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

#include "lsa.h"

/* global variables */

long sep_pos,  /* this value is set from `main', it's index of the first
                  char of base name part of full name of file */
  items_total, /* total number of files found in target directory */
  prc_index; /* index of file to process */
struct dirent **items; /* these structures hold information about files in
                          target directory that are suitable for
                          processing */
pthread_mutex_t lock;  /* mutex lock */
extern int optind; /* index of the next element to be processed by `getopt*/
struct audio_params **outputs; /* vector of pointers to structures that
                                  contain descriptions for individual
                                  files */
int op_help, op_license, op_version, op_total, op_frames, op_kbps, op_peak,
  op_comp; /* command line options (flags) */

/* structures & constants */

struct option options[] = /* structures for getopt_long */
  { { "help"       , no_argument, &op_help   , 1 },
    { "license"    , no_argument, &op_license, 1 },
    { "version"    , no_argument, &op_version, 1 },
    { "total"      , no_argument, &op_total  , 1 },
    { "frames"     , no_argument, &op_frames , 1 },
    { "bitrate"    , no_argument, &op_kbps   , 1 },
    { "peak"       , no_argument, &op_peak   , 1 },
    { "compression", no_argument, &op_comp   , 1 },
    { NULL         , 0          , NULL       , 0 } };

const char *s_exts[] = /* extensions of supported file formats */
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

static void *run_thread (void *);
static const char *get_ext(const char *);
static int ext_filter (const struct dirent *);
static int cmpstrp (const void *, const void *);
static char decode_format (int);
static void decompose_time (double, int *, int *, int *);
static char *decode_comp (int);

/* main */

int main (int argc, char **argv)
{
  /* Before we do some serious stuff, we need to initialize mutex, we cannot
     work without it. */
  if (pthread_mutex_init(&lock, NULL) != 0)
    {
      fprintf (stderr, "lsa: cannot initialize pthread mutex, exit\n");
      return EXIT_FAILURE;
    }
  /* Now let's check if SSE and SSE2 are supported by CPU and OS. */
  if (!(__builtin_cpu_supports ("sse") &&
        __builtin_cpu_supports ("sse2")))
    {
      fprintf (stderr, "lsa: the CPU doesn't support SSE and SSE2\n");
      return EXIT_FAILURE;
    }
  /* First, we process command line options with `getopt_long', see
     documentation for this function to understand what's going on here. */
  int opt;
  while ((opt = getopt_long (argc, argv, "+tfbpc", options, NULL)) != -1)
    {
      switch (opt)
        {
        case 't' : op_total  = 1; break;
        case 'f' : op_frames = 1; break;
        case 'b' : op_kbps   = 1; break;
        case 'p' : op_peak   = 1; break;
        case 'c' : op_comp   = 1; break;
        }
    }
  /* Some options are informational by their nature and they cancel other
     options, so we just check if user wants to see some info and print it
     if it's the case. */
  if (op_help)
    {
      printf (LSA_HELP);
      return EXIT_SUCCESS;
    }
  if (op_license)
    {
      printf (LSA_LICENSE);
      return EXIT_SUCCESS;
    }
  if (op_version)
    {
      printf ("LSA %s, built %s %s\n", LSA_VERSION, __DATE__, __TIME__);
      return EXIT_SUCCESS;
    }
  /* Find out current working directory, max length of string to hold full
     file name, set `sep_pos'. */
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
      sep_pos = temp_len + 1;
      *(wdir + sep_pos) = '\0';
    }
  else sep_pos = temp_len;
  if (optind >= argc) free (temp);
  /* First of all, we should check if the given directory exists. */
  struct stat sb;
  if (!(stat (wdir, &sb) == 0 && S_ISDIR (sb.st_mode)))
    {
      fprintf (stderr,
               "lsa: '%s' does not exist or it's not a directory\n", wdir);
      free (wdir);
      return EXIT_FAILURE;
    }
  /* Scan working directory, save number of items we can process and items
     themselves in global variables. */
  items_total = scandir (wdir, &items, ext_filter, NULL);
  /* Allocate memory for vector of result structures. */
  outputs = malloc (sizeof (struct audioParams *) * items_total);
  /* Get number of cores, start a thread per core, every thread gets string
     with copy of working directory, the string should have enough space to
     `strcat' base names to it later. We also need to allocate enough space
     for a vector of all thread ids. */
  long ncores = sysconf (_SC_NPROCESSORS_ONLN);
  pthread_t *tidv = malloc (sizeof (pthread_t) * ncores);
  long i;
  for (i = 0; i < ncores; i++)
    {
      char *temp = malloc (sizeof (char) * wdir_len);
      strcpy (temp, wdir);
      pthread_create (tidv + i, NULL, run_thread, temp);
    }
  /* Wait for all threads to finish using vector of ids. Now free the
     vector and working directory, destroy mutex. */
  for (i = 0; i < ncores; i++)
    {
      pthread_join (*(tidv + i), NULL);
    }
  free (tidv);
  free (wdir);
  pthread_mutex_destroy(&lock);
  /* Now, it's time to sort our strings and print results. */
  qsort (outputs, items_total, sizeof (struct audioParams *), cmpstrp);
  /* Here we determine if we should display hours + some auxiliary
     calculations for `--total' option. */
  AFframecount total_frames = 0;
  char show_hours = 0;
  double total_dur = 0, total_kbps = 0, total_peak = 0;
  for (i = 0; i < items_total; i++)
    {
      struct audio_params *a = *(outputs + i);
      if (a->duration > 3600) show_hours = 1;
      total_dur += a->duration;
      total_frames += a->frames;
      total_kbps += a->kbps * a->duration;
      if (a->peak > total_peak) total_peak = a->peak;
    }
  if (op_total && total_dur > 3600) show_hours = 1;
  if (op_total && total_dur) total_kbps /= total_dur;
  /* Print header of our table. */
  printf ("rate   B  f # ");
  if (show_hours) printf ("hh:");
  printf ("mm:ss ");
  if (op_frames) printf ("frames     ");
  if (op_kbps) printf ("kbps ");
  if (op_peak) printf ("peak     ");
  if (op_comp) printf ("compression ");
  printf ("file\n");
  /* Print items and free output structures. */
  for (i = 0; i < items_total; i++)
    {
      struct audio_params *p = *(outputs + i);
      if (p)
        {
          int dur_h, dur_m, dur_s;
          decompose_time (p->duration, &dur_h, &dur_m, &dur_s);
          printf ("%6d %-2d %c %d ",
                  p->rate,
                  p->width,
                  decode_format (p->format),
                  p->channels);
          if (show_hours) printf ("%02d:", dur_h);
          printf ("%02d:%02d ", dur_m, dur_s);
          if (op_frames) printf ("%10ld ", p->frames);
          if (op_kbps) printf ("%4d ", (int)round(p->kbps));
          if (op_peak) printf ("%8f ", p->peak);
          if (op_comp) printf ("%11s ", decode_comp (p->compression));
          printf ("%s\n", p->name);
          free (p);
        }
    }
  /* Optionally print totals. */
  if (op_total)
    {
      int dur_h, dur_m, dur_s;
      decompose_time (total_dur, &dur_h, &dur_m, &dur_s);
      printf ("              ");
      if (show_hours) printf ("%02d:", dur_h);
      printf ("%02d:%02d ", dur_m, dur_s);
      if (op_frames) printf ("%10ld ", total_frames);
      if (op_kbps) printf ("%4d ", (int)round(total_kbps));
      if (op_peak) printf ("%8f ", total_peak);
      if (op_comp) printf ("            ");
      printf ("%ld file%s\n", items_total, items_total == 1 ? "" : "s");
    }
  /* Now that we're done displaying information, we can free vector of
     output structures (structures are already freed, see above). */
  free (outputs);
  /* Free directory items. */
  for (i = 0; i < items_total; i++)
    {
      free (*(items + i));
    }
  free (items);
  return EXIT_SUCCESS;
}

/* functions */

static void *run_thread (void *dir)
/* This function describes behavior of an individual thread. It takes new
   item from vector of items (if there's any), updates name of file `dir',
   calls function `analyzeFile' with this name and takes result of this
   call. Note that `analyzeFile' allocates memory for its result structure
   with `malloc'. Finally this routine copies pointer to result structure to
   `outputs'. */
{
  while (prc_index < items_total)
    {
      pthread_mutex_lock (&lock);
      long i = prc_index++;
      pthread_mutex_unlock (&lock);
      *((char *)dir + sep_pos) = '\0';
      strcat ((char *)dir, (**(items + i)).d_name);
      *(outputs + i) = analyze_file ((char *)dir);
      if (*(outputs + i))
        (**(outputs + i)).name = (**(items + i)).d_name;
    }
  free (dir);
  return NULL;
}

static const char *get_ext (const char *arg)
/* This function extracts extension from a file name. */
{
    const char *dot = strrchr(arg, '.');
    if (!dot || dot == arg) return "";
    return dot + 1;
}

static int ext_filter (const struct dirent *arg)
/* This function filters every item of type `struct dirent'. It only accepts
   regular files and links and those items must have one of supported
   extensions. */
{
  if (arg->d_type != DT_REG && arg->d_type != DT_LNK) return 0;
  const char *ext = get_ext (arg->d_name);
  unsigned int i;
  for (i = 0; i < (sizeof (s_exts) / sizeof (s_exts[0])); i++)
    {
      if (!strcmp (ext, *(s_exts + i))) return 1;
    }
  return 0;
}

static int cmpstrp (const void *a, const void *b)
/* This is wrapper around `strcmp' to sort output structures with `qsort'. */
{
  return strcmp((**(struct audio_params **)a).name,
                (**(struct audio_params **)b).name);
}

static char decode_format (int arg)
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

static void decompose_time (double arg, int *h, int *m, int *s)
/* Extract number of hours, minutes, and seconds from given total number of
   seconds. */
{
  double t = round(arg);
  *h = t / 3600; /* hours */
  t -= *h * 3600;
  *m = t / 60; /* minutes */
  t -= *m * 60;
  *s = t; /* seconds */
}

static char *decode_comp (int arg)
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
