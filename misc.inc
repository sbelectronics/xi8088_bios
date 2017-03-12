;=========================================================================
; misc.inc - Miscellaneous BIOS Services
;       INT 15h, functions:
;       	4Fh	- OS hook keyboard intercept
;		90h	- Device busy loop
;		91h	- Interrupt completed
;		0C0h	- Get system configruation
;		0C2h	- PS/2 mouse services (see ps2aux.inc)
;		
;-------------------------------------------------------------------------
;
; Compiles with NASM 2.07, might work with other versions
;
; Copyright (C) 2011 - 2012 Sergey Kiselev.
; Provided for hobbyist use on the Xi 8088 board.
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

;-------------------------------------------------------------------------
; offsets for registers on stack
int_15_bp	equ	0
int_15_ip	equ	int_15_bp+2
int_15_cs	equ	int_15_ip+2
int_15_flags	equ	int_15_cs+2

;=========================================================================
; int_15 - Miscellaneous BIOS services
; Input:
;	AH = 4Fh - OS hook keyboard intercept
;		- Does nothing
;	AH = 84h - read joystick
;	AH = 0C2h - PS/2 mouse services
;		- Implemented in ps2aux.inc
;-------------------------------------------------------------------------
	setloc	0F859h			; INT 15 Entry Point
int_15:
	sti
	cmp	ah,4Fh
	je	int_15_exit		; continue with int 09h ISR
	cmp	ah,084h
	je	int_15_fn84
	cmp	ah,0C0h
	je	int_15_fnC0
	cmp	ax,9001h
	je	int_15_os_hook		; diskette - device busy hook
	cmp	ax,90FDh
	je	int_15_os_hook		; diskette - motor start hook
	cmp	ax,9101h
	je	int_15_os_hook		; diskette - interrupt completed

%ifdef PS2_MOUSE
	cmp	ah,0C2h
	jne	.1
	jmp	int_15_fnC2		; jump to PS/2 mouse handler
.1:
%endif

	mov	ah,86h			; no cassete present

int_15_err:
	push	bp
	mov	bp,sp
	or	byte [bp+int_15_flags],1 ; set CF flag
	pop	bp
	iret

int_15_ok:
	push	bp
	mov	bp,sp
	and	byte [bp+int_15_flags],~1 ; clear CF flag
	pop	bp
	iret

int_15_os_hook:
	mov	ah,00h

int_15_exit:
	iret

;=========================================================================
; int_15_fnC0 - Get configuration
; Input:
;	AH = 0C0h - get configuration
; Output:
;	AH = 00h - function supported
;	ES:BX = configuration table (0F000h:0E6F5h)
;	CF = 0 (success)
;-------------------------------------------------------------------------
int_15_fnC0:
	mov	ah,00h
	mov	bx,bioscseg
	mov	es,bx
	mov	bx,config_table
	jmp	int_15_ok

;=========================================================================
; int_15_fn84 - Get joystick
; Input:
;	AH = 084h - get configuration
;       DX = 0 = read switches, 1 = read pots
; Output:
;	INPUT							:
;	(DX)=0 READ THE CURRENT SWITCHES			:
;	       RETURNS (AL)= SWITCH SETTINGS IN BITS 7-4	:
;								:
;	(DX)=1	READ THE RESISTIVE INPUTS			:
;		RETURNS (AX)=A(x) VALUE 			:
;			(BX)=A(y) VALUE 			:
;			(CX)=B(x) VALUE 			:
;			(DX)=B(y) VALUE 			:
;
; Note: adapted from IBM PC BIOS SOURCE CODE reconstruction
;       https://sites.google.com/site/pcdosretro/ibmpcbios
;-------------------------------------------------------------------------

int_15_fn84:
	MOV	AX,DX			; GET SUB FUNCTION CODE
	MOV	DX,201H 		; ADDRESS OF PORT
	OR	AL,AL
	JZ	JOY_2			; READ SWITCHES
	DEC	AL
	JZ	JOY_3			; READ RESISTIVE INPUTS
	JMP	int_15_err		; GO TO ERROR RETURN
