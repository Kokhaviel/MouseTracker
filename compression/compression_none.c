/* 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Adrien Golinelli <golinelli.adrien@gmail.com>
 */
#include <stdio.h>
#include <stdlib.h>

#include "compression.h"


static int noop(const char *src, const char *dst) {
  (void)src;
  (void)dst;
  return 0;
}

int copy_file(const char *src, struct mt_buffer *dst) {
  FILE *in = fopen(src, "rb"); 
  if (!in) return -1;

  fseek(in, 0, SEEK_END);
  long size = ftell(in);
  rewind(in);

  uint8_t *buf = malloc(size);
  if (!buf) {
    fclose(in);
    return -1;
  }

  fread(buf, 1, size, in);
  fclose(in);

  dst->data = buf;
  dst->size = size;
  return 0;
}

const struct compressor none_compressor = {
    .name = "none",
    .extension = "mt",
    .compress_file = noop,
    .decompress_file = copy_file
};
