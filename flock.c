/* 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Adrien Golinelli <golinelli.adrien@gmail.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <fcntl.h>

#include "flock.h"

// Acquires a file lock to enforce single-instance execution
void acquire_instance_lock(char* lock_dir) {

  char *lock_file;
  mkdir(lock_dir, 0755);
  asprintf(&lock_file, "%s/mt.lock", lock_dir); // lock_dir -> lock_file

  int fd = open(lock_file, O_RDWR | O_CREAT, 0644); // Creates ~/.mouse-tracker/mt.lock
  free(lock_file);
  if (fd < 0) {
    perror("Unable to acquire file lock");
    exit(EXIT_FAILURE);
  }

  if (flock(fd, LOCK_EX | LOCK_NB) < 0) { 
    fprintf(stderr, "[mouse-tracker] another instance is already running\n");
    stop_existing_instance(read_locking_pid(fd));
  }

  ftruncate(fd, 0);
  dprintf(fd, "%d\n", getpid()); // Write current PID if no other instance
  close(fd);
}

// Send SIGTERM to the other instance and exit this one
// SIGTERM (from second instance or ^C) is caught in main 
void stop_existing_instance(pid_t pid) {
  if (pid > 0) {
    fprintf(stderr, "[mouse-tracker] sending SIGTERM to pid=%d\n", pid);
    kill(pid, SIGTERM);
  }

  exit(EXIT_SUCCESS);
}

// Read the pid holding the lock 
pid_t read_locking_pid(int fd) {
  char buf[32];
  lseek(fd, 0, SEEK_SET);
  int n = read(fd, buf, sizeof(buf)-1);
  if (n <= 0) return -1;
  buf[n] = 0;
  return (pid_t) atoi(buf);
}

