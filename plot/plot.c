/* 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Adrien Golinelli <golinelli.adrien@gmail.com>
 */
#include "plot.h"
#include "../opts.h"
#include "../flock.h"
#include "../storage/reader.h"
#include "../storage/format.h"

#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <gtk/gtk.h>

static int load_all_chunks(const char *session_dir, struct mt_events *events) {
  
  DIR *d = opendir(session_dir);
  if (!d) return -1;

  char **paths = NULL;
  int nb = 0;
  char *ext = NULL;
  struct dirent *dir;

  // Check all files in the directory
  while ((dir = readdir(d)) != NULL) {
    if (!is_regular_file(session_dir, dir->d_name)) 
      continue;
    
    char *curr_ext = strrchr(dir->d_name, '.');
    if(!curr_ext) continue;

    // The first extension encountered will be the only one processed
    if (ext == NULL)
      ext = strdup(curr_ext);

    if(strcmp(curr_ext, ext) != 0) {
      printf("Skipping %s (mixed extensions)\n", dir->d_name);
      continue;
    }

    char *path;
    asprintf(&path, "%s/%s", session_dir, dir->d_name);
    paths = realloc(paths, (nb+1) * sizeof *paths);
    paths[nb++] = path;
    printf("Found chunk : %s\n", dir->d_name);
  }

  closedir(d);
  free(ext);

  if(nb == 0) return 0;

  // Sort found chunks names
  for(int i = 0; i < nb - 1; i++)
    for(int j = i; j < nb; j++)
      if(strcmp(paths[i], paths[j]) > 0) {
        char *tmp = paths[i];
        paths[i] = paths[j];
        paths[j] = tmp;
      }

  size_t total_ev = 0;
  struct mt_event *all = NULL;
  
  struct mt_reader *r = malloc(sizeof *r);
  struct mt_buffer *buf = malloc(sizeof *buf);
  struct mt_events tmp_ev = {0};

  // Read every chunk and concatenate all events in one array
  for(int i = 0; i < nb; i++) {
    r->chunk_path = paths[i];
    if(read_chunk(r, buf) != 0) {
      fprintf(stderr, "Failed to read %s\n", paths[i]);
      free(paths[i]);
      continue;
    }

    if(parse_events(buf, &tmp_ev) != 0) {
      fprintf(stderr, "Failed to parse events from %s\n", paths[i]);
      free(paths[i]);
      continue;
    }

    all = realloc(all, (total_ev + tmp_ev.size) * sizeof *all);
    memcpy(all + total_ev, tmp_ev.ev, tmp_ev.size * sizeof *all);
    total_ev += tmp_ev.size;

    free(tmp_ev.ev);
    tmp_ev.ev = NULL;
    tmp_ev.size = 0;

    free(paths[i]);
  }

  free(paths);
  free(r);
  free(buf);

  events->ev = all;
  events->size = total_ev;

  printf("[mt-plot] Loaded %zu events\n", total_ev);
  return 0;
}

void run_plot_mode(struct mt_options *opts) {
  if (opts->session_name != NULL) {
    char *full_path;
    asprintf(&full_path, "%s/%s/%s", getenv("HOME"), MT_DIR, opts->session_name);
    if(check_directory_exists(opts->session_name) == -1) {
      free(full_path);
      opts->session_name = NULL;

    } else {
      opts->session_name = full_path;
    }
  }

  if (opts->session_name == NULL) {
    printf("[mouse-tracker] Asking for a session directory\n");
    run_file_chooser(opts); 
  }

  if (opts->session_name == NULL) {
    printf("[mouse-tracker] No session name provided. Exiting ...\n");
    exit(EXIT_FAILURE);
  }
  
  struct mt_events *events = malloc(sizeof *events);

  if(load_all_chunks(opts->session_name, events) < 0 || events->size ==0) {
    fprintf(stderr, "[mt-plot] No events to display.\n");
    exit(EXIT_FAILURE);
  }

  struct plot_state state = {
    .events = events,
    .events_drawn = 0,
    .scale = 1.0f,
    .cursor_x = 600.0f, // Half of 1200*800 resolution
    .cursor_y = 400.0f, // TODO : Check current resolution
    .off_x = 600.0f,
    .off_y = 400.0f
  };

  GtkApplication *app = gtk_application_new("mousetracker.plot", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(activate_plot), &state);
  g_application_run(G_APPLICATION(app), 0, NULL);
  g_object_unref(app);

  free(events->ev);
  free(events);
}

int is_regular_file(const char *dir, const char* file_name) {
  char *path;
  struct stat st;
  asprintf(&path, "%s/%s", dir, file_name);

  if (stat(path, &st) != 0) {
    fprintf(stderr, "[mouse-tracker] File '%s' does not exist (%s)\n", path, strerror(errno));
    return -1;
  }
  free(path);
  return S_ISREG(st.st_mode);
}

int check_directory_exists(const char *path) {
  struct stat st;

  if (stat(path, &st) != 0) {
    fprintf(stderr, "[mouse-tracker] Session '%s' does not exist (%s)\n", path, strerror(errno));
    return -1;
  }

  if (!S_ISDIR(st.st_mode)) {
    fprintf(stderr, "[mouse-tracker] '%s' is not a directory\n", path);
    return -1;
  }

  if (access(path, R_OK | X_OK) != 0) {
    fprintf(stderr, "[mouse-tracker] No read access to '%s'\n", path);
    return -1;
  }

  return 0; 
}

