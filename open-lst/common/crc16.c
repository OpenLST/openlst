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

#ifdef BOOTLOADER
#pragma codeseg APP_UPDATER
#endif
#include <cc1110.h>
#include "crc16.h"

// TODO: cleanup
uint16_t crc16(__xdata uint8_t *data, uint16_t len) {
  // Need to set ADCCON1.RCTRL?
  RNDL = 0xFF;
  RNDL = 0xFF;
  while (len--)
    RNDH = *data++;
  return RNDH << 8 | RNDL;
}
