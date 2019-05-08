//------------------------------------------------------------------------------
// Name: Io.cpp
// Desc: Holds all the functions to read and write to memory.
//------------------------------------------------------------------------------

// Includes
#include <windows.h>
#include "Io.h"
#include "Sound.h"


//------------------------------------------------------------------------------
// Name: GetMemoryByte()
// Desc: Returns a byte from the Cpu's memory.
//------------------------------------------------------------------------------
BYTE __stdcall GetMemoryByte(WORD wAddress)
{
	if (wAddress >= 0x8000 && wAddress < 0xC000)
		return CPU.pbyPRGROMBank1[wAddress-0x8000];
	else if (wAddress >= 0xC000 && wAddress <= 0xFFFF)
		return CPU.pbyPRGROMBank2[wAddress-0xC000];
	else
		return CPU.Memory[wAddress];
} // end GetMemoryByte()


//------------------------------------------------------------------------------
// Name: GetMemoryPointer()
// Desc: Returns a pointer to a byte from the Cpu's memory.
//------------------------------------------------------------------------------
BYTE* __stdcall GetMemoryPointer(WORD wAddress)
{
	if (wAddress >= 0x8000 && wAddress < 0xC000)
		return (&(CPU.pbyPRGROMBank1[wAddress-0x8000]));
	else if (wAddress >= 0xC000 && wAddress <= 0xFFFF)
		return (&(CPU.pbyPRGROMBank2[wAddress-0xC000]));
	else
		return (&(CPU.Memory[wAddress]));
} // end GetMemoryPointer()


//------------------------------------------------------------------------------
// Name: ReadMemory()
// Desc: N/A
//------------------------------------------------------------------------------
BYTE __stdcall ReadMemory(WORD wAddress)
{
	static BYTE byVRAMBuffer; // See the case where wAddress=0x2007.

	if (wAddress >= 0x8000 && wAddress <= 0xFFFF)
		return MapperOnRead(wAddress);
	else
	{
		// Check for all the register reads.
		if (wAddress == 0x2002)
		{
			BYTE byRetVal = CPU.Memory[0x2002];

			// Clear the vblank bit.
			CPU.Memory[0x2002] &= 0x7F;
			// Clear the VRAM toggle.
			byVRAMAddrRegToggle = 0;

			return byRetVal;
		}
		else if (wAddress == 0x2003)
		{
			return CPU.Memory[0x2003];
		}
		else if (wAddress == 0x2004)
		{
			return abySPRRAM[CPU.Memory[0x2003]];
		}
		else if (wAddress == 0x2007)
		{
			/*
			 * Cut and paste from yoshi's document nestech.txt
			 *
			 O. PPU Quirks
			 -------------
				The first read from VRAM is invalid. Due to this aspect, the NES will
				returned pseudo-buffered values from VRAM rather than linear as expec-
				ted. See the below example:

				VRAM $2000 contains $AA $BB $CC $DD.
				VRAM incrementation value is 1.
				The result of execution is printed in the comment field. 

				LDA #$20
				STA $2006
				LDA #$00
				STA $2006        ; VRAM address now set at $2000
				LDA $2007        ; A=??     VRAM Buffer=$AA
				LDA $2007        ; A=$AA    VRAM Buffer=$BB
				LDA $2007        ; A=$BB    VRAM Buffer=$CC
				LDA #$20
				STA $2006
				LDA #$00
				STA $2006        ; VRAM address now set at $2000
				LDA $2007        ; A=$CC    VRAM Buffer=$AA
				LDA $2007        ; A=$AA    VRAM Buffer=$BB

				As shown, the PPU will post-increment it's internal address data after
				the first read is performed. This *ONLY APPLIES* to VRAM $0000-3EFF
				(e.g. Palette data and their respective mirrors do not suffer from
				this phenomenon).
			*/
			if (wVRAMAddress >= 0 && wVRAMAddress < 0x3F00)
			{
				BYTE byReturnVal = byVRAMBuffer;

				// Read the byte into the VRAM buffer.
				byVRAMBuffer = PPUReadMemory(wVRAMAddress);
				
				// Increment the address by 1 or 32 depending on the
				// status of bit 2 of reg $2000.
				(CPU.Memory[0x2000] & 4) ? (wVRAMAddress += 32) : wVRAMAddress++;

				return byReturnVal;
			}
			else
				return PPUReadMemory(wVRAMAddress);
		}
		// All the APU registers.
		else if (wAddress == 0x4015)
		{
			return APU_Read(wAddress);
		}
		else if (wAddress == 0x4016)
		{
			return Joy1.read();
		}

		// If its not one of the registers, then just write the byte to memory.
		else
			return CPU.Memory[wAddress];
	}

	return 0;
} // end ReadMemory()


