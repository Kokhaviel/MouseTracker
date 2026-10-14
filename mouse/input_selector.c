/* 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Adrien Golinelli <golinelli.adrien@gmail.com>
 */
#include <libinput.h>
#include <libudev.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "device_input.h"

// Store all devices detected as a mouse
void select_mouse_device(struct libinput *li, struct mt_options *opts) {
  int devices_provided = 0;
  if (opts->devices.devices_names != NULL) devices_provided = 1;

  libinput_dispatch(li);

  struct libinput_event *event;
  while ((event = libinput_get_event(li)) != NULL) {
    if (libinput_event_get_type(event) == LIBINPUT_EVENT_DEVICE_ADDED) {
      struct libinput_device *dev = libinput_event_get_device(event);
      const char *path = udev_device_get_devnode(libinput_device_get_udev_device(dev));


      if (libinput_device_has_capability(dev, LIBINPUT_DEVICE_CAP_POINTER)) {
        if (devices_provided) {
          int idx;
          if ((idx = is_device_wanted(opts->devices, path)) >= 0) {
            opts->devices.devices_list = realloc(opts->devices.devices_list, 
                                               (opts->devices.count+1)*sizeof(*opts->devices.devices_list));
            opts->devices.devices_list[idx] = dev;
            opts->devices.devices_names[idx] = libinput_device_get_name(dev);
          } 
        } else {
          opts->devices.devices_list = realloc(opts->devices.devices_list, 
                                               (opts->devices.count+1)*sizeof(*opts->devices.devices_list));
          opts->devices.devices_names = realloc(opts->devices.devices_names, 
                                                (opts->devices.count+1)*sizeof(*opts->devices.devices_names));
        
          opts->devices.devices_list[opts->devices.count] = dev;

          const char *name = libinput_device_get_name(dev);
          opts->devices.devices_names[opts->devices.count++] = name;

          printf("[mouse-tracker] Pointer device detected : %s\n", name);
        }
      } else {
        if (devices_provided && (is_device_wanted(opts->devices, path) != -1)) {
          printf("Device %s ignored : Not a pointer device\n", path);
        }
      }
    }

    libinput_event_destroy(event);

  }

}

void free_device_list(struct devices *list) {
    for (size_t i = 0; i < list->count; ++i)
        libinput_device_unref(list->devices_list[i]);

    free(list->devices_list);
    list->devices_list = NULL;
    list->count = 0;
}
