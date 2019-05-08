;-------------------------------------------------------------------------------
; Name: Cpu.asm
; Desc: The cpu core written in assembly.
;-------------------------------------------------------------------------------

.486
.MODEL FLAT


; Public symbols so other files may use these functions.
PUBLIC RunCPU


; Constants
RUNCPU_STEP EQU 01h
RUNCPU_RUN  EQU 02h


; The CPU structure from Cpu.h
_NES6502 STRUCT 4
	A              DB ?             ; The Accumulator register on the 6502.
	X              DB ?             ; The X Index register on the 6502.
	Y              DB ?             ; The Y Index register on the 6502.
	S              DB ?             ; The stack pointer on the 6502.
	F              DB ?             ; The flags register on the 6502.
	P              DW ?             ; The program counter on the 6502.
	Memory         DB 08000h dup(?) ; All the memory on the NES except the PRG-ROM.
	pbyPRGROMBank1 DD ?             ; Points to the first PRG-ROM bank on the NES cartridge.
	pbyPRGROMBank2 DD ?             ; Points to the second PRG-ROM bank on the NES cartridge.
	byCycles       DB ?             ; Number of cycles left until the end of the scanline.
_NES6502 ENDS


; External variables/functions needed.
.FARDATA?
EXTRN ?CPU@@3UtagNES6502@@A       :_NES6502
EXTRN ?GetMemoryByte@@YGEG@Z      :PROC
EXTRN ?GetMemoryPointer@@YGPAEG@Z :PROC
EXTRN ?ReadMemory@@YGEG@Z         :PROC
EXTRN ?WriteMemory@@YGXGE@Z       :PROC

CPU EQU ?CPU@@3UtagNES6502@@A



.DATA



; Holds the address of all the instructions. This way,
; whatever the opcode is, we immediately know where to
; jump to execute it.
adwOpcodeJumpTable dd  _00, _01, _??, _??, _??, _05, _06, _??
                   dd  _08, _09, _0A, _??, _??, _0D, _0E, _??
                   dd  _10, _11, _??, _??, _??, _15, _16, _??
                   dd  _18, _19, _??, _??, _??, _1D, _1E, _??
                   dd  _20, _21, _??, _??, _24, _25, _26, _??
                   dd  _28, _29, _2A, _??, _2C, _2D, _2E, _??
                   dd  _30, _31, _??, _??, _??, _35, _36, _??
                   dd  _38, _39, _??, _??, _??, _3D, _3E, _??
                   dd  _40, _41, _??, _??, _??, _45, _46, _??
                   dd  _48, _49, _4A, _??, _4C, _4D, _4E, _??
                   dd  _50, _51, _??, _??, _??, _55, _56, _??
                   dd  _58, _59, _??, _??, _??, _5D, _5E, _??
                   dd  _60, _61, _??, _??, _??, _65, _66, _??
                   dd  _68, _69, _6A, _??, _6C, _6D, _6E, _??
                   dd  _70, _71, _??, _??, _??, _75, _76, _??
                   dd  _78, _79, _??, _??, _??, _7D, _7E, _??
                   dd  _??, _81, _??, _??, _84, _85, _86, _??
                   dd  _88, _??, _8A, _??, _8C, _8D, _8E, _??
                   dd  _90, _91, _??, _??, _94, _95, _96, _??
                   dd  _98, _99, _9A, _??, _??, _9D, _??, _??
                   dd  _A0, _A1, _A2, _??, _A4, _A5, _A6, _??
                   dd  _A8, _A9, _AA, _??, _AC, _AD, _AE, _??
                   dd  _B0, _B1, _??, _??, _B4, _B5, _B6, _??
                   dd  _B8, _B9, _BA, _??, _BC, _BD, _BE, _??
                   dd  _C0, _C1, _??, _??, _C4, _C5, _C6, _??
                   dd  _C8, _C9, _CA, _??, _CC, _CD, _CE, _??
                   dd  _D0, _D1, _??, _??, _??, _D5, _D6, _??
                   dd  _D8, _D9, _??, _??, _??, _DD, _DE, _??
                   dd  _E0, _E1, _??, _??, _E4, _E5, _E6, _??
                   dd  _E8, _E9, _??, _??, _EC, _ED, _EE, _??
                   dd  _F0, _F1, _??, _??, _??, _F5, _F6, _??
                   dd  _F8, _F9, _??, _??, _??, _FD, _FE, _??


; Number of cycles for each opcode. 
abyOpcodeCycles db  7, 6, 0, 0, 0, 3, 5, 0, 3, 2, 2, 0, 0, 4, 6, 0
                db  2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0
                db  6, 6, 0, 0, 3, 3, 5, 0, 4, 2, 2, 0, 4, 4, 6, 0
                db  2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0
                db  13, 6, 0, 0, 0, 3, 5, 0, 3, 2, 2, 0, 3, 4, 6, 0
                db  2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0
                db  6, 6, 0, 0, 0, 3, 5, 0, 4, 2, 2, 0, 5, 4, 6, 0
                db  2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0
                db  0, 6, 0, 0, 3, 3, 3, 0, 2, 0, 2, 0, 4, 4, 4, 0
                db  2, 6, 0, 0, 4, 4, 4, 0, 2, 5, 2, 0, 0, 5, 0, 0
                db  2, 6, 2, 2, 3, 3, 3, 2, 2, 2, 2, 2, 4, 4, 4, 2
                db  2, 5, 2, 2, 4, 4, 4, 2, 2, 4, 2, 2, 4, 4, 4, 2
                db  2, 6, 2, 2, 3, 3, 5, 2, 2, 2, 2, 2, 4, 4, 6, 2
                db  2, 5, 2, 2, 2, 4, 6, 2, 2, 4, 2, 2, 2, 4, 7, 2
                db  2, 6, 2, 2, 3, 3, 5, 2, 2, 2, 2, 2, 4, 4, 6, 2
                db  2, 5, 2, 2, 2, 4, 6, 2, 2, 4, 2, 2, 2, 4, 7, 2
    

; The number of bytes for each opcode. The default length is one
; so the PC will increment if a bad opcode or a NOP is executed
abyOpcodeBytes db 0, 2, 1, 1, 1, 2, 2, 1, 1, 2, 1, 1, 1, 3, 3, 1
               db 2, 2, 1, 1, 1, 2, 2, 1, 1, 3, 1, 1, 1, 3, 3, 1
               db 0, 2, 1, 1, 2, 2, 2, 1, 1, 2, 1, 1, 3, 3, 3, 1
               db 2, 2, 1, 1, 1, 2, 2, 1, 1, 3, 1, 1, 1, 3, 3, 1
               db 0, 2, 1, 1, 1, 2, 2, 1, 1, 2, 1, 1, 0, 3, 3, 1
               db 2, 2, 1, 1, 1, 2, 2, 1, 1, 3, 1, 1, 1, 3, 3, 1
               db 1, 2, 1, 1, 1, 2, 2, 1, 1, 2, 1, 1, 0, 3, 3, 1
               db 2, 2, 1, 1, 1, 2, 2, 1, 1, 3, 1, 1, 1, 3, 3, 1
               db 1, 2, 1, 1, 2, 2, 2, 1, 1, 1, 1, 1, 3, 3, 3, 1
               db 2, 2, 1, 1, 2, 2, 2, 1, 1, 3, 1, 1, 1, 3, 1, 1
               db 2, 2, 2, 1, 2, 2, 2, 1, 1, 2, 1, 1, 3, 3, 3, 1
               db 2, 2, 1, 1, 2, 2, 2, 1, 1, 3, 1, 1, 3, 3, 3, 1
               db 2, 2, 1, 1, 2, 2, 2, 1, 1, 2, 1, 1, 3, 3, 3, 1
               db 2, 2, 1, 1, 1, 2, 2, 1, 1, 3, 1, 1, 1, 3, 3, 1
               db 2, 2, 1, 1, 2, 2, 2, 1, 1, 2, 1, 1, 3, 3, 3, 1
               db 2, 2, 1, 1, 1, 2, 2, 1, 1, 3, 1, 1, 1, 3, 3, 1


