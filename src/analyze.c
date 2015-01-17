/*
 * This file is part of LSA.
 *
 * Copyright (c) 2014 Mark Karpov
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

/* declarations */

static float get_peak    (void *, AFframecount, int, int);
static float peak_int32  (void *, AFframecount);
static float peak_int16  (void *, AFframecount);
static float peak_int8   (void *, AFframecount);
static float peak_uint32 (void *, AFframecount);
static float peak_uint16 (void *, AFframecount);
static float peak_uint8  (void *, AFframecount);
static float peak_float  (void *, AFframecount);
static float peak_double (void *, AFframecount);

/* definitions */

struct audio_params *analyze_file (char *path)
/* This is the place where all the analyze happens. We take 'path', open
   file on this path with AudioFile library, allocate memory for 'struct
   audioParams', assign calculated values and return pointer to this
   structure. If we return 'NULL', this item will be ignored. */
{
  AFfilehandle h = afOpenFile ((const char *)path, "r", NULL);
  if (h == AF_NULL_FILEHANDLE) return NULL;
  struct audio_params *result = malloc (sizeof (*result));
  result->rate = (int)afGetRate (h, AF_DEFAULT_TRACK);
  afGetSampleFormat (h, AF_DEFAULT_TRACK, &result->format, &result->width);
  result->channels = afGetChannels (h, AF_DEFAULT_TRACK);
  result->frames = afGetFrameCount (h, AF_DEFAULT_TRACK);
  if (op_comp)
    result->compression = afGetCompression (h, AF_DEFAULT_TRACK);
  if (op_peak) /* check if any options that requires calculations on frames
                  are supplied */
    {
      void *frames = NULL;
      /* Calculate size of the buffer and allocate aligned memory. */
      AFframecount count;
      if (result->width > 32) count = 8;
      else if (result->width > 16) count = 4;
      else if (result->width > 8) count = 2;
      else count = 1;
      count *= result->frames * result->channels;
      if (posix_memalign (&frames, 16, count))
        {
          fprintf (stderr,
                   "lsa: cannot dynamically allocate aligned memory\n");
          return result;
        }
      AFframecount c =
        afReadFrames (h, AF_DEFAULT_TRACK, frames, result->frames);
      if (c == result->frames)
        {
          if (op_peak)
            result->peak = get_peak (frames,
                                     c * result->channels,
                                     result->format,
                                     result->width);
        }
      free (frames);
    }
  afCloseFile (h);
  return result;
}

static float get_peak (void *frames, AFframecount c, int format, int width)
{
  if (format == AF_SAMPFMT_TWOSCOMP)
    {
      if (width > 16) return peak_int32 (frames, c);
      else if (width > 8) return peak_int16 (frames, c);
      else return peak_int8 (frames, c);
    }
  else if (format == AF_SAMPFMT_UNSIGNED)
    {
      if (width > 16) return peak_uint32 (frames, c);
      else if (width > 8) return peak_uint16 (frames, c);
      else return peak_uint8 (frames, c);
    }
  else if (format == AF_SAMPFMT_FLOAT) return peak_float (frames, c);
  else if (format == AF_SAMPFMT_DOUBLE) return peak_double (frames, c);
  return 0;
}

static float peak_int32 (void *frames, AFframecount c)
{
  /* Doing trivial things here, because we use only SSE and SSE2 here, but
     necessary instructions for this format of samples are only in
     SSE4.1. */
  register AFframecount i;
  int32_t tmp0 = 0, tmp1 = 0;
  for (i = 0; i < c; i++)
    {
      int32_t a = *((int32_t *)frames + i);
      if (a > tmp0) tmp0 = a;
      if (a < tmp1) tmp1 = a;
    }
  float tp0 = tmp0;
  tp0 /= tmp0 < 0 ? -0x80000000 : 0x7fffffff;
  float tp1 = tmp1;
  tp1 /= tmp1 < 0 ? -0x80000000 : 0x7fffffff;
  return tp0 > tp1 ? tp0 : tp1;
}

static float peak_int16 (void *frames, AFframecount c)
{
  AFframecount t = c / 8;
  __m128i *src = (__m128i *)frames;
  __m128i m0 = _mm_set1_epi16 (0);
  __m128i m1 = _mm_set1_epi16 (0);
  register AFframecount i;
  for (i = 0; i < t; i++, src++)
    {
      m0 = _mm_max_epi16 (m0, *src);
      m1 = _mm_min_epi16 (m1, *src);
    }
  union u
  {
    __m128i m;
    int16_t n[8];
  } mx0, mx1;
  mx0.m = m0;
  mx1.m = m1;
  float tmp0 = 0, tmp1 = 0;
  for (i = 0; i < 8; i++)
    {
      int16_t a = *(mx0.n + i);
      if (a > tmp0) tmp0 = a;
      int16_t b = *(mx1.n + i);
      if (b < tmp1) tmp1 = b;
    }
  for (i = t * 8; i < c; i++)
    {
      int16_t a = *((int16_t *)frames + i);
      if (a > tmp0) tmp0 = a;
      if (a < tmp1) tmp1 = a;
    }
  tmp0 /= tmp0 < 0 ? -0x8000 : 0x7fff;
  tmp1 /= tmp1 < 0 ? -0x8000 : 0x7fff;
  return tmp0 > tmp1 ? tmp0 : tmp1;
}

