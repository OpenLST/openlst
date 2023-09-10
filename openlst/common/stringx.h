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

#ifndef __STRINGX_H__
#define __STRINGX_H__

#include <stdint.h>

#define MEMCPYX(d, s, n) memcpyx((__xdata char *)(d), (__xdata const char *)(s), n)
#define MEMCMPX(d, s, n) memcmpx((__xdata const char *)(d), (__xdata const char *)(s), n)

void memcpyx  (char __xdata *dst, const char __xdata *src, unsigned char acount);
void memsetx (char __xdata *dst, unsigned char value , unsigned char acount);
#if KEEP_CODE_SMALL == 0
__bit memcmpx (const __xdata char *a, const __xdata char *b, unsigned char n);
#endif
__bit memcmpx_ct (const __xdata char *a, const __xdata char *b, unsigned char n);
uint8_t strcpylenx(__xdata char *dst, __xdata char *src);

#define XDATA_ALIAS(p) ((__xdata void *)(0xFF00 | (unsigned char)&p))

#endif
