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

	.globl __start__stack
;--------------------------------------------------------
; Stack segment in internal ram
;--------------------------------------------------------
	.area	SSEG	(DATA)
__start__stack:
	.ds	1

;--------------------------------------------------------
; interrupt vector
; You have to make sure the VECTOR area ends up at flash addr 0.
; e.g. by putting start.rel as the first object in the linker
; command line, or (better) with -bVECTOR=0
;--------------------------------------------------------
	.area VECTOR    (CODE)
	.globl __interrupt_vect
__interrupt_vect:

; Code address 0 is the reset vector - jump to the sdcc init routine,
; which will ultimately jump back to __sdcc_program_startup

	ljmp	__sdcc_gsinit_startup

; That ljmp instruction took up 3 bytes, so absolute address is now 0x0003.
; This is the start of the interrupt vector table, see SWRS033H page 61.
; The CC1110 / 8051 doesn't natively support relocating the interrupt vector
; table, so any interrupts that fire when the app is running will end up back
; here.  Most interrupts aren't used by the bootloader, so we just forward them
; to the corresponding location in the app section.
; A couple are dual-use (app or bootloader), so we intercept them.

	ljmp #(0x0400+0x03)
	.ds	5
	ljmp #(0x0400+0x0B)
	.ds	5
	ljmp #(0x0400+0x13)
	.ds	5
	ljmp uart1_rx_isr_forward
	.ds	5
	ljmp #(0x0400+0x23)
	.ds	5
	ljmp #(0x0400+0x2B)
	.ds	5
	ljmp #(0x0400+0x33)
	.ds	5
	ljmp #(0x0400+0x3B)
	.ds	5
	ljmp #(0x0400+0x43)
	.ds	5
	ljmp #(0x0400+0x4B)
	.ds	5
	ljmp #(0x0400+0x53)
	.ds	5
	ljmp #(0x0400+0x5B)
	.ds	5
	ljmp #(0x0400+0x63)
	.ds	5
	ljmp #(0x0400+0x6B)
	.ds	5
	ljmp #(0x0400+0x73)
	.ds	5
	ljmp #(0x0400+0x7B)
	.ds	5
	ljmp rf_isr_forward
	.ds	5
	ljmp #(0x0400+0x8B)
	.ds	5

; Several interrupts are used by either bootloader or app.
; We use the user-defined bit 1 of the Program Status Word (PSW.F1)
; to determine whether we're in the bootloader or the app.
; It's set to 1 in main.c when we're in the bootloader, and
; cleared to 0 just before we jump into the app.


uart1_rx_isr_forward:
	jnb psw.1, 00001$
	ljmp _uart1_rx_isr ; We're in the bootloader
00001$:
	ljmp #(0x0400+0x1B) ; We're in the app


rf_isr_forward:
	jnb psw.1, 00002$
	ljmp _rf_isr ; We're in the bootloader
00002$:
	ljmp #(0x0400+0x83) ; We're in the app

; We have to put a couple of helper routines for the flash memory programming
; in VECTOR, so that they have a well-defined location due to some weirdness
; with the CC1110 flash peripheral.
		.area APP_UPDATER (CODE)
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

;--------------------------------------------------------
; external initialized ram data
;--------------------------------------------------------
	.area XISEG   (XDATA)
	.area HOME    (CODE)
	.area GSINIT0 (CODE)
	.area GSINIT1 (CODE)
	.area GSINIT2 (CODE)
	.area GSINIT3 (CODE)
	.area GSINIT4 (CODE)
	.area GSINIT5 (CODE)
	.area GSINIT  (CODE)
	.area GSFINAL (CODE)
	.area CSEG    (CODE)

;--------------------------------------------------------
; global & static initialisations
;--------------------------------------------------------
	
	.area GSINIT  (CODE)
	.globl __sdcc_gsinit_startup
	.globl __sdcc_program_startup
	.globl __start__stack
	.globl __mcs51_genXINIT
	.globl __mcs51_genXRAMCLEAR
	.globl __mcs51_genRAMCLEAR
	.area GSFINAL (CODE)
	.globl __sdcc_program_startup
	ljmp	__sdcc_program_startup
;--------------------------------------------------------
; Home
;--------------------------------------------------------
	.area HOME    (CODE)
	.area HOME    (CODE)
__sdcc_program_startup:
	lcall	_bootloader_main
	;	return from main will lock up
	sjmp .
