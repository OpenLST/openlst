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

#ifndef _WATCHDOG_H
#define _WATCHDOG_H

#define WATCHDOG_ENABLE \
	__asm \
	mov _WDCTL, #0x08 \
	__endasm;

#define WATCHDOG_CLEAR  \
	__asm \
      mov _WDCTL, #0xA8 \
      mov _WDCTL, #0x58 \
   __endasm;

void watchdog_reboot_now(void) __naked;

#endif
