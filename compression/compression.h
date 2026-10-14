/* 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Adrien Golinelli <golinelli.adrien@gmail.com>
 */
#pragma once

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>

#define IN_BUF_SIZE 8192

struct mt_buffer {
  uint8_t *data;
  size_t size;
};

struct compressor {
  const char *name;
  const char *extension;
  int (*compress_file)(const char *source, const char *dest);
  int (*decompress_file)(const char *source, struct mt_buffer *out);
};

extern const struct compressor *compressor;

static int compressor_available(const struct compressor *c) {
  return c && c->compress_file && c->decompress_file;
}

static size_t file_size(const char *path) {
    struct stat st;
    if (stat(path, &st) != 0)
        return 0;
    return st.st_size;
}

static void mt_buffer_free(struct mt_buffer *buf) {
    free(buf->data);
    buf->data = NULL;
    buf->size = 0;
}

void list_compressors(void);
const struct compressor *compression_select_auto(void);
const struct compressor *compression_select_by_name(const char *name);
const struct compressor *compression_select_by_ext(const char *ext);
