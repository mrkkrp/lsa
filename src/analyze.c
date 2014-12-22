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

#define ABS(X) (X >= 0 ? X : -X)

static float getPeak (void *frames, AFframecount c, int format, int width)
/* Calculate peak. */
{
  AFframecount i;
  if (format == AF_SAMPFMT_TWOSCOMP)
    {
      int32_t temp = 0;
      if (width > 16)
        {
          for (i = 0; i < c; i++)
            {
              int32_t x = ABS(*((int32_t *)frames + i));
              if (x > temp) temp = x;
            }
        }
      else if (width > 8)
        {
          for (i = 0; i < c; i++)
            {
              int16_t x = ABS(*((int16_t *)frames + i));
              if (x > temp) temp = x;
            }
        }
      else if (width > 0)
        {
          for (i = 0; i < c; i++)
            {
              int8_t x = ABS(*((int8_t *)frames + i));
              if (x > temp) temp = x;
            }
        }
      return (float)temp / ((1 << (width - 1)) - 1);
    }
  if (format == AF_SAMPFMT_UNSIGNED)
    {
      uint32_t temp = 0;
      if (width > 16)
        {
          for (i = 0; i < c; i++)
            {
              uint32_t x = *((uint32_t *)frames + i);
              if (x > temp) temp = x;
            }
        }
      else if (width > 8)
        {
          for (i = 0; i < c; i++)
            {
              uint16_t x = *((uint16_t *)frames + i);
              if (x > temp) temp = x;
            }
        }
      else if (width > 0)
        {
          for (i = 0; i < c; i++)
            {
              uint8_t x = *((uint8_t *)frames + i);
              if (x > temp) temp = x;
            }
        }
      return (float)temp / ((1 << width) - 1);
    }
  if (format == AF_SAMPFMT_FLOAT)
    {
      float temp = 0;
      for (i = 0; i < c; i++)
        {
          float x = ABS(*((float *)frames + i));
          if (x > temp) temp = x;
        }
      return temp;
    }
  if (format == AF_SAMPFMT_DOUBLE)
    {
      double temp = 0;
      for (i = 0; i < c; i++)
        {
          double x = ABS(*((double *)frames + i));
          if (x > temp) temp = x;
        }
      return (float)temp;
    }
  return 0;
}

struct audioParams *analyzeFile (char *path)
/* This is the place where all the analyze happens. We take 'path', open
   file on this path with AudioFile library, allocate memory for 'struct
   audioParams', assign calculated values and return pointer to this
   structure. If we return 'NULL', this item will be ignored. */
{
  AFfilehandle h = afOpenFile ((const char *)path, "r", NULL);
  if (h == AF_NULL_FILEHANDLE) return NULL;
  struct audioParams *result = malloc (sizeof (*result));
  result->rate = (int)afGetRate (h, AF_DEFAULT_TRACK);
  afGetSampleFormat (h, AF_DEFAULT_TRACK, &result->format, &result->width);
  result->channels = afGetChannels (h, AF_DEFAULT_TRACK);
  result->frames = afGetFrameCount (h, AF_DEFAULT_TRACK);
  if (opCompression)
    result->compression = afGetCompression (h, AF_DEFAULT_TRACK);
  void *frames = NULL;
  if (opPeak)
    {
      posix_memalign (&frames,
                      16,
                      (result->width / 8) * result->frames * result-> channels);
      AFframecount c =
        afReadFrames (h, AF_DEFAULT_TRACK, frames, result->frames);
      if (c == result->frames)
        {
          if (opPeak)
            result->peak = getPeak (frames,
                                    c * result->channels,
                                    result->format,
                                    result->width);
        }
      free (frames);
    }
  afCloseFile (h);
  return result;
}