; Used to set the zero and the sign flags.  
; Just a lookup table for all the possible values.
abyZNTable db 02h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h
           db 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h
           db 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h
           db 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h
           db 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h
           db 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h
           db 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h
           db 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h
           db 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h
           db 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h
           db 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h
           db 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h
           db 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h
           db 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h
           db 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h
           db 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h, 80h




.CODE



;******************************************************************************
;*                                                                            *
;* Macros for reading/writing memory                                          *
;*                                                                            *
;******************************************************************************

GET_MEMORY_BYTE MACRO wParam:REQ
	xor edx, edx
	mov dx, wParam
	push edx
	call ?GetMemoryByte@@YGEG@Z
	and eax, 000000FFh
ENDM

GET_MEMORY_POINTER MACRO wParam:REQ
	xor edx, edx
	mov dx, wParam
	push edx
	call ?GetMemoryPointer@@YGPAEG@Z
ENDM

READ_MEMORY MACRO wParam:REQ
	movzx edx, wParam
	push edx
	call ?ReadMemory@@YGEG@Z
	and eax, 000000FFh
ENDM

; NOTE: Since I don't know enough about writting macros to make
; sure that wParam isn't edx, you'll just have to make sure
; you never call this macro with edx as the wParam or it
; will not work right.
WRITE_MEMORY MACRO wParam:REQ, byData:REQ
	movzx edx, byData
	push edx
	movzx edx, wParam
	push edx
	call ?WriteMemory@@YGXGE@Z
ENDM

; Push a byte onto the NES's stack.
PUSH_BYTE MACRO byData:REQ
	; The stacks address space is 100h-1FFh
	mov cl, CPU.S
	mov ch, 1
	WRITE_MEMORY cx, byData
	dec CPU.S
ENDM

; Pop a byte from the NES's stack.
POP_BYTE MACRO byDest:REQ
	; The stacks address space is 100h-1FFh
	inc CPU.S
	mov cl, CPU.S
	mov ch, 1
	GET_MEMORY_BYTE cx
	mov byDest, al
ENDM


;******************************************************************************
;*                                                                            *
;* Macro for setting/clearing the flags register on the 6502                  *
;******************************************************************************

MODIFYFLAGS MACRO Sign, Overflow, Zero, Carry

	; Clear all the flags if they are passed.
	pushfd
	IFNB <Sign>
		and CPU.F, 01111111b
	ENDIF
	IFNB <Overflow>
		and CPU.F, 10111111b
	ENDIF
	IFNB <Zero>
		and CPU.F, 11111101b
	ENDIF
	IFNB <Carry>
		and CPU.F, 11111110b
	ENDIF
	popfd
	
	; Set the flags that are passed to the macro.
	
	IFNB <Sign>
		pushfd
		sets dl
		shl dl, 7
		or CPU.F, dl
		popfd
	ENDIF
	IFNB <Overflow>
		pushfd
		seto dl
		shl dl, 6
		or CPU.F, dl
		popfd
	ENDIF
	IFNB <Zero>
		pushfd
		setz dl
		shl dl, 1
		or CPU.F, dl
		popfd
	ENDIF
	IFNB <Carry>
		pushfd
		setc dl
		or CPU.F, dl
		popfd
	ENDIF
ENDM


;******************************************************************************
;*                                                                            *
;*                        Addressing mode macros                              *
;*                                                                            *
;******************************************************************************

;1) Immediate
;
;   In this mode the operand's value is given in the instruction itself. In
;   assembly language this is indicated by "#" before the operand.
;   eg.  LDA #$0A - means "load the accumulator with the hex value 0A"
;   In machine code different modes are indicated by different codes. So LDA
;   would be translated into different codes depending on the addressing mode.
;   In this mode, it is: $A9 $0A

GET_ADDR_IMMEDIATE MACRO
	; Move the PC address up by one.
	mov di, CPU.P
	inc di

	; Now get the memory byte to be used as immediate data.
	GET_MEMORY_BYTE di
ENDM

;2 & 3) Absolute and Zero-page Absolute
;  In these modes the operands address is given.
;  eg.  LDA $31F6 - (assembler)
;       $AD $31F6 - (machine code)
;  If the address is on zero page - i.e. any address where the high byte is
;  00 - only 1 byte is needed for the address. The processor automatically
;  fills the 00 high byte.
;  eg.  LDA $F4
;       $A5 $F4
;  Note the different instruction codes for the different modes.
;  Note also that for 2 byte addresses, the low byte is store first, eg.
;  LDA $31F6 is stored as three bytes in memory, $AD $F6 $31.
;  Zero-page absolute is usually just called zero-page.

GET_ADDR_ZEROPAGE MACRO
	; Increment the PC address.
	mov di, CPU.P
	inc di

	; Get the byte that is the address of the byte to be read.
	GET_MEMORY_BYTE di
ENDM

GET_ADDR_ABSOLUTE MACRO
	mov di, CPU.P

	; Get the low address byte.
	inc di
	GET_MEMORY_BYTE di
	mov bl, al

	; Get the high address byte.
	inc di
	GET_MEMORY_BYTE di
	mov bh, al
ENDM

; 4) Implied
; No operand addresses are required for this mode. They are implied by the
; instruction.
; eg.  TAX - (transfer accumulator contents to X-register)
;      $AA
;
; 5) Accumulator
; In this mode the instruction operates on data in the accumulator, so no
; operands are needed.
; eg.  LSR - logical bit shift right
;      $4A

; 6 & 7) Indexed and Zero-page Indexed
; In these modes the address given is added to the value in either the X or
; Y index register to give the actual address of the operand.
; eg.  LDA $31F6, Y
;      $D9 $31F6
;      LDA $31F6, X
;      $DD $31F6
; Note that the different operation codes determine the index register used.
; In the zero-page version, you should note that the X and Y registers are
; not interchangeable. Most instructions which can be used with zero-page
; indexing do so with X only.
; eg.  LDA $20, X
;      $B5 $20

GET_ADDR_ZEROPAGE_X MACRO
    ; Get the normal zero page address
	GET_ADDR_ZEROPAGE

	; Now add the X register to the address.
	add al, CPU.X
ENDM

GET_ADDR_ZEROPAGE_Y MACRO
    ; Get the normal zero page address
	GET_ADDR_ZEROPAGE

	; Now add the Y register to the address.
	add al, CPU.Y
ENDM

GET_ADDR_ABSOLUTE_X MACRO
	; Get the normal absolute page address.
	GET_ADDR_ABSOLUTE

	; Now add the X register to the address.
	xor ax, ax
	mov al, CPU.X
    add bx, ax
ENDM

GET_ADDR_ABSOLUTE_Y MACRO
	; Get the normal absolute page address.
	GET_ADDR_ABSOLUTE

	; Now add the Y register to the address.
	xor ax, ax
	mov al, CPU.Y
    add bx, ax
ENDM


;  8) Indirect
;  This mode applies only to the JMP instruction - JuMP to new location. It is
;  indicated by parenthesis around the operand. The operand is the address of
;  the bytes whose value is the new location.
;  eg.  JMP ($215F)
;  Assume the following -        byte      value
;                                $215F     $76
;                                $2160     $30
;  This instruction takes the value of bytes $215F, $2160 and uses that as the
;  address to jump to - i.e. $3076 (remember that addresses are stored with
;  low byte first).

GET_ADDR_INDIRECT MACRO
	; Get the address of the value to set the PC to.
	GET_ADDR_ABSOLUTE 

	; Get the value to store in the PC register.
	mov di, bx
	GET_MEMORY_BYTE di
	mov bl, al
	inc di
	GET_MEMORY_BYTE di
	mov bh, al
ENDM


