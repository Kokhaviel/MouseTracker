/* 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Adrien Golinelli <golinelli.adrien@gmail.com>
 */
#pragma once 

#define MT_DIR ".mouse-tracker"

void acquire_instance_lock(char *lock_dir);
int read_locking_pid(int fd);
void stop_existing_instance(pid_t pid);
