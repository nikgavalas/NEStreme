//------------------------------------------------------------------------------
// Name: Io.h
// Desc: Header file for Io.cpp
//------------------------------------------------------------------------------

#ifndef __IO_H__
#define __IO_H__

#include "Cpu.h"
#include "Ppu.h"
#include "Joystick.h"
#include "NESData.h"

// Function prototypes
BYTE  __stdcall GetMemoryByte(WORD wAddress);
BYTE* __stdcall GetMemoryPointer(WORD wAddress);
BYTE  __stdcall ReadMemory(WORD wAddress);
VOID  __stdcall WriteMemory(WORD wAddress, BYTE byData);
BYTE  __stdcall PPUReadMemory(WORD wAddress);
VOID  __stdcall PPUWriteMemory(WORD wAddress, BYTE byData);

// Externals needed.
typedef BOOL (*MAPPERLOAD)(NESDATA*);
typedef BYTE (*MAPPERREAD)(WORD);
typedef BOOL (*MAPPERWRITE)(WORD, BYTE);

extern MAPPERLOAD  MapperOnLoad;
extern MAPPERREAD  MapperOnRead;
extern MAPPERWRITE MapperOnWrite;

extern NES6502  CPU;
extern NESPPU   PPU;
extern Joystick Joy1;
extern BYTE     byVRAMAddrRegToggle;
extern WORD     wVRAMAddress;
extern BYTE     abySPRRAM[256];
extern BYTE     byHScroll[300];
extern BYTE     byVScroll[300];
extern WORD     wScanline;
extern BOOL     bBreak;

#endif