;  9) Pre-indexed indirect
;  In this mode a zer0-page address is added to the contents of the X-register
;  to give the address of the bytes holding the address of the operand. The
;  indirection is indicated by parenthesis in assembly language.
;  eg.  LDA ($3E, X)
;       $A1 $3E
;  Assume the following -        byte      value
;                                X-reg.    $05
;                                $0043     $15
;                                $0044     $24
;                                $2415     $6E
;
;  Then the instruction is executed by:
;  (i)   adding $3E and $05 = $0043
;  (ii)  getting address contained in bytes $0043, $0044 = $2415
;  (iii) loading contents of $2415 - i.e. $6E - into accumulator
;
;  Note a) When adding the 1-byte address and the X-register, wrap around
;          addition is used - i.e. the sum is always a zero-page address.
;          eg. FF + 2 = 0001 not 0101 as you might expect.
;          DON'T FORGET THIS WHEN EMULATING THIS MODE.
;       b) Only the X register is used in this mode.

GET_ADDR_PREINDEXED MACRO
	; The first part is just like zero-paged x addressing,
	; but lets save the address for later use.
	GET_ADDR_ZEROPAGE_X
	mov di, ax

	; Now that we have the address of our address, we can
	; get our real address. (thats confusing...hehe)
	GET_MEMORY_BYTE di
	mov bl, al
	inc di
	GET_MEMORY_BYTE di
	mov bh, al
ENDM


; 10) Post-indexed indirect
; In this mode the contents of a zero-page address (and the following byte)
; give the indirect addressm which is added to the contents of the Y-register
; to yield the actual address of the operand. Again, inassembly language,
; the instruction is indicated by parenthesis.
; eg.  LDA ($4C), Y
; Note that the parenthesis are only around the 2nd byte of the instruction
; since it is the part that does the indirection.
; Assume the following -        byte       value
;                               $004C      $00
;                               $004D      $21
;                               Y-reg.     $05
;                               $2105      $6D
; Then the instruction above executes by:
; (i)   getting the address in bytes $4C, $4D = $2100
; (ii)  adding the contents of the Y-register = $2105
; (111) loading the contents of the byte $2105 - i.e. $6D into the
;       accumulator.
; Note: only the Y-register is used in this mode.

GET_ADDR_POSTINDEXED MACRO
	; Move to the next byte.
	mov di, CPU.P
	inc di

	; Get the address of our first address.
	GET_MEMORY_BYTE di
	mov di, ax

	; Now get the address to add the y register to.
	GET_MEMORY_BYTE di
	mov bl, al
	inc di
	GET_MEMORY_BYTE di
	mov bh, al

	; Finally add the contents of the y register to the address.
	xor ax, ax
	mov al, CPU.Y
	add bx, ax
ENDM


; 11) Relative
; This mode is used with Branch-on-Condition instructions. It is probably
; the mode you will use most often. A 1 byte value is added to the program
; counter, and the program continues execution from that address. The 1
; byte number is treated as a signed number - i.e. if bit 7 is 1, the number
; given byt bits 0-6 is negative; if bit 7 is 0, the number is positive. This
; enables a branch displacement of up to 127 bytes in either direction.
; eg  bit no.  7 6 5 4 3 2 1 0    signed value          unsigned value
;     value    1 0 1 0 0 1 1 1    -39                   $A7
;     value    0 0 1 0 0 1 1 1    +39                   $27
; Instruction example:
;   BEQ $A7
;   $F0 $A7
; This instruction will check the zero status bit. If it is set, 39 decimal
; will be subtracted from the program counter and execution continues from
; that address. If the zero status bit is not set, execution continues from
; the following instruction.
; Notes:  a) The program counter points to the start of the instruction
; after the branch instruction before the branch displacement is added.
; Remember to take this into account when calculating displacements.
;         b) Branch-on-condition instructions work by checking the relevant
; status bits in the status register. Make sure that they have been set or
; unset as you want them. This is often done using a CMP instruction.
;         c) If you find you need to branch further than 127 bytes, use the
; opposite branch-on-condition and a JMP.

GET_ADDR_RELATIVE MACRO
	; Move the PC address up by one.
	mov di, CPU.P
	inc di

	; Now get the memory byte to be used as a relative address.
	GET_MEMORY_BYTE di
ENDM



;******************************************************************************
;*                                                                            *
;* Instruction macros                                                         *
;*                                                                            *
;******************************************************************************



; ********************************************************************
; *          ADC. Opcodes 69, 65, 75, 6D, 7D, 79, 61, 71             *
; ********************************************************************
CPU_ADC MACRO Operand:REQ
	; Move the NES's carry flag into the real cpu's carry flag.
	mov dl, CPU.F
	shr dl, 1
	
	; Now add the operand to the A register with the carry flag.
	adc CPU.A, Operand
	MODIFYFLAGS S, V, Z, C
ENDM

; ********************************************************************
; *          AND. Opcodes 29, 25, 35, 2D, 3D, 39, 21, 31             *
; ********************************************************************

CPU_AND MACRO Operand:REQ
	; Perform the operation.
	and CPU.A, Operand
	
	; Test and set/clear the sign and zero flags.
	MODIFYFLAGS S, , Z,
ENDM

; ********************************************************************
; *          ASL. Opcodes 0A, 06, 16, 0E, 1E                         *
; ********************************************************************
CPU_ASL MACRO Operand:REQ
	; Perform the operation.
	shl BYTE PTR [Operand], 1
	MODIFYFLAGS , , , C
	
	mov al, [Operand]
	test al, al
	
	; Test and set/clear the flags.
	MODIFYFLAGS S, , Z, 
ENDM

; ********************************************************************
; *          BCC. Opcode 90                                          *
; ********************************************************************
CPU_BCC MACRO Operand:REQ
	; Test the carry flag.
	test CPU.F, 1
	
	; If the carry flag is clear, then branch.
	.IF ZERO?
		; If the operand is negative we subtract it from the PC,
		; otherwise we need to add it to the PC.
		.IF Operand & 10000000b
			mov di, 0100h
			sub di, Operand
			sub CPU.P, di
		.ELSE
			add CPU.P, Operand
		.ENDIF
	.ENDIF
ENDM

; ********************************************************************
; *          BCS. Opcode B0                                          *
; ********************************************************************
CPU_BCS MACRO Operand:REQ
	; Test the carry flag.
	test CPU.F, 00000001b
	
	; If the carry flag is set, then branch.
	.IF !ZERO?
		; If the operand is negative we subtract it from the PC,
		; otherwise we need to add it to the PC.
		.IF Operand & 10000000b
			mov di, 0100h
			sub di, Operand
			sub CPU.P, di
		.ELSE
			add CPU.P, Operand
		.ENDIF
	.ENDIF
ENDM

; ********************************************************************
; *          BEQ. Opcode F0                                          *
; ********************************************************************
CPU_BEQ MACRO Operand:REQ
	; Test the zero flag.
	test CPU.F, 00000010b
	
	; If the zero flag is set, then branch.
	.IF !ZERO?
		; If the operand is negative we subtract it from the PC,
		; otherwise we need to add it to the PC.
		.IF Operand & 10000000b
			mov di, 0100h
			sub di, Operand
			sub CPU.P, di
		.ELSE
			add CPU.P, Operand
		.ENDIF
	.ENDIF
ENDM

; ********************************************************************
; *          BIT. Opcodes 24, 2C                                     *
; ********************************************************************
CPU_BIT MACRO Operand:REQ
	; Save the operand for later and perform the bit operation
	; which is really just an add instruction.
	mov dl, Operand
	and CPU.A, Operand
	
	; Test and set/clear the zero flag.
	MODIFYFLAGS S, , Z,

	; Bit 7 of the operand is transferred to the sign flag.
	; Bit 6 of the operand is transferred to the overflow flag.
	and dl, 11000000b
	and CPU.F, 00111111b
	or CPU.F, dl
ENDM

; ********************************************************************
; *          BMI. Opcode 30                                          *
; ********************************************************************
CPU_BMI MACRO Operand:REQ
	; Test the sign flag.
	test CPU.F, 10000000b
	
	; If the sign flag is set, then branch.
	.IF !ZERO?
		; If the operand is negative we subtract it from the PC,
		; otherwise we need to add it to the PC.
		.IF Operand & 10000000b
			mov di, 0100h
			sub di, Operand
			sub CPU.P, di
		.ELSE
			add CPU.P, Operand
		.ENDIF
	.ENDIF
