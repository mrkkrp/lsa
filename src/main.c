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

/* structures & constants */

enum /* ids of flags */
  { OPT_DEFAULT = 0,
    OPT_HELP    = 1,
    OPT_LICENSE = 2,
    OPT_VERSION = 3 };

struct option options[] = /* structures for getopt_long */
  { { "help"   , no_argument, NULL, OPT_HELP    },
    { "license", no_argument, NULL, OPT_LICENSE },
    { "version", no_argument, NULL, OPT_VERSION },
    { NULL     , 0,           NULL, OPT_DEFAULT } };

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

/* global variables */

long sepPos,  /* this value is set from main, it's index of first char of
                 base name part of full name of file */
  itemsTotal, /* total number of items found in target directory */
  prcIndex = 0; /* index of file to process */
struct dirent **items; /* these structures hold information about files in
                          target directory that are suitable for processing */
pthread_mutex_t lock;  /* mutex lock */
struct audioParams **outputs; /* vector of pointers to structures that
                                 contain descriptions for individual
                                 files */

/* functions */

static void *runThread (void *dir)
/* This function describes behavior of an individual thread. It takes new
   item from vector of items (if there's any), updates name of file 'dir',
   calls function 'analyzeFile' with this name and takes result of this
   call. Note that 'analyzeFile' allocates memory for its result structure
   with 'malloc'. Finally this routine copies pointer to result string to
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
      if (*(outputs +i))
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
   regular files and links and those files must have one of supported
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

static int cmpStrp (const void *str0, const void *str1)
/* This is wrapper around 'strcmp' to sort strings with 'qsort'. */
{
  return strcmp((**(struct audioParams **)str0).name,
                (**(struct audioParams **)str1).name);
}

/* main */

int main (int argc, char **argv)
{
  /* First, we process flags with 'getopt_long', see documentation for this
     function to understand what's going on here. */
  int optIndex = getopt_long (argc, argv, "", options, NULL);
  if (optIndex == OPT_HELP)
    {
      printf (LSA_HELP);
      return EXIT_SUCCESS;
    }
  if (optIndex == OPT_LICENSE)
    {
      printf (LSA_LICENSE);
      return EXIT_SUCCESS;
    }
  if (optIndex == OPT_VERSION)
    {
      printf ("LSA %s, built %s %s\n", LSA_VERSION, __DATE__, __TIME__);
      return EXIT_SUCCESS;
    }
  /* Find out current working directory, max length of string for full file
     name, set 'sepPos'. */
  char *temp = argc >= 2 ? *(argv + 1) : getcwd (NULL, 0);
  long temp_len = strlen (temp);
  if (!temp_len)
    {
      free (temp);
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
  if (argc == 1) free (temp);
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
     with copy of working directory with enough space to 'strcat' base names
     later. We also need to allocate enough space for a vector of all thread
     ids. */
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
  /* Now, it's time to sort our strings and print results. */
  qsort (outputs, itemsTotal, sizeof (struct audioParams *), cmpStrp);
  printf ("%-6s %-2s file\n", "rate", "ch");
  for (i = 0; i < itemsTotal; i++)
    {
      struct audioParams *a = *(outputs + i);
      if (a)
        {
          printf ("%-6d %-2d %s\n",
                  a->rate,
                  a->channels,
                  a->name);
          free (a);
        }
      free (*(items + i));
    }
  free (outputs);
  free (items);
  free (wdir);
  return EXIT_SUCCESS;
}
