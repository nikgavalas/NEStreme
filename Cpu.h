//------------------------------------------------------------------------------
// Name: Cpu.h
// Desc: Header file for all the Cpu stuff. Note to self, write better desc!!!
//------------------------------------------------------------------------------

#ifndef __CPU_H__
#define __CPU_H__


/*
  THE REGISTERS INSIDE THE 6502 MICROPROCESSOR

    Almost all calculations are done in the microprocessor. Registers are
  special pieces of memory in the processor which are used to carry out, and
  store information about calculations. The 6502 has the following registers:


  THE ACCUMULATOR

    This is THE most important register in the microprocessor. Various ma-
  chine language instructions allow you to copy the contents of a memory
  location into the accumulator, copy the contents of the accumulator into
  a memory location, modify the contents of the accumulator or some other
  register directly, without affecting any memory. And the accumulator is
  the only register that has instructions for performing math.


  THE X INDEX REGISTER

    This is a very important register. There are instructions for nearly
  all of the transformations you can make to the accumulator. But there are
  other instructions for things that only the X register can do. Various
  machine language instructions allow you to copy the contents of a memory
  location into the X register, copy the contents of the X register into a
  memory location, and modify the contents of the X, or some other register
  directly.


  THE Y INDEX REGISTER

    This is a very important register. There are instructions for nearly
  all of the transformations you can make to the accumulator, and the X
  register. But there are other instructions for things that only the Y
  register can do. Various machine language instructions allow you to copy
  the contents of a memory location into the Y register, copy the contents
  of the Y register into a memory location, and modify the contents of the
  Y, or some other register directly.


  THE STATUS REGISTER

    This register consists of eight "flags" (a flag = something that indi-
  cates whether something has, or has not occurred). Bits of this register
  are altered depending on the result of arithmetic and logical operations.
  These bits are described below:

     Bit No.       7   6   5   4   3   2   1   0
                   S   V       B   D   I   Z   C

   Bit0 - C - Carry flag: this holds the carry out of the most significant
   bit in any arithmetic operation. In subtraction operations however, this
   flag is cleared - set to 0 - if a borrow is required, set to 1 - if no
   borrow is required. The carry flag is also used in shift and rotate
   logical operations.

   Bit1 - Z - Zero flag: this is set to 1 when any arithmetic or logical
   operation produces a zero result, and is set to 0 if the result is
   non-zero.

   Bit 2 - I: this is an interrupt enable/disable flag. If it is set,
   interrupts are disabled. If it is cleared, interrupts are enabled.

   Bit 3 - D: this is the decimal mode status flag. When set, and an Add with
   Carry or Subtract with Carry instruction is executed, the source values are
   treated as valid BCD (Binary Coded Decimal, eg. 0x00-0x99 = 0-99) numbers.
   The result generated is also a BCD number.

   Bit 4 - B: this is set when a software interrupt (BRK instruction) is
   executed.

   Bit 5: not used. Supposed to be logical 1 at all times.

   Bit 6 - V - Overflow flag: when an arithmetic operation produces a result
   too large to be represented in a byte, V is set.

   Bit 7 - S - Sign flag: this is set if the result of an operation is
   negative, cleared if positive.

   The most commonly used flags are C, Z, V, S.

  

  THE PROGRAM COUNTER

    This contains the address of the current machine language instruction
  being executed. Since the operating system is always "RUN"ning in the
  Commodore VIC-20 (or, for that matter, any computer), the program counter
  is always changing. It could only be stopped by halting the microprocessor
  in some way.


  THE STACK POINTER

    This register contains the location of the first empty place on the
  stack. The stack is used for temporary storage by machine language pro-
  grams, and by the computer.
*/



// The CPU structure.
typedef struct tagNES6502
{
	BYTE  A;              // The Accumulator register on the 6502.
	BYTE  X;              // The X Index register on the 6502.
	BYTE  Y;              // The Y Index register on the 6502.
	BYTE  S;              // The stack pointer on the 6502.
	BYTE  F;              // The flags register on the 6502.
	WORD  P;              // The program counter on the 6502.
	BYTE  Memory[0x8000]; // All the memory on the NES except the PRG-ROM.
	BYTE* pbyPRGROMBank1; // Points to the first PRG-ROM bank on the NES cartridge.
	BYTE* pbyPRGROMBank2; // Points to the second PRG-ROM bank on the NES cartridge.
	BYTE  byCycles;       // Number of cycles left until the end of the scanline.
} NES6502;


// Number of cycles each scanline takes.
#define NUM_CYCLES_PER_SCANLINE 113


// Valid control flags used in RunCPU()
#define RUNCPU_STEP 0x00000001
#define RUNCPU_RUN  0x00000002


// CPU frequency
#define CPU_FREQUENCY 1789772

// External ASM function to run the CPU
extern "C" void RunCPU(DWORD dwFlags);

#endif