ENDM

; ********************************************************************
; *          BNE. Opcode D0                                          *
; ********************************************************************
CPU_BNE MACRO Operand:REQ
	; Test the zero flag.
	test CPU.F, 00000010b
	
	; If the zero flag is clear, then branch.
	.IF ZERO?
		; If the operand is negative we subtract it from the PC,
		; otherwise we need to add it to the PC.
		.IF Operand & 10000000b
			mov di, 0100h
			sub di, Operand
			sub CPU.P, di
		.ELSE
			add CPU.P, Operand
		.ENDIF
	.ENDIF
ENDM

; ********************************************************************
; *          BPL. Opcode 10                                          *
; ********************************************************************
CPU_BPL MACRO Operand:REQ
	; Test the sign flag.
	test CPU.F, 10000000b
	
	; If the sign flag is clear, then branch.
	.IF ZERO?
		; If the operand is negative we subtract it from the PC,
		; otherwise we need to add it to the PC.
		.IF Operand & 10000000b
			mov di, 0100h
			sub di, Operand
			sub CPU.P, di
		.ELSE
			add CPU.P, Operand
		.ENDIF
	.ENDIF
ENDM

; ********************************************************************
; *          BRK. Opcode 00                                          *
; ********************************************************************
CPU_BRK MACRO
	; Push the PC+2 onto the stack high byte first.
	mov bx, CPU.P
	add bx, 2
	PUSH_BYTE bh
	PUSH_BYTE bl
	
	; Push the flags onto the stack.
	PUSH_BYTE CPU.F
	
	; Set the new PC address byte getting the address from the
	; break vector on the PRG-ROM.
	GET_MEMORY_BYTE 0FFFEh
	mov bl, al
	GET_MEMORY_BYTE 0FFFFh
	mov bh, al
	mov CPU.P, bx
	
	; Set the break flag.
	or CPU.F, 00010000b
ENDM

; ********************************************************************
; *          BVC. Opcode 50                                          *
; ********************************************************************
CPU_BVC MACRO Operand:REQ
	; Test the overflow flag.
	test CPU.F, 01000000b
	
	; If the overflow flag is clear, then branch.
	.IF ZERO?
		; If the operand is negative we subtract it from the PC,
		; otherwise we need to add it to the PC.
		.IF Operand & 10000000b
			mov di, 0100h
			sub di, Operand
			sub CPU.P, di
		.ELSE
			add CPU.P, Operand
		.ENDIF
	.ENDIF
ENDM

; ********************************************************************
; *          BVS. Opcode 70                                          *
; ********************************************************************
CPU_BVS MACRO Operand:REQ
	; Test the overflow flag.
	test CPU.F, 01000000b
	
	; If the overflow flag is set, then branch.
	.IF !ZERO?
		; If the operand is negative we subtract it from the PC,
		; otherwise we need to add it to the PC.
		.IF Operand & 10000000b
			mov di, 0100h
			sub di, Operand
			sub CPU.P, di
		.ELSE
			add CPU.P, Operand
		.ENDIF
	.ENDIF
ENDM

; ********************************************************************
; *          CLC. Opcode 18                                          *
; ********************************************************************
CPU_CLC MACRO
	; Clear the carry flag.
	and CPU.F, 11111110b
ENDM

; ********************************************************************
; *          CLD. Opcode D8                                          *
; ********************************************************************
CPU_CLD MACRO
	; Clear the decimal flag.
	and CPU.F, 11110111b
ENDM

; ********************************************************************
; *          CLI. Opcode 58                                          *
; ********************************************************************
CPU_CLI MACRO
	; Clear the interrupt flag.
	and CPU.F, 11111011b
ENDM

; ********************************************************************
; *          CLV. Opcode B8                                          *
; ********************************************************************
CPU_CLV MACRO
	; Clear the overflow flag.
	and CPU.F, 10111111b
ENDM

; ********************************************************************
; *          CMP. Opcodes C9, C5, D5, CD, DD, D9, C1, D1             *
; *          CPX. Opcodes E0, E4, EC                                 *
; *          CPY. Opcodes C0, C4, CC                                 *
; ********************************************************************
CPU_CP? MACRO byReg:REQ, Operand:REQ
	; Compare and reverse the carry flag becuase the 6502 is
	; opposite the Intel when it comes to subtraction.
	cmp byReg, Operand
	cmc
	MODIFYFLAGS S, , Z, C
ENDM

; ********************************************************************
; *          DEC. Opcodes C6, D6, CE, DE                             *
; ********************************************************************
CPU_DEC MACRO pbyMem:REQ
	; Decrement the memory.
	dec BYTE PTR [pbyMem]
	
	; Now set the flags accordingly.
	MODIFYFLAGS S, , Z,
ENDM

; ********************************************************************
; *          DEX. Opcode  CA                                         *
; ********************************************************************
CPU_DEX MACRO
	; Decrement the register.
	dec CPU.X
	
	; Now set the flags accordingly.
	MODIFYFLAGS S, , Z,
ENDM

; ********************************************************************
; *          DEY. Opcode  88                                         *
; ********************************************************************
CPU_DEY MACRO
	; Decrement the register.
	dec CPU.Y
	
	; Now set the flags accordingly.
	MODIFYFLAGS S, , Z,
ENDM

; ********************************************************************
; *          EOR. Opcodes 49, 45, 55, 4D, 5D, 59, 41, 51             *
; ********************************************************************
CPU_EOR MACRO Operand:REQ
	; Perform the operation.
	xor CPU.A, Operand
	
	; Test and set/clear the sign and zero flags.
	MODIFYFLAGS S, , Z,
ENDM

; ********************************************************************
; *          INC. Opcodes E6, F6, EE, FE                             *
; ********************************************************************
CPU_INC MACRO pbyMem:REQ
	; Increment the memory.
	inc BYTE PTR [pbyMem]
	
	; Now set the flags accordingly.
	MODIFYFLAGS S, , Z,
ENDM

; ********************************************************************
; *          INX. Opcode E8                                          *
; ********************************************************************
CPU_INX MACRO
	; Increment the register.
	inc CPU.X
	
	; Now set the flags accordingly.
	MODIFYFLAGS S, , Z,
ENDM

; ********************************************************************
; *          INY. Opcode C8                                          *
; ********************************************************************
CPU_INY MACRO
	; Increment the register.
	inc CPU.Y
	
	; Now set the flags accordingly.
	MODIFYFLAGS S, , Z,
ENDM

; ********************************************************************
; *          JMP. Opcodes 4C, 6C                                     *
; ********************************************************************
CPU_JMP MACRO Operand:REQ
	; Set the new PC address.
	mov CPU.P, Operand
ENDM

; ********************************************************************
; *          JSR. Opcode 20                                          *
; ********************************************************************
CPU_JSR MACRO Operand:REQ
	; Save the operand so it doesn't get overwritten.
	push Operand
	
	; Push the PC+2 onto the stack high byte first.
	mov bx, CPU.P
	add bx, 2
	PUSH_BYTE bh
	PUSH_BYTE bl

	; Restore the operand set the new PC address.
	pop Operand
	mov CPU.P, Operand
ENDM

; ********************************************************************
; *          LDA. Opcodes A9, A5, B5, AD, BD, B9, A1, B1             *
; *          LDX. Opcodes A2, A6, B6, AE, BE                         *
; *          LDY. Opcodes A0, A4, B4, AC, BC                         *
; ********************************************************************
CPU_LD? MACRO byReg:REQ, byData:REQ
	; Move the operand into the A register.
	mov byReg, byData
	mov al, byReg
	test al, al
	
	; Test and Set/Clear the sign and zero flags.
	MODIFYFLAGS S, , Z,
ENDM

; ********************************************************************
; *          LSR. Opcodes 4A, 46, 56, 4E, 5E                         *
; ********************************************************************
CPU_LSR MACRO Operand:REQ
	; Perform the operation.
	shr BYTE PTR [Operand], 1
	MODIFYFLAGS , , , C
	
	mov al, [Operand]
	test al, al
	
	; Test and set/clear the flags.
	MODIFYFLAGS S, , Z, 