JOY_1:
	STI
        JMP     int_15_ok               ; GO TO COMMON RETURN

JOY_2:
	IN	AL,DX
	AND	AL,0F0H 		; STRIP UNWANTED BITS OFF
	JMP	JOY_1			; FINISHED

JOY_3:
	MOV	BL,1
	CALL	TEST_CORD
	PUSH	CX			; SAVE A(X) VALUE
	MOV	BL,2
	CALL	TEST_CORD
	PUSH	CX			; SAVE A(Y) VALUE
	MOV	BL,4
	CALL	TEST_CORD
	PUSH	CX			; SAVE B(X) VALUE
	MOV	BL,8
	CALL	TEST_CORD
	MOV	DX,CX			; SAVE B(Y) VALUE
	POP	CX			; GET B(X) VALUE
	POP	BX			; GET A(Y) VALUE
	POP	AX			; GET A(X) VALUE
	JMP	JOY_1			; FINISHED - RETURN

TEST_CORD:
	PUSH	DX			; SAVE
	CLI				; BLOCK INTERRUPTS WHILE READING
	MOV	AL,0			; SET UP TO LATCH TIMER 0
	OUT	pit_ch0_reg+3,AL
	JMP	$+2
	IN	AL,pit_ch0_reg		; READ LOW BYTE OF TIMER 0
	JMP	$+2
	MOV	AH,AL
	IN	AL,pit_ch0_reg		; READ HIGH BYTE OF TIMER 0
	XCHG	AH,AL			; REARRANGE TO HIGH,LOW
	PUSH	AX			; SAVE
	MOV	CX,4FFH 		; SET COUNT
	OUT	DX,AL			; FIRE TIMER
	JMP	$+2
TEST_CORD_1:
	IN	AL,DX			; READ VALUES
	TEST	AL,BL			; HAS PULSE ENDED?
	LOOPNZ	TEST_CORD_1
	CMP	CX,0
	POP	CX			; ORIGINAL COUNT
	JNZ	SHORT TEST_CORD_2
	SUB	CX,CX			; SET 0 COUNT FOR RETURN
	JMP	SHORT TEST_CORD_3	; EXIT WITH COUNT = 0
TEST_CORD_2:
	MOV	AL,0			; SET UP TO LATCH TIMER 0
	OUT	pit_ch0_reg+3,AL
	JMP	$+2
	IN	AL,pit_ch0_reg		; READ LOW BYTE OF TIMER 0
	MOV	AH,AL
	JMP	$+2
	IN	AL,pit_ch0_reg		; READ HIGH BYTE OF TIMER 0
	XCHG	AH,AL			; REARRANGE TO HIGH,LOW

	CMP	CX,AX			; CHECK FOR COUNTER WRAP
	JAE	TEST_CORD_4		; GO IF NO
	PUSH	DX
	MOV	DX,-1

	SUB	DX,AX			; ADJUST FOR WRAP
	ADD	CX,DX
	POP	DX
	JMP	SHORT TEST_CORD_5

TEST_CORD_4:
	SUB	CX,AX
TEST_CORD_5:
	AND	CX,1FF0H		; ADJUST
	SHR	CX,1                    ; SHR CX,4 not supported by 8088
	SHR	CX,1
	SHR	CX,1
	SHR	CX,1

TEST_CORD_3:
	STI				; INTERRUPTS BACK ON
	MOV	DX,201H 		; FLUSH OTHER INPUTS
	PUSH	CX
	PUSH	AX
	MOV	CX,4FFH 		; COUNT
TEST_CORD_6:
	IN	AL,DX
	TEST	AL,0FH
	LOOPNZ	TEST_CORD_6

	POP	AX
	POP	CX
	POP	DX			; SET COUNT

	RET				; RETURN
