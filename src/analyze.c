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

void analyzeFile (const char *filename)
{
  AFfilehandle h = afOpenFile (filename, "r", NULL);
  if (h == AF_NULL_FILEHANDLE)
    {
      printf ("could not open \"%s\";\n", filename);
      return;
    }
  int rere;
  int format = afGetFileFormat(h, &rere);
  if (format == AF_FILE_UNKNOWN)
    {
      printf ("format of \"%s\" is not supported;\n", filename);
      return;
    }
  else
    {
      printf ("here it is: %d\n", format);
    }
  double sampleRate = afGetRate (h, AF_DEFAULT_TRACK);
  printf ("sample rate is: %f\n", sampleRate);
  afCloseFile (h);
}