ENDM

; ********************************************************************
; *          ORA. Opcodes 09, 05, 15, 0D, 1D, 19, 01, 11             *
; ********************************************************************
CPU_ORA MACRO Operand:REQ
	; Perform the operation.
	or CPU.A, Operand
	
	; Test and set/clear the sign and zero flags.
	MODIFYFLAGS S, , Z,
ENDM

; ********************************************************************
; *          PHA. Opcode 48                                          *
; ********************************************************************
CPU_PHA MACRO
	; Push the A register onto the stack.
	PUSH_BYTE CPU.A
ENDM

; ********************************************************************
; *          PHP. Opcode 08                                          *
; ********************************************************************
CPU_PHP MACRO
	; Push the flags register onto the stack.
	PUSH_BYTE CPU.F
ENDM

; ********************************************************************
; *          PLA. Opcode 68                                          *
; ********************************************************************
CPU_PLA MACRO
	; Pop the A register from the stack.
	POP_BYTE CPU.A
	
	; Now set the flags
	mov al, CPU.A
	test al, al
	MODIFYFLAGS S, , Z, 
ENDM

; ********************************************************************
; *          PLP. Opcode 28                                          *
; ********************************************************************
CPU_PLP MACRO
	; Pop the flags register from the stack.
	POP_BYTE CPU.F
ENDM

; ********************************************************************
; *          ROL. Opcodes 2A, 26, 36, 2E, 3E                         *
; ********************************************************************
CPU_ROL MACRO Operand:REQ
	; Set/clear the carry flag on the real cpu.
	mov dl, CPU.F
	shr dl, 1
	
	; Perform the operation.
	rcl BYTE PTR [Operand], 1
	MODIFYFLAGS , , , C
	
	mov al, [Operand]
	test al, al
	
	; Test and set/clear the flags.
	MODIFYFLAGS S, , Z, 
ENDM

; ********************************************************************
; *          ROR. Opcodes 6A, 66, 76, 6E, 7E                         *
; ********************************************************************
CPU_ROR MACRO Operand:REQ
	; Set/clear the carry flag on the real cpu.
	mov dl, CPU.F
	shr dl, 1
	
	; Perform the operation.
	rcr BYTE PTR [Operand], 1
	MODIFYFLAGS , , , C
	
	mov al, [Operand]
	test al, al
	
	; Test and set/clear the flags.
	MODIFYFLAGS S, , Z, 
ENDM

; ********************************************************************
; *          RTI. Opcode 4D                                          *
; ********************************************************************
CPU_RTI MACRO
	; Pop the flags register from the stack.
	POP_BYTE CPU.F
	
	; Pop the PC from the stack low byte first.
	POP_BYTE bl
	POP_BYTE bh
	
	; Set the PC to the new address.
	mov CPU.P, bx
ENDM

; ********************************************************************
; *          RTS. Opcode 60                                          *
; ********************************************************************
CPU_RTS MACRO
	; Pop the PC from the stack low byte first.
	POP_BYTE bl
	POP_BYTE bh
	
	; Set the PC to the new address.
	mov CPU.P, bx
ENDM

; ********************************************************************
; *          SBC. Opcodes E9, E5, F5, ED, FD, F9, E1, F1             *
; ********************************************************************
CPU_SBC MACRO Operand:REQ
	; Move the NES's carry flag into the real cpu's carry flag.
	mov dl, CPU.F
	shr dl, 1
	cmc
	
	; Now subtract the operand from the A register with the carry flag.
	sbb CPU.A, Operand
	cmc
	MODIFYFLAGS S, V, Z, C
ENDM

; ********************************************************************
; *          SEC. Opcode 38                                          *
; ********************************************************************
CPU_SEC MACRO
	; Set the carry flag.
	or CPU.F, 00000001b
ENDM

; ********************************************************************
; *          SED. Opcode F8                                          *
; ********************************************************************
CPU_SED MACRO
	; Set the decimal flag.
	or CPU.F, 00001000b
ENDM

; ********************************************************************
; *          SEI. Opcode 78                                          *
; ********************************************************************
CPU_SEI MACRO
	; Set the interrupt flag.
	or CPU.F, 00000100b
ENDM

; ********************************************************************
; *             STA. Opcodes 85, 95, 8D, 9D, 99, 81, 91              *
; *             STX. Opcodes 86, 96, 8E                              *
; *             STY. Opcodes 84, 94, 8C                              *
; ********************************************************************
CPU_ST? MACRO wAddress:REQ, byData:REQ
	WRITE_MEMORY wAddress, byData
ENDM

; ********************************************************************
; *          TAX. Opcode AA                                          *
; ********************************************************************
CPU_TAX MACRO
	; Transfer the A register to the X register.
	mov al, CPU.A
	mov CPU.X, al
	test al, al
	
	; Set/clear the sign and zero flags.
	MODIFYFLAGS S, , Z,
ENDM

; ********************************************************************
; *          TAY. Opcode A8                                          *
; ********************************************************************
CPU_TAY MACRO
	; Transfer the A register to the Y register.
	mov al, CPU.A
	mov CPU.Y, al
	test al, al
	
	; Set/clear the sign and zero flags.
	MODIFYFLAGS S, , Z,
ENDM

; ********************************************************************
; *          TSX. Opcode BA                                          *
; ********************************************************************
CPU_TSX MACRO
	; Transfer the SP register to the X register.
	mov al, CPU.S
	mov CPU.X, al
	test al, al
	
	; Set/clear the sign and zero flags.
	MODIFYFLAGS S, , Z,
ENDM

; ********************************************************************
; *          TXA. Opcode 8A                                          *
; ********************************************************************
CPU_TXA MACRO
	; Transfer the X register to the A register.
	mov al, CPU.X
	mov CPU.A, al
	test al, al
	
	; Set/clear the sign and zero flags.
	MODIFYFLAGS S, , Z,
ENDM

; ********************************************************************
; *          TXS. Opcode 9A                                          *
; ********************************************************************
CPU_TXS MACRO
	; Transfer the X register to the SP register.
	mov al, CPU.X
	mov CPU.S, al
ENDM

; ********************************************************************
; *          TYA. Opcode 98                                          *
; ********************************************************************
CPU_TYA MACRO
	; Transfer the Y register to the A register.
	mov al, CPU.Y
	mov CPU.A, al
	test al, al
	
	; Set/clear the sign and zero flags.
	MODIFYFLAGS S, , Z,
ENDM





;******************************************************************************
;* Name: _RunCPU
;* Desc: TBA
;******************************************************************************
RunCPU PROC C dwFlags : DWORD
	
	LOCAL dwOpcode : DWORD ; Temporary storage for our opcode.

    ; Save all the registers before entering.
	pushad

RunCPU_RunAgain:

    ; Get the opcode byte.
	GET_MEMORY_BYTE CPU.P

    ; Save the opcode for later use and then jump to the
	; correct opcode using the jump table defined above.
	and eax, 000000FFh
	mov dwOpcode, eax
    jmp [adwOpcodeJumpTable+eax*4]


;********************************** ADC ***************************************
; Add memory to the accumulator with the carry bit as well.

_69::
	GET_ADDR_IMMEDIATE
	CPU_ADC al
	jmp _??

_65::
	GET_ADDR_ZEROPAGE
	GET_MEMORY_BYTE ax
	CPU_ADC al
	jmp _??

_75::
	GET_ADDR_ZEROPAGE_X
	GET_MEMORY_BYTE ax
	CPU_ADC al
	jmp _??

_6D::
	GET_ADDR_ABSOLUTE
	GET_MEMORY_BYTE bx
	CPU_ADC al
    jmp _??

_79::
	GET_ADDR_ABSOLUTE_Y
	GET_MEMORY_BYTE bx
	CPU_ADC al
    jmp _??

_7D::
	GET_ADDR_ABSOLUTE_X
	GET_MEMORY_BYTE bx
	CPU_ADC al
    jmp _??

