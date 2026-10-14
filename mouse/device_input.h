/* 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Adrien Golinelli <golinelli.adrien@gmail.com>
 */
#pragma once

#include <libinput.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

#include "../opts.h"

extern char dialog_title[64], dialog_message[2048];

// List remediation paths to inaccessible /dev/input/*
#define EACCES_MESSAGE "MouseTracker: Cannot access mouse devices.\n" \
  "To run the program correctly, choose one of the following options:\n" \
  " 1) Run with sudo: 'sudo MouseTracker rec'\n" \
  " 2) Add your user to the 'input' group:\n" \
  "     sudo usermod -aG input $USER\n" \
  " then log out and log back in.\n" \
  " 3) Create a specific udev rule to allow the mouse:\n" \
  "     /etc/udev/rules.d/99-mouse-recorder.rules\n" \
  " Example : KERNEL==\"event*\", ATTRS{name}==\"Exact_mouse_name using `libinput list-devices`\", MODE=\"660\", GROUP=\"input\"\n" \
  " then run: sudo udevadm control --reload && sudo udevadm trigger"

void show_message_box(const char*);

static int open_restricted(const char *path, int flags, void *user_data) {
  int fd = open(path, flags);
  if (fd < 0) {
    if (errno == EACCES) {
      strcpy(dialog_title, "Missing permissions");
      dialog_title[63] = '\0';
      strcpy(dialog_message, EACCES_MESSAGE);
      dialog_message[2047] = '\0';
      show_message_box(EACCES_MESSAGE);
    }
    return fd < 0 ? -errno : fd;
  }
  return fd;
}

static void close_restricted(int fd, void *user_data) {
  close(fd);
}

static const struct libinput_interface libinput_iface = {
  .open_restricted = open_restricted,
  .close_restricted = close_restricted,
};

void select_mouse_device(struct libinput *li, struct mt_options *opts);
void free_device_list(struct devices *);

static bool device_is_selected(struct libinput_device *dev, struct devices *list) {
  for (size_t i = 0; i < list->count; ++i)
    if (list->devices_list[i] == dev)
      return true;
  return false;
}

static int is_device_wanted(const struct devices wanted_devices, const char* name_to_check) {
  for (size_t i = 0; i < wanted_devices.count; ++i)
      if (strcmp(wanted_devices.devices_names[i], name_to_check) == 0)
        return i;
  return -1;
}
