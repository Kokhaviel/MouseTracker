/* 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Adrien Golinelli <golinelli.adrien@gmail.com>
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>

#include "opts.h"
#include "compression/compression.h"

extern struct compressor none_compressor;

int fill_opts(int argc, char *argv[], struct mt_options *opts) {
  int opt;

  while ((opt = getopt_long(argc, argv, "rpc:ns:d:e:hl", long_opts, NULL)) != -1) {
    switch (opt) {
      case 'r':
        opts->mode = MT_MODE_RECORD;
        break;

      case 'p':
        opts->mode = MT_MODE_PLOT;
        opts->plot_dir = optarg;
        break;

      case 'c':
        opts->compressor_name = optarg;
        break;

      case 'n':
        opts->no_compress = 1;
        break;
      case 'd':
        if (access(optarg, R_OK) != 0) {
          perror(optarg);
          exit(EXIT_FAILURE);
        }

        opts->devices.devices_names = realloc(opts->devices.devices_names,
          (opts->devices.count +1) * sizeof(char*));

        opts->devices.devices_names[opts->devices.count++] = strdup(optarg);

        break;

      case 's':
        char *end = NULL;
        long sec = strtol(optarg, &end, 10);

        if (!end || *end != '\0' || sec <= 0) {
          fprintf(stderr, "[mouse-tracker] Invalid duration: %s\n", optarg);
          exit(EXIT_FAILURE);
        }

        opts->chunk_duration_ns = (uint64_t)sec * 1000000000ULL;
        break;

      case 'e':
        if (strstr(optarg, "/") || strstr(optarg, "..")) {
          fprintf(stderr, "[mouse-tracker] Invalid session name: '%s' "
                          "(must not contain '/' or '..')\n", optarg);
          exit(EXIT_FAILURE);
        }
        opts->session_name = optarg;
        break;

      case 'l':
        list_compressors();
        exit(EXIT_SUCCESS);

      case 'h':
        print_usage(argv[0]);
        exit(EXIT_SUCCESS);

      default:
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }
  }

  if (opts->mode == MT_MODE_NONE) {
    fprintf(stderr, "[mouse-tracker] No mode specified\n");
    print_usage(argv[0]);
    return 1;
  }

  if (opts->compressor_name) {
    opts->compressor = compression_select_by_name(opts->compressor_name);
    if (!opts->compressor) {
        fprintf(stderr, "[mouse-tracker] Unknown compressor: %s\n",
                opts->compressor_name);
        return 1;
    }
  } else {
    opts->compressor = compression_select_auto();
  }

  if (opts->no_compress && strcmp(opts->compressor->name, none_compressor.name) != 0) {
    fprintf(stderr, "[mouse-tracker] --no-compress overrides -c %s\n", opts->compressor_name);
    opts->compressor = &none_compressor;
    printf("%s", opts->compressor->name);
    opts->compressor_name = opts->compressor->name;
  }

  return 0;
}