_61::
	GET_ADDR_PREINDEXED
	GET_MEMORY_BYTE bx
	CPU_ADC al
    jmp _??

_71::
	GET_ADDR_POSTINDEXED
	GET_MEMORY_BYTE bx
	CPU_ADC al
    jmp _??

;********************************** AND ***************************************
; For all the addressing modes except for immediate, get the address 
; of the byte to AND A with, then use that address to get the byte
; and perform the AND operation with it. For the immediate mode,
; just AND the byte with the A register.

_29::
	GET_ADDR_IMMEDIATE
	CPU_AND al
	jmp _??

_25::
	GET_ADDR_ZEROPAGE
	GET_MEMORY_BYTE ax
	CPU_AND al
	jmp _??

_35::
	GET_ADDR_ZEROPAGE_X
	GET_MEMORY_BYTE ax
	CPU_AND al
	jmp _??

_2D::
	GET_ADDR_ABSOLUTE
	GET_MEMORY_BYTE bx
    CPU_AND al
    jmp _??

_39::
	GET_ADDR_ABSOLUTE_Y
	GET_MEMORY_BYTE bx
    CPU_AND al
    jmp _??

_3D::
	GET_ADDR_ABSOLUTE_X
	GET_MEMORY_BYTE bx
    CPU_AND al
    jmp _??

_21::
	GET_ADDR_PREINDEXED
	GET_MEMORY_BYTE bx
    CPU_AND al
    jmp _??

_31::
	GET_ADDR_POSTINDEXED
	GET_MEMORY_BYTE bx
    CPU_AND al
    jmp _??

;********************************** ASL ***************************************
; Perform a logical shift left with least significant bit being filled
; with a zero and the most significant bit going into the carry.
_0A::
	mov ebx, OFFSET (CPU.A)
	CPU_ASL ebx
	jmp _??
	
_06::
	GET_ADDR_ZEROPAGE
	GET_MEMORY_POINTER ax
	mov ebx, eax
	CPU_ASL ebx
	jmp _??
	
_16::
	GET_ADDR_ZEROPAGE_X
	GET_MEMORY_POINTER ax
	mov ebx, eax
	CPU_ASL ebx
	jmp _??

_0E::
	GET_ADDR_ABSOLUTE
	GET_MEMORY_POINTER bx
	mov ebx, eax
	CPU_ASL ebx
	jmp _??

_1E::
	GET_ADDR_ABSOLUTE_X
	GET_MEMORY_POINTER bx
	mov ebx, eax
	CPU_ASL ebx
	jmp _??

;********************************** BCC ***************************************
; Branches to a memory location if the carry flag is clear.
_90::
	GET_ADDR_RELATIVE
	CPU_BCC ax
	jmp _??

;********************************** BCS ***************************************
; Branches to a memory location if the carry flag is set.
_B0::
	GET_ADDR_RELATIVE
	CPU_BCS ax
	jmp _??

;********************************** BEQ ***************************************
; Branches to a memory location if the zero flag is set.
_F0::
	GET_ADDR_RELATIVE
	CPU_BEQ ax
	jmp _??

;********************************** BIT ***************************************
; Ands the operand with the A register and transfers bit 7 and 6 to 
; the flags register.

_24::
	GET_ADDR_ZEROPAGE
	GET_MEMORY_BYTE ax
	CPU_BIT al
	jmp _??
	
_2C::
	GET_ADDR_ABSOLUTE
	GET_MEMORY_BYTE bx
	CPU_BIT al
	jmp _??

;********************************** BMI ***************************************
; Branches to a memory location if the sign flag is set.
_30::
	GET_ADDR_RELATIVE
	CPU_BMI ax
	jmp _??

;********************************** BNE ***************************************
; Branches to a memory location if the zero flag is clear.
_D0::
	GET_ADDR_RELATIVE
	CPU_BNE ax
	jmp _??

;********************************** BPL ***************************************
; Branches to a memory location if the sign flag is clear.
_10::
	GET_ADDR_RELATIVE
	CPU_BPL ax
	jmp _??

;********************************** BRK ***************************************
; Push the PC and flags onto the stack and then jump to the word 
; address specified in the PRG-ROM at $FFFE
_00::
	CPU_BRK
	jmp _??
	
;********************************** BVC ***************************************
; Branches to a memory location if the overflow flag is clear.
_50::
	GET_ADDR_RELATIVE
	CPU_BVC ax
	jmp _??

;********************************** BVS ***************************************
; Branches to a memory location if the overflow flag is set.
_70::
	GET_ADDR_RELATIVE
	CPU_BVS ax
	jmp _??

;********************************** CLC ***************************************
; Clears the carry flag.
_18::
	CPU_CLC
	jmp _??

;********************************** CLD ***************************************
; Clears the decimal flag.
_D8::
	CPU_CLD
	jmp _??

;********************************** CLI ***************************************
; Clears the interrupt flag.
_58::
	CPU_CLI
	jmp _??

;********************************** CLV ***************************************
; Clears the overflow flag.
_B8::
	CPU_CLV
	jmp _??

;********************************** CMP ***************************************
; Compares memory with the A register by using subtraction.

_C9::
	GET_ADDR_IMMEDIATE
	CPU_CP? CPU.A, al
	jmp _??

_C5::
	GET_ADDR_ZEROPAGE
	GET_MEMORY_BYTE ax
	CPU_CP? CPU.A, al
	jmp _??

_D5::
	GET_ADDR_ZEROPAGE_X
	GET_MEMORY_BYTE ax
	CPU_CP? CPU.A, al
	jmp _??

_CD::
	GET_ADDR_ABSOLUTE
	GET_MEMORY_BYTE bx
	CPU_CP? CPU.A, al
    jmp _??

_D9::
	GET_ADDR_ABSOLUTE_Y
	GET_MEMORY_BYTE bx
	CPU_CP? CPU.A, al
    jmp _??

_DD::
	GET_ADDR_ABSOLUTE_X
	GET_MEMORY_BYTE bx
	CPU_CP? CPU.A, al
    jmp _??

_C1::
	GET_ADDR_PREINDEXED
	GET_MEMORY_BYTE bx
	CPU_CP? CPU.A, al
    jmp _??

_D1::
	GET_ADDR_POSTINDEXED
	GET_MEMORY_BYTE bx
	CPU_CP? CPU.A, al
    jmp _??

;********************************** CPX ***************************************
; Compares memory with the X register by using subtraction.

_E0::
	GET_ADDR_IMMEDIATE
	CPU_CP? CPU.X, al
	jmp _??

_E4:: 
	GET_ADDR_ZEROPAGE
	GET_MEMORY_BYTE ax
    CPU_CP? CPU.X, al
	jmp _??

_EC::
	GET_ADDR_ABSOLUTE
	GET_MEMORY_BYTE bx
    CPU_CP? CPU.X, al
    jmp _??

;********************************** CPY ***************************************
; Compares memory with the Y register by using subtraction.

_C0::
	GET_ADDR_IMMEDIATE
	CPU_CP? CPU.Y, al
	jmp _??

_C4:: 
	GET_ADDR_ZEROPAGE
	GET_MEMORY_BYTE ax
    CPU_CP? CPU.Y, al
	jmp _??

_CC::
	GET_ADDR_ABSOLUTE
	GET_MEMORY_BYTE bx
    CPU_CP? CPU.Y, al
    jmp _??

;********************************** DEC ***************************************
; Decrement the value in memory by 1.

_C6::
	GET_ADDR_ZEROPAGE
	GET_MEMORY_POINTER ax
	mov ebx, eax
	CPU_DEC ebx
	jmp _??
_D6::
	GET_ADDR_ZEROPAGE_X
	GET_MEMORY_POINTER ax
	mov ebx, eax
	CPU_DEC ebx
	jmp _??

_CE::
	GET_ADDR_ABSOLUTE
	GET_MEMORY_POINTER bx
	mov ebx, eax
	CPU_DEC ebx
	jmp _??