//------------------------------------------------------------------------------
// Name: WriteMemory()
// Desc: Writes to CPU memory and checks to see if its a register write as
//       well. Then depending on if its a register write, it does the
//       appropriate action.
//------------------------------------------------------------------------------
VOID __stdcall WriteMemory(WORD wAddress, BYTE byData)
{
	// PRG-ROM writes.
	if (wAddress >= 0x8000 && wAddress <= 0xFFFF)
		MapperOnWrite(wAddress, byData);
	else 
	{
		// Check for all the register writes.
		if (wAddress == 0x2000)
		{
			CPU.Memory[0x2000] = byData;
		}
		else if (wAddress == 0x2001)
		{
			CPU.Memory[0x2001] = byData;
		}
		else if (wAddress == 0x2003)
		{
			// Save the address to access sprite ram with.
			CPU.Memory[0x2003] = byData;
		}
		else if (wAddress == 0x2004)
		{
			// Write the data into the sprite ram at the
			// addres specified in register $2003.
			abySPRRAM[CPU.Memory[0x2003]] = byData;
		}
		else if (wAddress == 0x2005)
		{
			// The second write is the vertical scroll, the
			// first write is the horizontal scroll.
			if (byVRAMAddrRegToggle)
				byVScroll[0] = byData;
			else
				byHScroll[0] = byData;

			// Toggle to indicate that we are on the second write.
			byVRAMAddrRegToggle ^= 1;
		}
		else if (wAddress == 0x2006)
		{
			if (byVRAMAddrRegToggle)
				// Write the lower byte into the VRAM address register.
				*((BYTE*)(&wVRAMAddress)) = byData;		
			else
				// Write the upper byte into the VRAM address register.
				*(((BYTE*)(&wVRAMAddress))+1) = byData;

			// Toggle to indicate that we are on the second write.
			byVRAMAddrRegToggle ^= 1;
		}
		else if (wAddress == 0x2007)
		{
			// Write the byte to video memory.
			PPUWriteMemory(wVRAMAddress, byData);

			// Increment the address by 1 or 32 depending on the
			// status of bit 2 of reg $2000.
			(CPU.Memory[0x2000] & 4) ? (wVRAMAddress += 32) : wVRAMAddress++;
		}
		// All the APU registers.
		else if (wAddress == 0x4003 || wAddress == 0x4015)
		{
			CPU.Memory[wAddress] = byData;
			APU_Write(wAddress, byData);
		}
		else if (wAddress == 0x4014)
		{
			// Transfers 256 bytes of memory into SPR-RAM. The address
			// read from is $100*N, where N is the value written.
			memcpy(abySPRRAM, &CPU.Memory[0x100*byData], 256);
			CPU.Memory[0x4014] = byData;
		}
		else if (wAddress == 0x4016)
		{
			Joy1.write(byData);
			CPU.Memory[0x4016] = byData;
		}
		// If its not one of the registers, then just write the byte to memory.
		else
			CPU.Memory[wAddress] = byData;
	}

} // end WriteMemory()


//------------------------------------------------------------------------------
// Name: PPUReadMemory()
// Desc: N/A
//------------------------------------------------------------------------------
BYTE __stdcall PPUReadMemory(WORD wAddress)
{
	// Pattern table read.
	if (wAddress < 0x1000)
		return *((PPU.apbyPatternTables[0])+(wAddress));
	else if (wAddress >= 0x1000 && wAddress < 0x2000)
		return *((PPU.apbyPatternTables[1])+(wAddress-0x1000));
	else if (wAddress >= 0x2000 && wAddress < 0x3000)
	{
		if (wAddress >= 0x2000 && wAddress < 0x2400)
			return *(PPU.apbyNameTables[0] + (wAddress - 0x2000));
		else if (wAddress >= 0x2400 && wAddress < 0x2800)
			return *(PPU.apbyNameTables[1] + (wAddress - 0x2400));
		else if (wAddress >= 0x2800 && wAddress < 0x2C00)
			return *(PPU.apbyNameTables[2] + (wAddress - 0x2800));
		else
			return *(PPU.apbyNameTables[3] + (wAddress - 0x2C00));
	}
	else if (wAddress >= 0x3F00 && wAddress < 0x3F20)
		return PPU.abyPalettes[wAddress-0x3F00];
	else
		return 0;
} // end PPUReadMemory()


//------------------------------------------------------------------------------
// Name: PPUWriteMemory()
// Desc: N/A
//------------------------------------------------------------------------------
VOID __stdcall PPUWriteMemory(WORD wAddress, BYTE byData)
{
	// Pattern table write.
	if (wAddress < 0x1000)
		*((PPU.apbyPatternTables[0])+(wAddress)) = byData;
	else if (wAddress >= 0x1000 && wAddress < 0x2000)
		*((PPU.apbyPatternTables[1])+(wAddress-0x1000)) = byData;
	else if (wAddress >= 0x2000 && wAddress < 0x3000)
	{
		if (wAddress >= 0x2000 && wAddress < 0x2400)
			*(PPU.apbyNameTables[0] + (wAddress - 0x2000)) = byData;
		else if (wAddress >= 0x2400 && wAddress < 0x2800)
			*(PPU.apbyNameTables[1] + (wAddress - 0x2400)) = byData;
		else if (wAddress >= 0x2800 && wAddress < 0x2C00)
			*(PPU.apbyNameTables[2] + (wAddress - 0x2800)) = byData;
		else
			*(PPU.apbyNameTables[3] + (wAddress - 0x2C00)) = byData;
	}
	else if (wAddress >= 0x3F00 && wAddress < 0x3F20)
		PPU.abyPalettes[wAddress-0x3F00] = byData;
} // end PPUWriteMemory()