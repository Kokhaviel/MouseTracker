/* 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Adrien Golinelli <golinelli.adrien@gmail.com>
 */
#include <stdlib.h>
#include <stdio.h>

#include "format.h"
#include "chunk_writer.h"

void chunk_write_move(FILE *f, float dx, float dy) {
  struct mt_event_move move = {
    .dx = dx,
    .dy = dy
  };

  struct mt_event ev = {
    .type = MT_MOVE,
    .event.move = move
  };

  fwrite(&ev, sizeof(ev), 1, f);
}

void chunk_write_button(FILE *f, uint32_t button) {
  struct mt_event_button btn = {
    .button = button
  };

  struct mt_event ev = {
    .type = MT_BUTTON,
    .event.button = btn
  };

  fwrite(&ev, sizeof(ev), 1, f);
}

void chunk_close(FILE *f) {
  if (!f) return;

  fflush(f);
  fclose(f);
}
