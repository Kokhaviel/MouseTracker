/* 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Adrien Golinelli <golinelli.adrien@gmail.com>
 */
#include "compression.h" 
#include <zlib.h>
#include <stdio.h>
#include <string.h>

int gzip_compress_file(const char *src, const char *dst) {
  FILE *in = fopen(src, "rb");
  if (!in) return -1;

  gzFile out = gzopen(dst, "wb9");  // compression level 9
  if (!out) {
    fclose(in);
    return -1;
  }

  char buf[IN_BUF_SIZE];
  size_t n;
  while ((n = fread(buf, 1, sizeof(buf), in)) > 0)
    gzwrite(out, buf, n);

  fclose(in);
  gzclose(out);
  
  return 0;
}

int gzip_decompress_file(const char *src, struct mt_buffer *dst) {
  gzFile in = gzopen(src, "rb");
  if (!in) return -1;

  uint8_t *buf = NULL;
  uint8_t tmp[IN_BUF_SIZE];
  size_t size = 0;
  size_t capacity = 0;

  while (1) {
    int n = gzread(in, tmp, sizeof tmp);
    
    if (n>0) {
      if (size + n > capacity) {
        size_t new_cap = capacity ? capacity * 2 : 16384;
        while (new_cap < size + n)
          new_cap *= 2;

        uint8_t *new_buf = realloc(buf, new_cap);
        if (!new_buf) {
          free(buf);
          gzclose(in);
          return -1;
        }

        buf = new_buf;
        capacity = new_cap;
      }

      memcpy(buf + size, tmp, n);
      size += n;
    }

    if (n == 0) break;

    if (n < 0) {
      free(buf);
      gzclose(in);
      return -1;
    }
  }

  gzclose(in);
  dst->data = buf;
  dst->size = size;

  return 0;
}

const struct compressor gzip_compressor = {
    .name = "gzip",
    .extension = "gz",
    .compress_file = gzip_compress_file,
    .decompress_file = gzip_decompress_file
};
