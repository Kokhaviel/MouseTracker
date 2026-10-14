/* 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Adrien Golinelli <golinelli.adrien@gmail.com>
 */
#pragma once

#include <stdio.h>
#include "format.h"
#include "../compression/compression.h"

struct mt_reader {
  char *chunk_path;
  FILE *f;
  const struct compressor *compressor;
};

int  read_chunk(struct mt_reader *r, struct mt_buffer *buf);
int  parse_events(struct mt_buffer *buf, struct mt_events *events);
