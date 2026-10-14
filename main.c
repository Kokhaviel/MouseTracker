/* 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Adrien Golinelli <golinelli.adrien@gmail.com>
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#include "flock.h"
#include "opts.h"

void run_record_mode(const struct mt_options *opts);
void run_plot_mode(struct mt_options *opts);

volatile int stop_requested = 0;

// Triggers the end of recording loop (in recording.c)
void finalize(int code) {
  stop_requested = 1;
}

struct mt_options opts = {
  .mode = MT_MODE_NONE,
  .plot_dir = NULL,
  .compressor_name = NULL,
  .compressor = NULL,
  .no_compress = 0,
  .chunk_duration_ns = 1800*1000000000ULL, // 30 minutes per chunk
  .devices = {
    .devices_list = NULL,
    .devices_names = NULL,
    .count = 0
  },
  .session_name = NULL,
};

int main(int argc, char **argv) {

  const char *home = getenv("HOME");
  if (!home) { fprintf(stderr, "HOME not set\n"); exit(EXIT_FAILURE); }

  if (fill_opts(argc, argv, &opts)) exit(EXIT_FAILURE);

  switch (opts.mode) {
    case MT_MODE_RECORD:
      signal(SIGTERM, finalize);
      signal(SIGINT, finalize);

      fprintf(stdout, "--- Running %s in record mode ---\n", argv[0]);

      char *lock_dir;
      asprintf(&lock_dir, "%s/%s", getenv("HOME"), MT_DIR);    
      acquire_instance_lock(lock_dir); // Lock file for this instance or terminate existing
      free(lock_dir);
      
      run_record_mode(&opts);
      break;

    case MT_MODE_PLOT:
      fprintf(stdout, "--- Running %s in plot mode ---\n", argv[0]);
      run_plot_mode(&opts);
      break;

    default:
      __builtin_unreachable();
  }
  return 0;
}
