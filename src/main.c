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

#define LSA_VERSION "0.1.0"
#define LSA_LICENSE "LSA - List properties of audio files.\n\n"         \
  "Copyright (c) 2014 Mark Karpov\n\n"                                  \
  "This program is free software: you can redistribute it and/or modify\n" \
  "it under the terms of the GNU General Public License as published by\n" \
  "the Free Software Foundation, either version 3 of the License, or\n" \
  "(at your option) any later version.\n\n"                             \
  "This program is distributed in the hope that it will be useful,\n"   \
  "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"    \
  "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"     \
  "GNU General Public License for more details.\n\n"                    \
  "You should have received a copy of the GNU General Public License\n" \
  "along with this program.  If not, see <http://www.gnu.org/licenses/>.\n"

enum
  { OPT_DEFAULT = 0,
    OPT_VERSION = 1,
    OPT_LICENSE = 2,
    OPT_HELP    = 3,
    OPT_FORMATS = 4 };

struct option options[] =
  { { "version", no_argument, NULL, OPT_VERSION },
    { "license", no_argument, NULL, OPT_LICENSE },
    { "help"   , no_argument, NULL, OPT_HELP },
    { "formats", no_argument, NULL, OPT_FORMATS },
    { NULL     , 0,           NULL, OPT_DEFAULT } };

int main (int argc, char **argv)
{
  int optIndex = getopt_long (argc, argv, "", options, NULL);
  if (optIndex == OPT_VERSION)
    {
      printf ("LSA %s, built %s %s\n", LSA_VERSION, __DATE__, __TIME__);
      return EXIT_SUCCESS;
    }
  if (optIndex == OPT_LICENSE)
    {
      printf (LSA_LICENSE);
      return EXIT_SUCCESS;
    }
  if (optIndex == OPT_HELP)
    {
      printf ("help blah;\n");
      return EXIT_SUCCESS;
    }
  if (optIndex == OPT_FORMATS)
    {
      printf ("formats blah;\n");
      return EXIT_SUCCESS;
    }
  int i;
  for (i = 1; i < argc; i++)
    {
      pthread_t tid;
      pthread_create (&tid, NULL, analyzeFile, *(argv + i));
      pthread_join (tid, NULL);
    }
  return EXIT_SUCCESS;
}
