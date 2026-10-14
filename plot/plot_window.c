/* 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Adrien Golinelli <golinelli.adrien@gmail.com>
 */
#include <gtk/gtk.h>
#include "plot.h"
#include "../storage/format.h"

// Clear all drawings
void reset_state(struct plot_state *state) {
  int w = gtk_widget_get_width(state->drawing_area);
  int h = gtk_widget_get_height(state->drawing_area);

  state->events_drawn = 0;
  state->cursor_x = w/2.0f;
  state->cursor_y = h/2.0f;
  state->off_x = w/2.0f;
  state->off_y = h/2.0f;
}

// Shift drawing area if next point is outside
static void step_forward(struct plot_state *state, struct mt_event *ev) {
  if (ev->type != MT_MOVE) {
    state->events_drawn++;
    return;
  }

  const float margin = 20.0f;

  int w = gtk_widget_get_width(state->drawing_area);
  int h = gtk_widget_get_height(state->drawing_area);

  float dx = ev->event.move.dx * state->scale;
  float dy = ev->event.move.dy * state->scale;

  float nx = state->cursor_x + dx;
  float ny = state->cursor_y + dy;

  if      (nx < margin)     state->off_x += (w - 2 * margin);
  else if (nx > w - margin) state->off_x -= (w - 2 * margin);
  if      (ny < margin)     state->off_y += (h - 2 * margin);
  else if (ny > h - margin) state->off_y -= (h - 2 * margin);

  state->cursor_x = nx;
  state->cursor_y = ny;
  state->events_drawn++;
}

gboolean next_event(gpointer user_data) {
  struct plot_state *state = user_data;

  if (!state->animating) {
    return G_SOURCE_REMOVE;
  }

  if (state->events_drawn >= state->events->size) {
    state->animating = false;
    return G_SOURCE_REMOVE;
  }

  step_forward(state, &state->events->ev[state->events_drawn]);
  gtk_widget_queue_draw(state->drawing_area);
  return G_SOURCE_CONTINUE;
}

// Draw mouse recording all at once
void draw_all(struct plot_state *state) {
  state->animating = false;
  while (state->events_drawn < state->events->size) 
    step_forward(state, &state->events->ev[state->events_drawn]);
  gtk_widget_queue_draw(state->drawing_area);
}

void start_animation(struct plot_state *state) {
  reset_state(state);
  state->animating = true;
  gtk_widget_queue_draw(state->drawing_area);
  g_timeout_add(1, next_event, state);
}

void on_draw(GtkDrawingArea *area, cairo_t *cr, 
             int width, int height, gpointer user_data) {
  struct plot_state *state = user_data;

  cairo_set_source_rgb(cr, 0.08, 0.08, 0.12);
  cairo_paint(cr);

  if(state->events_drawn == 0) return;

  float x = state->off_x;
  float y = state->off_y;

  cairo_set_line_width(cr, 1.5);
  cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
  cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);

  gboolean already_started = false;
  for(size_t i = 0; i < state->events_drawn; i++) {
    struct mt_event *curr_ev = &state->events->ev[i];

    if(curr_ev->type == MT_MOVE) {

      const float margin = 20.0f;
      float dx = curr_ev->event.move.dx * state->scale;
      float dy = curr_ev->event.move.dy * state->scale;

      float nx = x + dx;
      float ny = y + dy;

      gboolean wrapped = FALSE;
      if      (nx < margin)         { x += (width  - 2*margin); wrapped = TRUE; }
      else if (nx > width - margin) { x -= (width  - 2*margin); wrapped = TRUE; }
      if      (ny < margin)         { y += (height - 2*margin); wrapped = TRUE; }
      else if (ny > height - margin){ y -= (height - 2*margin); wrapped = TRUE; }
 
      x += dx;
      y += dy;

      if(!already_started || wrapped) {
        if(already_started) {
          double t = (double) i / (double) state->events->size;
          cairo_set_source_rgba(cr, 0.2 + 0.6*t, 0.5 + 0.4*t, 1.0, 0.6 + 0.4*t);
          cairo_stroke(cr);
        }
        cairo_move_to(cr, x, y);
        already_started = true;
      } else {
        cairo_line_to(cr, x, y);
      }
    }
    else if(curr_ev->type == MT_BUTTON) {
      if(already_started) {
        double t = (double) i / (double) state->events->size;
        cairo_set_source_rgba(cr, 0.2 + 0.6*t, 0.5 + 0.4*t, 1.0, 0.6 + 0.4*t);
        cairo_stroke(cr);
        already_started = false;
      }
      cairo_arc(cr, x, y, 4.0, 0, 2*G_PI);
      cairo_set_source_rgba(cr, 1.0, 0.85, 0.2, 0.9);
      cairo_fill(cr);
    }
  }

  if(already_started) {
    double t = (double) state->events_drawn / (double) state->events->size;
    cairo_set_source_rgba(cr, 0.2 + 0.6*t, 0.5 + 0.4*t, 1.0, 0.6 + 0.4*t);
    cairo_stroke(cr);
  }
}

static gboolean on_key_pressed(GtkEventControllerKey *controller, guint keyval, 
                                guint keycode, GdkModifierType state_mod, 
                                gpointer user_data) {
  struct plot_state *state = user_data; 

  if(keyval == GDK_KEY_r || keyval == GDK_KEY_R) {
    start_animation(state);
    return true;
  }

  return false;
}

void on_window_map(GtkWidget *widget, gpointer user_data) {
  draw_all(user_data);
}

void activate_plot(GtkApplication *app, gpointer user_data) {
  struct plot_state *state = user_data;
  
  GtkWidget *window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window), "MouseTracker – Plot");
  gtk_window_set_default_size(GTK_WINDOW(window), 1200, 800);

  GtkWidget *da = gtk_drawing_area_new();
  state->drawing_area = da;
  gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(da), on_draw, state, NULL);
  gtk_window_set_child(GTK_WINDOW(window), da);

  GtkEventController *key_ctrl = gtk_event_controller_key_new();
  g_signal_connect(key_ctrl, "key-pressed", G_CALLBACK(on_key_pressed), state);
  gtk_widget_add_controller(window, key_ctrl);

  g_signal_connect(window, "map", G_CALLBACK(on_window_map), state);
  gtk_window_present(GTK_WINDOW(window));

}