static float peak_int8 (void *frames, AFframecount c)
{
  /* need SSE4.1 */
  register AFframecount i;
  int8_t tmp0 = 0, tmp1 = 0;
  for (i = 0; i < c; i++)
    {
      int8_t a = *((int8_t *)frames + i);
      if (a > tmp0) tmp0 = a;
      if (a < tmp1) tmp1 = a;
    }
  float tp0 = tmp0;
  tp0 /= tmp0 < 0 ? -0x80 : 0x7f;
  float tp1 = tmp1;
  tp1 /= tmp1 < 0 ? -0x80 : 0x7f;
  return tp0 > tp1 ? tp0 : tp1;
}

static float peak_uint32 (void *frames, AFframecount c)
{
  /* need SSE4.1 */
  register AFframecount i;
  uint32_t tmp0 = 0;
  for (i = 0; i < c; i++)
    {
      uint32_t a = *((uint32_t *)frames + i);
      if (a > tmp0) tmp0 = a;
    }
  float tp0 = tmp0;
  return tp0 / 0xffffffff;
}

static float peak_uint16 (void *frames, AFframecount c)
{
  /* need SSE4.1 */
  register AFframecount i;
  uint16_t tmp0 = 0;
  for (i = 0; i < c; i++)
    {
      uint16_t a = *((uint16_t *)frames + i);
      if (a > tmp0) tmp0 = a;
    }
  float tp0 = tmp0;
  return tp0 / 0xffff;
}

static float peak_uint8 (void *frames, AFframecount c)
{
  AFframecount t = c / 16;
  __m128i *src = (__m128i *)frames;
  __m128i m0 = _mm_set1_epi8 (0);
  register AFframecount i;
  for (i = 0; i < t; i++, src++)
    {
      m0 = _mm_max_epu8 (m0, *src);
    }
  union u
  {
    __m128i m;
    uint8_t n[16];
  } mx0;
  mx0.m = m0;
  float tmp0 = 0;
  for (i = 0; i < 8; i++)
    {
      uint8_t a = *(mx0.n + i);
      if (a > tmp0) tmp0 = a;
    }
  for (i = t * 8; i < c; i++)
    {
      uint8_t a = *((uint8_t *)frames + i);
      if (a > tmp0) tmp0 = a;
    }
  return tmp0 / 0xff;
}

static float peak_float (void *frames, AFframecount c)
{
  AFframecount t = c / 4;
  __m128 *src = (__m128 *)frames;
  __m128 m0 = _mm_set1_ps (0);
  __m128 m1 = _mm_set1_ps (0);
  register AFframecount i;
  for (i = 0; i < t; i++, src++)
    {
      m0 = _mm_max_ps (m0, *src);
      m1 = _mm_min_ps (m1, *src);
    }
  union u
  {
    __m128 m;
    float n[4];
  } mx0, mx1;
  mx0.m = m0;
  mx1.m = m1;
  float tmp0 = 0, tmp1 = 0;
  for (i = 0; i < 8; i++)
    {
      float a = *(mx0.n + i);
      if (a > tmp0) tmp0 = a;
      float b = *(mx1.n + i);
      if (b < tmp1) tmp1 = b;
    }
  for (i = t * 8; i < c; i++)
    {
      float a = *((float *)frames + i);
      if (a > tmp0) tmp0 = a;
      if (a < tmp1) tmp1 = a;
    }
  if (tmp0 < 0) tmp0 *= -1;
  if (tmp1 < 0) tmp1 *= -1;
  return tmp0 > tmp1 ? tmp0 : tmp1;
}

static float peak_double (void *frames, AFframecount c)
{
  AFframecount t = c / 2;
  __m128d *src = (__m128d *)frames;
  __m128d m0 = _mm_set1_pd (0);
  __m128d m1 = _mm_set1_pd (0);
  register AFframecount i;
  for (i = 0; i < t; i++, src++)
    {
      m0 = _mm_max_pd (m0, *src);
      m1 = _mm_min_pd (m1, *src);
    }
  union u
  {
    __m128d m;
    double n[2];
  } mx0, mx1;
  mx0.m = m0;
  mx1.m = m1;
  float tmp0 = 0, tmp1 = 0;
  for (i = 0; i < 8; i++)
    {
      double a = *(mx0.n + i);
      if (a > tmp0) tmp0 = a;
      double b = *(mx1.n + i);
      if (b < tmp1) tmp1 = b;
    }
  for (i = t * 8; i < c; i++)
    {
      double a = *((double *)frames + i);
      if (a > tmp0) tmp0 = a;
      if (a < tmp1) tmp1 = a;
    }
  if (tmp0 < 0) tmp0 *= -1;
  if (tmp1 < 0) tmp1 *= -1;
  return tmp0 > tmp1 ? tmp0 : tmp1;
}
