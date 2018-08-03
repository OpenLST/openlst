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

#ifndef _INNTERUPTS_H
#define _INNTERUPTS_H

// This flag bit in the Program Status Word (PSW) is used to tell the
// interrupt trampoline in start.asm that the bootloader is running
// rather than the app.
#define IN_BOOTLOADER_FLAG F1

#define INTERRUPT_GLOBAL_ENABLE  __asm \
    setb _EA \
    __endasm
#define INTERRUPT_GLOBAL_DISABLE __asm \
    clr _EA \
    __endasm

#endif
