; OpenLST
; Copyright (C) 2018 Planet Labs Inc.
; 
; This program is free software: you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation, either version 3 of the License, or
; (at your option) any later version.
; 
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
; 
; You should have received a copy of the GNU General Public License
; along with this program.  If not, see <https://www.gnu.org/licenses/>.

; We have to put a couple of helper routines for the flash memory programming
; in FLASHTRIGSEG, so that they have a well-defined location due to some
; weirdness with the CC1110 flash peripheral.

        .area FLASHTRIGSEG    (CODE)
        .globl _flash_trigger_write
        .even ; Code that initiates the flash write op must be even-aligned
              ; to avoid undefined behavior (SWRS033H page 86)
_flash_trigger_write:
        .globl _FCTL ; import FCTL symbol
        MOV _FCTL, #0x02
        NOP   ; maybe necessary by analogy to NOP in flash_trigger_erase
        RET

        .globl _flash_trigger_erase
        .even ; Code that initiates the flash erase op must be even-aligned
              ; to avoid undefined behavior (not mentioned in CC docs, but
              ; definitely real)
_flash_trigger_erase:
        MOV _FCTL, #0x01
        NOP   ; necessary to avoid undefined behavior (SWRS033H page 88)
        RET
