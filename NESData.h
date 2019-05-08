//------------------------------------------------------------------------------
// Name: NESData.h
// Desc: This header file holds the structure that is passed to the memory
//       mapper .dll. This structure holds pointers to our variables that
//       the mapper may need to use or modify.
//------------------------------------------------------------------------------

#ifndef __NESDATA_H__
#define __NESDATA_H__

#include "Cpu.h"
#include "Ppu.h"

typedef struct tagNESData
{
	NES6502* pCPU;            // Pointer to the CPU structure.
	NESPPU*  pPPU;            // Pointer to the PPU structure.
	BYTE*    pabyPRGROM;      // Pointer to the array of program rom.
	WORD     wNumPRGROMPages; // Number of program rom pages.
	BYTE*    pabyCHRROM;      // Pointer to the array of pattern rom.
	WORD     wNumCHRROMPages; // Number of program rom pages.
} NESDATA;


#endif
