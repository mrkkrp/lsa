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

struct audioParams *analyzeFile (char *path)
/* This is the place where all analyze happens. This is incomplete version
   of the function. TODO: finish it. */
{
  AFfilehandle h = afOpenFile ((const char *)path, "r", NULL);
  if (h == AF_NULL_FILEHANDLE) return NULL;
  struct audioParams *result = malloc (sizeof (*result));
  result->rate = (int)afGetRate (h, AF_DEFAULT_TRACK);
  afGetSampleFormat (h, AF_DEFAULT_TRACK, &result->format, &result->width);
  result->channels = afGetChannels (h, AF_DEFAULT_TRACK);
  result->frames = afGetFrameCount(h, AF_DEFAULT_TRACK);
  afCloseFile (h);
  return result;
}
