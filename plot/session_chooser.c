/* 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Adrien Golinelli <golinelli.adrien@gmail.com>
 */
#include <gtk/gtk.h>

#include "../flock.h"
#include "../opts.h"

void on_folder_chosen(GObject *dialog, GAsyncResult  *res, gpointer user_data) {
  struct mt_options *opts = user_data;
  GError *error = NULL;

  GFile *file = gtk_file_dialog_select_folder_finish((GtkFileDialog *) dialog, res, &error);
  if (!file) {
    g_printerr("[mouse-tracker] No folder selected\n");
    g_clear_error(&error);
    gtk_window_destroy(GTK_WINDOW(gtk_application_get_active_window(
      GTK_APPLICATION(g_application_get_default()))));
    return;
  }

  char *path = g_file_get_path(file);
  g_object_unref(file);

  opts->session_name = strdup(path);
  g_free(path);

  g_application_quit(g_application_get_default());
}

void fc_on_activate(GtkApplication *app, gpointer user_data) {
  struct mt_options *opts = user_data;
  GtkFileDialog *dialog = gtk_file_dialog_new();

  gtk_file_dialog_set_title(dialog, "Select MouseTracker session");
  gtk_file_dialog_set_accept_label(dialog, "Open session");

  char *default_path;
  asprintf(&default_path, "%s/%s", getenv("HOME"), MT_DIR);

  GFile *default_dir = g_file_new_for_path(default_path);
  gtk_file_dialog_set_initial_folder(dialog, default_dir);

  g_object_unref(default_dir);
  free(default_path);

  gtk_file_dialog_select_folder(dialog, GTK_WINDOW(gtk_application_window_new(app)),
                                  NULL, on_folder_chosen, opts);
}

void run_file_chooser(struct mt_options *opts) {
  GtkApplication *app = gtk_application_new ("org.mousetracker.plot", G_APPLICATION_DEFAULT_FLAGS);

  g_signal_connect(app, "activate", G_CALLBACK(fc_on_activate), opts);
  int status = g_application_run(G_APPLICATION(app), 0, NULL);
  g_object_unref(app);

}
