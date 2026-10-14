/* 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Adrien Golinelli <golinelli.adrien@gmail.com>
 */
#include "compression.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <zstd.h>

int zstd_compress_file(const char *src, const char *dst) {
  FILE *in = fopen(src, "rb");
  if (!in) return -1;

  FILE *out = fopen(dst, "wb");
  if (!out) {
    fclose(in);
    return -1;
  }

  size_t src_size = file_size(src);
  if (src_size == 0) {
    goto close_resources;
  }

  void *src_buf = malloc(src_size);
  if(!src_buf) {
    free(src_buf);
    goto close_resources;
  }

  fread(src_buf, 1, src_size, in);

  size_t max_dst_size = ZSTD_compressBound(src_size);
  void *dst_buf = malloc(max_dst_size);
  if(!dst_buf) {
    free(src_buf);
    free(dst_buf);
    goto close_resources;
  }

  int level = 19;
  size_t compressed_size = ZSTD_compress(dst_buf, max_dst_size, src_buf, src_size, level);

  if (ZSTD_isError(compressed_size)) {
    fprintf(stderr, "zstd compress error : %s\n", ZSTD_getErrorName(compressed_size));
    free(src_buf);
    free(dst_buf);
    goto close_resources;
  }

  fwrite(dst_buf, 1, compressed_size, out);
  free(src_buf);
  free(dst_buf);
  fclose(in);
  fclose(out);
  return 0;

close_resources:
  fclose(in);
  fclose(out);
  return -1;
}

int zstd_decompress_file(const char *src, struct mt_buffer *dst) {

  FILE *in = fopen(src, "rb");
  if (!in) return -1;

  // FILE *out = fopen(dst, "wb");
  // if (!out) {
    // fclose(in);
    // return -1;
  // }

  size_t compressed_size = file_size(src);
  if (compressed_size == 0) {
    fclose(in);
    return -1;
  }

  void *compressed_buf = malloc(compressed_size);
  if (!compressed_buf) {
    fclose(in);
    return -1;
  }

  if (fread(compressed_buf, 1, compressed_size, in) != compressed_size) {
    free(compressed_buf);
    fclose(in);
    return -1;
  }

  unsigned long long decompressed_size = ZSTD_getFrameContentSize(compressed_buf, compressed_size);

  if (decompressed_size == ZSTD_CONTENTSIZE_ERROR ||
    decompressed_size == ZSTD_CONTENTSIZE_UNKNOWN) {
    fprintf(stderr, "zstd: invalid or unknown content size\n");
    free(compressed_buf);
    fclose(in);
    return -1;
  }

  void *dst_buf = malloc(decompressed_size);
  if (!dst_buf) {
    free(compressed_buf);
    free(dst_buf);
    fclose(in);
    return -1;
  }

  size_t ret = ZSTD_decompress(dst_buf, decompressed_size, compressed_buf, compressed_size);

  if (ZSTD_isError(ret)) {
    fprintf(stderr, "zstd decompress error: %s\n", ZSTD_getErrorName(ret));
    free(compressed_buf);
    free(dst_buf);
    fclose(in);
    return -1;
  }

  dst->data = dst_buf;
  dst->size = decompressed_size;

  free(compressed_buf);
  fclose(in);
  return 0;
}

const struct compressor zstd_compressor = {
    .name = "zstd",
    .extension = "zstd",
    .compress_file = zstd_compress_file,
    .decompress_file = zstd_decompress_file
};
