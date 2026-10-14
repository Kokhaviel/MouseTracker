/* 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Adrien Golinelli <golinelli.adrien@gmail.com>
 */
#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <time.h>

enum mt_event_type : uint8_t {
  MT_MOVE = 1,
  MT_BUTTON = 2
};

#pragma pack(push, 1)

struct mt_event_move {
  float dx;
  float dy;
};

struct mt_event_button {
  uint32_t button;
};

// Represents a single input event stored
struct mt_event {
  uint8_t type; 
  union {
    struct mt_event_move move;
    struct mt_event_button button;
  } event;
};

#pragma pack(pop)

struct mt_events {
  struct mt_event *ev;
  size_t size;
};

// Current monotonic time in nanoseconds
static inline uint64_t now_ns(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

static void format_timestamp(char *buf, size_t sz) {
    time_t t = time(NULL);
    struct tm tm;
    localtime_r(&t, &tm);
    strftime(buf, sz, "%Y-%m-%d_%H:%M:%S", &tm);
}

