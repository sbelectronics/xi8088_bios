;=========================================================================
; config.inc - Compilation time settings and settings
;-------------------------------------------------------------------------
;
; Compiles with NASM 2.07, might work with other versions
;
; Copyright (C) 2011 Sergey Kiselev.
; Provided for hobbyist use on the Sergey's XT board.
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
; along with this program.  If not, see <http://www.gnu.org/licenses/>.
;
;=========================================================================

%define AT_COMPAT			; AT-compatible system, two PICs
%define PS2_MOUSE			; Enable PS/2 auxiliary device support
%define DISABLE_KBD_DURING_INTERRUPTS	; Don't disable keyboard in INT1
%define MIN_RAM_SIZE 32			; At least 32 KiB to boot the system
%define MAX_RAM_SIZE 640		; Scan this much memory during POST
%define RAM_TEST_BLOCK 16384		; block size for RAM test
%define EBDA_SIZE 1			; 1KB reserved for EBDA
					; EBDA is required for PS/2 aux support
%define SLOW_FLOPPY			; turn off turbo during floppy I/O