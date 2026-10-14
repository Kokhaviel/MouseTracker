/* 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Adrien Golinelli <golinelli.adrien@gmail.com>
 */
#pragma once

#include <stdio.h>
#include <stdint.h>

void chunk_close(FILE *f);
void chunk_write_move(FILE *f, float dx, float dy);
void chunk_write_button(FILE *f, uint32_t button);



