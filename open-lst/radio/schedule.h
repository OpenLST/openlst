// OpenLST
// Copyright (C) 2018 Planet Labs Inc.
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#ifndef _SCHEDULE_H
#define _SCHEDULE_H
#include <stdint.h>

#define SCHEDULE_REBOOT_POSTPONED 0
#define SCHEDULE_REBOOT_TOO_LONG  1

void schedule_init(void);
uint8_t schedule_postpone_reboot(uint32_t postpone);
void schedule_handle_events(void);

#endif