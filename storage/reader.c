/* 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Adrien Golinelli <golinelli.adrien@gmail.com>
 */
#include <string.h>
#include <unistd.h>

#include "reader.h"

// Copy uncompressed chunk file specified in the given buffer
int read_chunk(struct mt_reader *r, struct mt_buffer *buf) {
  char *file_ext = strrchr(r->chunk_path, '.');
  if (!file_ext) return -1;
  file_ext++;

  r->compressor = compression_select_by_ext(file_ext);
  if (!r->compressor)
        return -1;

  if (r->compressor->decompress_file(r->chunk_path, buf) != 0) {
    return -1;
  }

  return 0;
}


int parse_events(struct mt_buffer *buf, struct mt_events *events) {
  // Except buffer size to be an exact multiple of mt_event size
  if (buf->size % sizeof(struct mt_event)) {
    printf("Invalid chunk size\n");
    return -1;
  }

  size_t n_events = buf->size / sizeof(struct mt_event);

  events->ev = malloc(n_events * sizeof *(events->ev));
  if (!events->ev) return -1;

  // Copy raw data as an array of events
  memcpy(events->ev, buf->data, buf->size);
  events->size = n_events;

  return 0;
}
