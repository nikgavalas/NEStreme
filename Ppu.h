//------------------------------------------------------------------------------
// Name: Ppu.h
// Desc: Holds the definition of the PPU structure.
//------------------------------------------------------------------------------
#ifndef __PPU_H__
#define __PPU_H__


/* 
 * The following is cut from yoshi's NES document NESTECH.txt
 *
  B. Memory Map
  -------------
    Included here are two (2) memory maps. The first is a "RAM Memory
    Map," which despite being less verbose describes the actual regions
    which point to physical RAM in the NES itself. The second is a
    "Programmer Memory Map" which is quite verbose and describes the
    entire memory region of the NES and how it's used/manipulated.

        RAM Memory Map
      +---------+-------+--------------------+
      | Address | Size  | Description        |
      +---------+-------+--------------------+
      | $0000   | $1000 | Pattern Table #0   |
      | $1000   | $1000 | Pattern Table #1   |
      | $2000   | $800  | Name Tables        |
      | $3F00   | $20   | Palettes           |
      +---------+-------+--------------------+


        Programmer Memory Map
      +---------+-------+-------+--------------------+
      | Address | Size  | Flags | Description        |
      +---------+-------+-------+--------------------+
      | $0000   | $1000 | C     | Pattern Table #0   |
      | $1000   | $1000 | C     | Pattern Table #1   |
      | $2000   | $3C0  |       | Name Table #0      |
      | $23C0   | $40   |  N    | Attribute Table #0 |
      | $2400   | $3C0  |  N    | Name Table #1      |
      | $27C0   | $40   |  N    | Attribute Table #1 |
      | $2800   | $3C0  |  N    | Name Table #2      |
      | $2BC0   | $40   |  N    | Attribute Table #2 |
      | $2C00   | $3C0  |  N    | Name Table #3      |
      | $2FC0   | $40   |  N    | Attribute Table #3 |
      | $3000   | $F00  |   R   |                    |
      | $3F00   | $10   |       | Image Palette #1   |
      | $3F10   | $10   |       | Sprite Palette #1  |
      | $3F20   | $E0   |    P  |                    |
      | $4000   | $C000 |     F |                    |
      +---------+-------+-------+--------------------+
                          C = Possibly CHR-ROM
                          N = Mirrored (see Subsection G)
                          P = Mirrored (see Subsection H)
                          R = Mirror of $2000-2EFF (VRAM)
                          F = Mirror of $0000-3FFF (VRAM)
*/


// The structure for the PPU of the Nintendo.
typedef struct tagNESPPU
{
	BYTE* apbyPatternTables[2]; // The 2 pointers to the pattern tables.
	BYTE* apbyNameTables[4];    // The 4 pointers to the name tables.
	BYTE  abyNameTables[0x800]; // The actual memory for the name tables.
	                            // Since the NES only has enough physical RAM
	                            // for 2 name tables, the rest must be stored
	                            // on the Cart; this is mapper specific.

	BYTE  abyPalettes[0x20];    // Memory for the palettes.
} NESPPU;

#endif 