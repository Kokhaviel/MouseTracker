/* 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Adrien Golinelli <golinelli.adrien@gmail.com>
 */
#pragma once

#include <gtk/gtk.h>
#include "../storage/format.h"

struct mt_options;

struct plot_state {
  GtkWidget *drawing_area;
  struct mt_events *events;
  size_t events_drawn;
  float scale;
  float cursor_x, cursor_y;
  float off_x, off_y;
  gboolean animating;
};

int check_directory_exists(const char *path);
int is_regular_file(const char *dir, const char *file_name);
void run_file_chooser(struct mt_options *opts);
void activate_plot(GtkApplication *app, gpointer user_data);

gboolean next_event(gpointer user_data);
void on_draw(GtkDrawingArea *area, cairo_t *cr, int width, int height, gpointer user_data);
void on_window_map(GtkWidget *widget, gpointer user_data);
