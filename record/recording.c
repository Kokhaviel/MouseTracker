/* 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Adrien Golinelli <golinelli.adrien@gmail.com>
 */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <libinput.h>
#include <sys/stat.h>

#include "../flock.h"
#include "../opts.h"
#include "../mouse/device_input.h"
#include "../compression/compression.h"
#include "../storage/format.h"
#include "../storage/chunk_writer.h"

#define TIMESTAMP_LEN 20 // "YYYY-MM-DD-hh:mm:ss\0"

extern volatile int stop_requested;
extern struct compressor none_compressor;

void compress_chunk_file(const uint32_t chunk_index, const char *chunk_path, 
                         const char *compressed_path, const struct mt_options opts) {
  int res = opts.compressor->compress_file(chunk_path, compressed_path);
  if (!res) {
    printf("[mouse-tracker] Chunk %03u compressed using %s compressor\n", chunk_index, opts.compressor->name);
    if(!(opts.compressor->name == none_compressor.name))
      remove(chunk_path);
  } else {
    printf("[mouse-tracker] Unable to compress chunk %03u with %s compressor\n", chunk_index, opts.compressor->name);
  } 
}

void run_record_mode(struct mt_options *opts) {
  struct libinput *li;
  struct udev *udev;

  udev = udev_new();
  
  if (!udev) {
    fprintf(stderr, "[mouse-tracker] Failed to create udev context\n");
    return;
  }

  // libinput initialization 
  li = libinput_udev_create_context(&libinput_iface, NULL, udev);

  if (!li) {
    fprintf(stderr, "[mouse-tracker] Failed to create libinput context\n");
    udev_unref(udev);
    return;
  }

  if (libinput_udev_assign_seat(li, "seat0") != 0) {
    fprintf(stderr, "[mouse-tracker] Failed to assign seat\n");
    libinput_unref(li);
    udev_unref(udev);
    return;
  }

  // Detect connected mice
  select_mouse_device(li, opts);

  if (opts->devices.devices_list == NULL) {
    fprintf(stderr, "No output device available. Exiting ...");
    exit(EXIT_FAILURE);
  }


  struct libinput_event *event;

  // Set default session name to current datetime
  if (opts->session_name == NULL) {
    opts->session_name = malloc(TIMESTAMP_LEN); 
    format_timestamp(opts->session_name, TIMESTAMP_LEN);
  }

  uint64_t chunk_start_ns = now_ns();
  uint32_t chunk_index = 0;
  
  char *session_dir; 
  char *base_dir;
  char *chunk_path;
  char *compressed_path;

  FILE *chunk_file = NULL;

  asprintf(&base_dir, "%s/%s", getenv("HOME"), MT_DIR);
  asprintf(&session_dir, "%s/%s", base_dir, opts->session_name);

  asprintf(&chunk_path, "%s/chunk_%03u.mt", session_dir, chunk_index);
  asprintf(&compressed_path, "%s.%s", chunk_path, opts->compressor->extension);

  mkdir(session_dir, 0755);

  chunk_file = fopen(chunk_path, "wb");

  printf("[mouse-tracker] Session started: %s\n", session_dir);
  while (1) {
    libinput_dispatch(li);

    // Run if an input device state has changed
    while ((event = libinput_get_event(li)) != NULL) {
      enum libinput_event_type type = libinput_event_get_type(event);

      struct libinput_device *dev = libinput_event_get_device(event);

      // Check if it was a mouse
      if (!device_is_selected(dev, &opts->devices)) {
        libinput_event_destroy(event);
        continue;
      }

      // Compress current and exit after receiving SIGTERM
      if (stop_requested) {
        chunk_close(chunk_file);
        printf("\n[mouse-tracker] SIGINT or SIGTERM received, compressing current chunk\n");
        compress_chunk_file(chunk_index, chunk_path, compressed_path, *opts);
        exit(EXIT_SUCCESS);
      }

      uint64_t t = now_ns();
      if (t - chunk_start_ns >= opts->chunk_duration_ns) { // Rotate the recording file every n seconds
        chunk_close(chunk_file);
        printf("[mouse-tracker] Chunk %03u closed\n", chunk_index);

        compress_chunk_file(chunk_index, chunk_path, compressed_path, *opts);

        chunk_start_ns = t;
        chunk_index++;
        free(chunk_path);
        free(compressed_path);
        asprintf(&chunk_path, "%s/chunk_%03u.mt", session_dir, chunk_index);
        asprintf(&compressed_path, "%s.%s", chunk_path, opts->compressor->extension);

        printf("[mouse-tracker] Rotating chunk %03u -> %03u (duration=%lus)\n", chunk_index -1, chunk_index, opts->chunk_duration_ns / 1000000000UL);
        
        chunk_file = fopen(chunk_path, "wb");
        printf("[mouse-tracker] Chunk %03u opened (session %s)\n", chunk_index, opts->session_name);
      }

      // Mouse cursor movement
      if (type == LIBINPUT_EVENT_POINTER_MOTION) {

        struct libinput_event_pointer *pev = libinput_event_get_pointer_event(event);

        double dx = libinput_event_pointer_get_dx(pev);
        double dy = libinput_event_pointer_get_dy(pev);

        const char *name = libinput_device_get_name(dev);

        // printf("[%s] MOVE dx=%.2f dy=%.2f\n", name, dx, dy);
        chunk_write_move(chunk_file, dx, dy);
      }

      // Mouse button click
      if (type == LIBINPUT_EVENT_POINTER_BUTTON) {
        struct libinput_event_pointer *pev = libinput_event_get_pointer_event(event);

        uint32_t button = libinput_event_pointer_get_button(pev);
        enum libinput_button_state state = libinput_event_pointer_get_button_state(pev);

        // printf("BUTTON %u %s\n", button, state == LIBINPUT_BUTTON_STATE_PRESSED
        //           ? "pressed" : "released");
        chunk_write_button(chunk_file, button);
      }
      libinput_event_destroy(event);
    }
    usleep(5000); // Avoids burning the cpu
  }

  free(base_dir);
  free(session_dir);
}

