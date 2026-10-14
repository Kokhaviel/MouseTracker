/* 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Adrien Golinelli <golinelli.adrien@gmail.com>
 */
#pragma once

#include <getopt.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <libinput.h>

struct devices {
  struct libinput_device **devices_list;
  const char **devices_names;
  int count;
};

enum mt_mode {
  MT_MODE_NONE,
  MT_MODE_RECORD,
  MT_MODE_PLOT
};

struct mt_options {
  enum mt_mode mode;
  const char *plot_dir;
  const char *compressor_name;
  const struct compressor *compressor;
  int no_compress;
  uint64_t chunk_duration_ns;
  struct devices devices;
  char *session_name;
};


static struct option long_opts[] = {
  { "record",           no_argument,       NULL, 'r' },
  { "plot",             optional_argument, NULL, 'p' },
  { "compressor",       required_argument, 0,    'c' },
  { "no-compress",      no_argument,       NULL, 'n'},
  { "chunk-size",       required_argument, NULL, 's'},
  { "device",           required_argument, NULL, 'd'},
  { "session-name",     required_argument, NULL, 'e'},
  { "list-compressors", no_argument,       NULL, 'l'},
  { "help",             no_argument,       0,    'h' },
  { NULL,               0,                 NULL, 0 }
};

static void print_usage(const char *progname) {
  printf(
    "Usage: %s [OPTIONS]\n"
    "\n"
    "Modes (one required):\n"
    "  -r, --record                 Record mouse activity\n"
    "  -p, --plot <session-dir>     Plot a recorded session\n"
    "\n"
    "Recording options:\n"
    "  -c, --compressor <name>      Force compressor (see --list-compressors)\n"
    "  -n, --no-compress            Disable compression (overrides -c)\n"
    "  -s, --chunk-size <seconds>   Chunk duration in seconds (default: 3)\n"
    "  -d, --device <event-path>    Restrict to a specific input device\n"
    "                                (can be specified multiple times)\n"
    "  -e, --session-name <name>    Custom session name (directory)\n"
    "\n"
    "Information:\n"
    "  -l, --list-compressors       List available compression backends\n"
    "  -h, --help                   Show this help and exit\n"
    "\n"
    "Notes:\n"
    "  - If both --no-compress and -c are provided, compression is disabled.\n"
    "  - Devices must be valid /dev/input/eventX paths.\n"
    "\n",
    progname
  );
}

int fill_opts(int argc, char *argv[], struct mt_options *opts);
