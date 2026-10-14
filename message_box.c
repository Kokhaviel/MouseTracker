/* 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Adrien Golinelli <golinelli.adrien@gmail.com>
 */
#include <gtk/gtk.h>

// https://www.reddit.com/r/gnome/comments/137w9l4/gtk4_v410_alert_dialog_example_in_c/

char dialog_title[64];
char dialog_message[2048];

void on_activate (GtkApplication *app);
void on_choose (GObject *source_object, GAsyncResult *res, gpointer user_data);

void show_message_box(const char *msg) {
  GtkApplication *app = gtk_application_new ("fr.mousetracker.alert-dialog", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect (app, "activate", G_CALLBACK (on_activate), NULL);
  
  char *args[] = {"MouseTracker"};
  g_application_run (G_APPLICATION (app), 1, args);
}


void on_activate (GtkApplication *app) {
  GtkWidget *win = gtk_application_window_new (app);
  GtkAlertDialog *dialog = gtk_alert_dialog_new (dialog_title);
  const char* buttons[] = {"Ok", NULL};
  gtk_alert_dialog_set_detail (dialog, dialog_message);
  gtk_alert_dialog_set_buttons (dialog, buttons);
  gtk_alert_dialog_set_default_button (dialog, 0);  // If the user presses enter key, "Proceed" button
                                                    // should be pressed. Proceed is at index 1

  gtk_alert_dialog_choose (dialog, GTK_WINDOW (win), NULL, on_choose, NULL);
}

void on_choose (GObject *source_object, GAsyncResult *res, gpointer user_data) {
  GtkAlertDialog *dialog = GTK_ALERT_DIALOG (source_object);
  GError *err = NULL;

  int button = gtk_alert_dialog_choose_finish (dialog, res, &err);

  if (err) {
    g_print("An error occured!\n");
    g_print("Error Message: %s\n", err->message);
    return;
  }

  if (button == 0) 
    exit(EXIT_FAILURE);
  else
    g_assert_not_reached();
}
