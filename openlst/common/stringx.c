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

/*
Lightweight versions of some of the standard string library functions.

By taking advantage of the CC111x's unified memory space, these can be
smaller and typically faster than the sdcc versions which use generic pointers.

They are limited to operating on buffers <= 255 bytes long.
 */
#ifdef BOOTLOADER
#pragma codeseg APP_UPDATER
#endif
#include "./stringx.h"

/* Copy memory area */
void memcpyx (char __xdata *dst, const char __xdata *src, unsigned char acount) {
	// copy from lower addresses to higher addresses
	while (acount--) {
		*dst++ = *src++;
	}
}

/* Clear/set memory area */
void memsetx (char __xdata *dst, unsigned char value , unsigned char acount) {
	// copy from lower addresses to higher addresses
	while (acount--) {
		*dst++ = value;
	}
}

#if KEEP_CODE_SMALL == 0
/* Compare memory areas */
__bit memcmpx (const char __xdata *a, const char __xdata *b, unsigned char n) {
	while (n--) {
		if (*a++ != *b++) return 1;
	}
	return 0;
}
#endif

/* Compare memory areas in constant time (for security purposes) */
__bit memcmpx_ct (const char __xdata *a, const char __xdata *b, unsigned char n) {
	char acc = 0;
	while(n--) {
		/* Any nonequal bytes will accumulate some 1 bits into acc. */
		acc |= *a++ ^ *b++;
	}
	return acc;
}

/* Return the string length */
uint8_t strcpylenx(__xdata char *dst, __xdata char *src) {
	uint8_t l = 0;
	while (dst[l] = src[l]) {
		l++;
	}
	return l;
}