_DE::
	GET_ADDR_ABSOLUTE_X
	GET_MEMORY_POINTER bx
	mov ebx, eax
	CPU_DEC ebx
	jmp _??

;********************************** DEX ***************************************
; Decrement the value in the X register by one.
_CA::
	CPU_DEX
	jmp _??
	
;********************************** DEY ***************************************
; Decrement the value in the Y register by one.
_88::
	CPU_DEY
	jmp _??
	
;********************************** EOR ***************************************
; Perform a logical exclusive or operation on the memory and the A register
; storing the result in the A register.
_49::
	GET_ADDR_IMMEDIATE
	CPU_EOR al
	jmp _??

_45::
	GET_ADDR_ZEROPAGE
	GET_MEMORY_BYTE ax
	CPU_EOR al
	jmp _??

_55::
	GET_ADDR_ZEROPAGE_X
	GET_MEMORY_BYTE ax
	CPU_EOR al
	jmp _??

_4D::
	GET_ADDR_ABSOLUTE
	GET_MEMORY_BYTE bx
	CPU_EOR al
    jmp _??

_59::
	GET_ADDR_ABSOLUTE_Y
	GET_MEMORY_BYTE bx
	CPU_EOR al
    jmp _??

_5D::
	GET_ADDR_ABSOLUTE_X
	GET_MEMORY_BYTE bx
	CPU_EOR al
    jmp _??

_41::
	GET_ADDR_PREINDEXED
	GET_MEMORY_BYTE bx
	CPU_EOR al
    jmp _??

_51::
	GET_ADDR_POSTINDEXED
	GET_MEMORY_BYTE bx
	CPU_EOR al
    jmp _??

;********************************** INC ***************************************
; Increment the value in memory by 1.

_E6::
	GET_ADDR_ZEROPAGE
	GET_MEMORY_POINTER ax
	mov ebx, eax
	CPU_INC ebx
	jmp _??
_F6::
	GET_ADDR_ZEROPAGE_X
	GET_MEMORY_POINTER ax
	mov ebx, eax
	CPU_INC ebx
	jmp _??

_EE::
	GET_ADDR_ABSOLUTE
	GET_MEMORY_POINTER bx
	mov ebx, eax
	CPU_INC ebx
	jmp _??

_FE::
	GET_ADDR_ABSOLUTE_X
	GET_MEMORY_POINTER bx
	mov ebx, eax
	CPU_INC ebx
	jmp _??

;********************************** INX ***************************************
; Increment the value in the X register by one.
_E8::
	CPU_INX
	jmp _??
	
;********************************** INY ***************************************
; Increment the value in the Y register by one.
_C8::
	CPU_INY
	jmp _??

;********************************** JMP ***************************************
; Get the value to set the PC to and store that value in the PC register.

_4C::
	GET_ADDR_ABSOLUTE
	CPU_JMP bx
	jmp _??
	
_6C::
	GET_ADDR_INDIRECT
	CPU_JMP bx
	jmp _??

;********************************** JSR ***************************************
; Get the value to set the PC to and store that value in the PC register.
; Save the return address on the stack.
_20::
	GET_ADDR_ABSOLUTE
	CPU_JSR bx
	jmp _??

;********************************** LDA ***************************************
; For all the addressing modes except for immediate, get the address 
; to read memory from, then read memory and load it into the A register.
; For the immediate mode just load the byte into the A register.

_A9::
	GET_ADDR_IMMEDIATE
	CPU_LD? CPU.A, al
	jmp _??

_A5::
	GET_ADDR_ZEROPAGE
	READ_MEMORY ax
    CPU_LD? CPU.A, al
	jmp _??

_B5::
	GET_ADDR_ZEROPAGE_X
	READ_MEMORY ax
    CPU_LD? CPU.A, al
	jmp _??

_AD::
	GET_ADDR_ABSOLUTE
	READ_MEMORY bx
    CPU_LD? CPU.A, al
    jmp _??

_B9::
	GET_ADDR_ABSOLUTE_Y
	READ_MEMORY bx
    CPU_LD? CPU.A, al
    jmp _??

_BD::
	GET_ADDR_ABSOLUTE_X
	READ_MEMORY bx
    CPU_LD? CPU.A, al
    jmp _??

_A1::
	GET_ADDR_PREINDEXED
	READ_MEMORY bx
    CPU_LD? CPU.A, al
    jmp _??

_B1::
	GET_ADDR_POSTINDEXED
	READ_MEMORY bx
    CPU_LD? CPU.A, al
    jmp _??


;********************************** LDX ***************************************
; For all the addressing modes except for immediate, get the address 
; to read memory from, then read memory and load it into the X register.
; For the immediate mode just load the byte into the X register.

_A2::
	GET_ADDR_IMMEDIATE
	CPU_LD? CPU.X, al
	jmp _??

_A6:: 
	GET_ADDR_ZEROPAGE
	READ_MEMORY ax
    CPU_LD? CPU.X, al
	jmp _??

_B6::
	GET_ADDR_ZEROPAGE_Y
	READ_MEMORY ax
    CPU_LD? CPU.X, al
	jmp _??

_AE::
	GET_ADDR_ABSOLUTE
	READ_MEMORY bx
    CPU_LD? CPU.X, al
    jmp _??

_BE::
	GET_ADDR_ABSOLUTE_Y
	READ_MEMORY bx
    CPU_LD? CPU.X, al
    jmp _??

;********************************** LDY ***************************************
; For all the addressing modes except for immediate, get the address 
; to read memory from, then read memory and load it into the Y register.
; For the immediate mode just load the byte into the Y register.

_A0::
	GET_ADDR_IMMEDIATE
	CPU_LD? CPU.Y, al
	jmp _??

_A4:: 
	GET_ADDR_ZEROPAGE
	READ_MEMORY ax
    CPU_LD? CPU.Y, al
	jmp _??

_B4::
	GET_ADDR_ZEROPAGE_X
	READ_MEMORY ax
    CPU_LD? CPU.Y, al
	jmp _??

_AC::
	GET_ADDR_ABSOLUTE
	READ_MEMORY bx
    CPU_LD? CPU.Y, al
    jmp _??

_BC::
	GET_ADDR_ABSOLUTE_X
	READ_MEMORY bx
    CPU_LD? CPU.Y, al
    jmp _??

;********************************** LSR ***************************************
; Perform a logical shift right with most significant bit being filled
; with a zero and the least significant bit going into the carry.
_4A::
	mov ebx, OFFSET (CPU.A)
	CPU_LSR ebx
	jmp _??
	
_46::
	GET_ADDR_ZEROPAGE
	GET_MEMORY_POINTER ax
	mov ebx, eax
	CPU_LSR ebx
	jmp _??
	
_56::
	GET_ADDR_ZEROPAGE_X
	GET_MEMORY_POINTER ax
	mov ebx, eax
	CPU_LSR ebx
	jmp _??

_4E::
	GET_ADDR_ABSOLUTE
	GET_MEMORY_POINTER bx
	mov ebx, eax
	CPU_LSR ebx
	jmp _??

_5E::
	GET_ADDR_ABSOLUTE_X
	GET_MEMORY_POINTER bx
	mov ebx, eax
	CPU_LSR ebx
	jmp _??

;********************************** ORA ***************************************
; Perform a logical or operation on the memory and the A register
; storing the result in the A register.
_09::
	GET_ADDR_IMMEDIATE
	CPU_ORA al
	jmp _??

_05::
	GET_ADDR_ZEROPAGE
	GET_MEMORY_BYTE ax
	CPU_ORA al
	jmp _??

_15::
	GET_ADDR_ZEROPAGE_X
	GET_MEMORY_BYTE ax
	CPU_ORA al
	jmp _??

_0D::
	GET_ADDR_ABSOLUTE
	GET_MEMORY_BYTE bx
	CPU_ORA al
    jmp _??

_19::
	GET_ADDR_ABSOLUTE_Y
	GET_MEMORY_BYTE bx
	CPU_ORA al
    jmp _??

_1D::
	GET_ADDR_ABSOLUTE_X
	GET_MEMORY_BYTE bx
	CPU_ORA al
    jmp _??

