/*
 *  System specific stuff
 *
 *  Copyright (C) 2008 Andreas Öman
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <stdint.h>
#include <time.h>

void arch_exit(void) __attribute__((noreturn));

int64_t arch_cache_avail_bytes(void);

int arch_pipe(int pipefd[2]);

// If arch_stop_req() returns non ozer it will not actually
// schedule an exit of showtime but rather suspend the UI and turn off 
// video output, etc
//
// Typically used on some targets where we want to enter a 
// semi-standby state.

int arch_stop_req(void);