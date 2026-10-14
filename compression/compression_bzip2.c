/* 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Adrien Golinelli <golinelli.adrien@gmail.com>
 */
#include <bzlib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "compression.h"

int bzip2_compress_file(const char *src, const char *dst) {
  FILE *in = fopen(src, "rb");
  if (!in) return -1;
  FILE *out = fopen(dst, "wb");
  if (!out) {
    fclose(in);
    return -1;
  }

  int bzerr;
  BZFILE *bz = BZ2_bzWriteOpen(&bzerr, out, 9, 0, 30); // Eviter les magic numbers
  if (bzerr != BZ_OK) {
    fclose(out);
    fclose(in);
    return -1;
  }

  char buf[IN_BUF_SIZE];
  size_t n;
  while ((n = fread(buf, 1, sizeof(buf), in)) > 0)
    BZ2_bzWrite(&bzerr, bz, buf, n);
  BZ2_bzWriteClose(&bzerr, bz, 0, NULL, NULL);

  fclose(in);
  fclose(out);
  return 0;
}

int bzip2_decompress_file(const char *src, struct mt_buffer *dst) {
  FILE *in = fopen(src, "rb");
  if (!in) return -1;

  int bzerr;
  BZFILE *bz = BZ2_bzReadOpen(&bzerr, in, 0, 0, NULL, 0);
  if (bzerr != BZ_OK) {
    fclose(in);
    return -1;
  }

  uint8_t *buf = NULL;
  uint8_t tmp[IN_BUF_SIZE];
  size_t size = 0;
  size_t capacity = 0;
  
  while (1) {
    int n = BZ2_bzRead(&bzerr, bz, tmp, sizeof tmp);

    if (n > 0) {
      if (size + n > capacity) {
        size_t new_cap = capacity ? capacity * 2 : 16384;
        while (new_cap < size + n)
          new_cap *= 2;

        uint8_t *new_buf = realloc(buf, new_cap);
        if (!new_buf) {
          free(buf);
          BZ2_bzReadClose(&bzerr, bz);
          fclose(in);
          return -1;
        }

        buf = new_buf;
        capacity = new_cap;
      }

      memcpy(buf + size, tmp, n);
      size += n;
    }

    if (bzerr == BZ_STREAM_END)
      break;

    if (bzerr != BZ_OK) {
      free(buf);
      BZ2_bzReadClose(&bzerr, bz);
      fclose(in);
      return -1;
    }
  }

  BZ2_bzReadClose(&bzerr, bz);
  fclose(in);

  dst->data = buf;
  dst->size = size;

  return 0;
}

const struct compressor bzip2_compressor = {
    .name = "bzip2",
    .extension = "bz2",
    .compress_file = bzip2_compress_file,
    .decompress_file = bzip2_decompress_file
};