_01::
	GET_ADDR_PREINDEXED
	GET_MEMORY_BYTE bx
	CPU_ORA al
    jmp _??

_11::
	GET_ADDR_POSTINDEXED
	GET_MEMORY_BYTE bx
	CPU_ORA al
    jmp _??

;********************************** PHA ***************************************
; Pushes the A register onto the stack.
_48::
	CPU_PHA
	jmp _??
	
;********************************** PHP ***************************************
; Pushes the flags register onto the stack.
_08::
	CPU_PHP
	jmp _??

;********************************** PLA ***************************************
; Pops the A register from the stack.
_68::
	CPU_PLA
	jmp _??

;********************************** PLP ***************************************
; Pops the flags register from the stack.
_28::
	CPU_PLP
	jmp _??

;********************************** ROL ***************************************
; Perform a rotate left with most significant bit going into the carry flag
; and the least significant bit coming from the carry flag.
_2A::
	mov ebx, OFFSET (CPU.A)
	CPU_ROL ebx
	jmp _??
	
_26::
	GET_ADDR_ZEROPAGE
	GET_MEMORY_POINTER ax
	mov ebx, eax
	CPU_ROL ebx
	jmp _??
	
_36::
	GET_ADDR_ZEROPAGE_X
	GET_MEMORY_POINTER ax
	mov ebx, eax
	CPU_ROL ebx
	jmp _??

_2E::
	GET_ADDR_ABSOLUTE
	GET_MEMORY_POINTER bx
	mov ebx, eax
	CPU_ROL ebx
	jmp _??

_3E::
	GET_ADDR_ABSOLUTE_X
	GET_MEMORY_POINTER bx
	mov ebx, eax
	CPU_ROL ebx
	jmp _??

;********************************** ROR ***************************************
; Perform a rotate right with least significant bit going into the carry flag
; and the most significant bit coming from the carry flag.
_6A::
	mov ebx, OFFSET (CPU.A)
	CPU_ROR ebx
	jmp _??
	
_66::
	GET_ADDR_ZEROPAGE
	GET_MEMORY_POINTER ax
	mov ebx, eax
	CPU_ROR ebx
	jmp _??
	
_76::
	GET_ADDR_ZEROPAGE_X
	GET_MEMORY_POINTER ax
	mov ebx, eax
	CPU_ROR ebx
	jmp _??

_6E::
	GET_ADDR_ABSOLUTE
	GET_MEMORY_POINTER bx
	mov ebx, eax
	CPU_ROR ebx
	jmp _??

_7E::
	GET_ADDR_ABSOLUTE_X
	GET_MEMORY_POINTER bx
	mov ebx, eax
	CPU_ROR ebx
	jmp _??

;********************************** RTI ***************************************
; Return from an interrupt.
_40::
	CPU_RTI
	jmp _??
	
;********************************** RTS ***************************************
; Return from an subroutine.
_60::
	CPU_RTS
	jmp _??

;********************************** SBC ***************************************
; Subtract memory from the accumulator and with the carry bit as well.

_E9::
	GET_ADDR_IMMEDIATE
	CPU_SBC al
	jmp _??

_E5::
	GET_ADDR_ZEROPAGE
	GET_MEMORY_BYTE ax
	CPU_SBC al
	jmp _??

_F5::
	GET_ADDR_ZEROPAGE_X
	GET_MEMORY_BYTE ax
	CPU_SBC al
	jmp _??

_ED::
	GET_ADDR_ABSOLUTE
	GET_MEMORY_BYTE bx
	CPU_SBC al
    jmp _??

_F9::
	GET_ADDR_ABSOLUTE_Y
	GET_MEMORY_BYTE bx
	CPU_SBC al
    jmp _??

_FD::
	GET_ADDR_ABSOLUTE_X
	GET_MEMORY_BYTE bx
	CPU_SBC al
    jmp _??

_E1::
	GET_ADDR_PREINDEXED
	GET_MEMORY_BYTE bx
	CPU_SBC al
    jmp _??

_F1::
	GET_ADDR_POSTINDEXED
	GET_MEMORY_BYTE bx
	CPU_SBC al
    jmp _??
    
;********************************** SEC ***************************************
; Set the carry flag.
_38::
	CPU_SEC
	jmp _??

;********************************** SED ***************************************
; Set the decimal flag.
_F8::
	CPU_SED
	jmp _??

;********************************** SEI ***************************************
; Set the interrupt flag.
_78::
	CPU_SEI
	jmp _??

;********************************** STA ***************************************
; For all the addressing modes, get the address to write the A register
; to, then store the A register into that memory address.

_85::
	GET_ADDR_ZEROPAGE
    CPU_ST? ax, CPU.A
	jmp _??

_95::
	GET_ADDR_ZEROPAGE_X
    CPU_ST? ax, CPU.A
	jmp _??

_8D::
	GET_ADDR_ABSOLUTE
    CPU_ST? bx, CPU.A
    jmp _??

_99::
	GET_ADDR_ABSOLUTE_Y
    CPU_ST? bx, CPU.A
    jmp _??

_9D::
	GET_ADDR_ABSOLUTE_X
    CPU_ST? bx, CPU.A
    jmp _??

_81::
	GET_ADDR_PREINDEXED
    CPU_ST? bx, CPU.A
    jmp _??

_91::
	GET_ADDR_POSTINDEXED
    CPU_ST? bx, CPU.A
    jmp _??


;********************************** STX ***************************************

; For all the addressing modes, get the address to write the X register
; to, then store the X register into that memory address.

_86::
	GET_ADDR_ZEROPAGE
    CPU_ST? ax, CPU.X
	jmp _??

_96::
	GET_ADDR_ZEROPAGE_Y
    CPU_ST? ax, CPU.X
	jmp _??

_8E::
	GET_ADDR_ABSOLUTE
    CPU_ST? bx, CPU.X
    jmp _??


;********************************** STY ***************************************

; For all the addressing modes, get the address to write the Y register
; to, then store the Y register into that memory address.

_84::
	GET_ADDR_ZEROPAGE
    CPU_ST? ax, CPU.Y
	jmp _??

_94::
	GET_ADDR_ZEROPAGE_X
    CPU_ST? ax, CPU.Y
	jmp _??

_8C::
	GET_ADDR_ABSOLUTE
    CPU_ST? bx, CPU.Y
    jmp _??

;********************************** TAX ***************************************
; Transfers the A register to the X register.
_AA::
	CPU_TAX
	jmp _??

;********************************** TAY ***************************************
; Transfers the A register to the YX register.
_A8::
	CPU_TAY
	jmp _??

;********************************** TSX ***************************************
; Transfers the S register to the X register.
_BA::
	CPU_TSX
	jmp _??

;********************************** TXA ***************************************
; Transfers the X register to the A register.
_8A::
	CPU_TXA
	jmp _??

;********************************** TXS ***************************************
; Transfers the X register to the S register.
_9A::
	CPU_TXS
	jmp _??

;********************************** TYA ***************************************
; Transfers the Y register to the A register.
_98::
	CPU_TYA
	jmp _??

_??::
    
	; When the instruction is finished me must subtract the
	; number of cycles the instruction takes and add the
	; number of bytes for the instruction to the PC.
	xor ax, ax
	mov ebx, dwOpcode
    mov al, [abyOpcodeCycles+ebx]
    sub CPU.byCycles, al
    mov al, [abyOpcodeBytes+ebx]
    add CPU.P, ax

	; In step mode, we execute one instruction and then exit
	; the function to let an external module control what
	; happens next. In run mode, we run until the cpu cycles
	; are less that zero and then let an external module 
	; control what happens next.
	.IF dwFlags == RUNCPU_STEP
		jmp RunCPU_End
	.ELSEIF dwFlags == RUNCPU_RUN
		cmp CPU.byCycles, 0
		jg RunCPU_RunAgain
	.ENDIF
	
    ; We're done now so pop all the flags and return.
RunCPU_End:
	popad
    ret

RunCPU ENDP

END