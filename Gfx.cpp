//------------------------------------------------------------------------------
// Name: Gfx.cpp
// Desc: Holds all the graphics related functions. This includes drawing
//       scanlines, directx stuff, etc.
//------------------------------------------------------------------------------

#include "Gfx.h"
#include "Palette.h"
#include "AttributeTable.h"

// Global array of colors.
BYTE abyColors[NUMBER_OF_COLORS][4];

// DirectDraw Surfaces used for the graphics engine.
LPDIRECTDRAW7        lpDD;
LPDIRECTDRAWSURFACE7 lpddsPrimary;
LPDIRECTDRAWSURFACE7 lpddsBack;
LPDIRECTDRAWSURFACE7 lpddsScreen;
LPDIRECTDRAWCLIPPER  lpClipper;
RECT                 rcScreen;

BYTE* pSurfaceMemory; // Pointer to our locked memory to draw our pixels on.
LONG  lBytesPerPixel; // Number of bytes per pixel.
LONG  FourMinusBPP;   // 4 - Number of bytes per pixel (to speed up putpixel loop).
LONG  lSurfacePitch;  // The pitch of the scanline use to move to the next line.

//------------------------------------------------------------------------------
// Name: CreateDirectDraw()
// Desc: Creates DirectDraw and all the surfaces and sets up the colors
//       array with the paletted nes colors.
//------------------------------------------------------------------------------
HRESULT CreateDirectDraw(HWND hwnd)
{
	DDSURFACEDESC2 ddsd;
	DDPIXELFORMAT DDpf;  // DirectDraw pixel format structure.
	HRESULT ddrval;
	RECT rcClient;
   
	// Create the main DirectDraw object.
	ddrval = DirectDrawCreateEx(NULL, (VOID**)&lpDD, IID_IDirectDraw7, NULL);
	if(ddrval != DD_OK)
		return FALSE;

	// Using DDSCL_NORMAL means we will coexist with GDI.
	ddrval = lpDD->SetCooperativeLevel(hwnd, DDSCL_NORMAL);
	if(ddrval != DD_OK)
	{
		lpDD->Release();
		return FALSE;
	}
	
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

	// The primary surface is not a page flipping surface.
	ddrval = lpDD->CreateSurface(&ddsd, &lpddsPrimary, NULL);
	if (ddrval != DD_OK)
	{
		lpDD->Release();
		return FALSE;
	}

	// Create a clipper to ensure that our drawing stays inside our window.
	ddrval = lpDD->CreateClipper(0, &lpClipper, NULL);
	if (ddrval != DD_OK)
	{
		lpddsPrimary->Release();
		lpDD->Release();
		return FALSE;
	}

	// Setting it to our hwnd gives the clipper the coordinates from our window.
	ddrval = lpClipper->SetHWnd(0, hwnd);
	if (ddrval != DD_OK)
	{
		lpClipper-> Release();
		lpddsPrimary->Release();
		lpDD->Release();
		return FALSE;
	}

	// Attach the clipper to the primary surface.
	ddrval = lpddsPrimary->SetClipper(lpClipper);
	if (ddrval != DD_OK)
	{
		lpClipper-> Release();
		lpddsPrimary->Release();
		lpDD->Release();
		return FALSE;
	}

	GetClientRect(hwnd, &rcClient);
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	ddsd.dwWidth = rcClient.right - rcClient.left;
	ddsd.dwHeight = rcClient.bottom - rcClient.top;

	// Create the backbuffer separately.
	ddrval = lpDD->CreateSurface(&ddsd, &lpddsBack, NULL);
	if (ddrval != DD_OK)
	{
		lpClipper-> Release();
		lpddsPrimary->Release();
		lpDD->Release();
		return FALSE;
	}
	
	ddsd.dwWidth = SCREEN_WIDTH;
	ddsd.dwHeight = SCREEN_HEIGHT;

	// Create the offscreen surface for drawing our screen to.
	ddrval = lpDD->CreateSurface(&ddsd, &lpddsScreen, NULL);
	if (ddrval != DD_OK)
	{
		lpClipper-> Release();
		lpddsPrimary->Release();
		lpddsBack->Release();
		lpDD->Release();
		return FALSE;
	}

	// Fill the DDpf structure and get the number of bytes per pixel.
	ZeroMemory(&DDpf, sizeof(DDpf));
	DDpf.dwSize = sizeof(DDpf);
	lpddsScreen->GetPixelFormat(&DDpf);
	
	// Save the number of bytes per pixel and 4 - the number of bytes
	// per pixel to speed things up when drawing pixels.
	lBytesPerPixel = DDpf.dwRGBBitCount / 8;
	FourMinusBPP = 4 - lBytesPerPixel;

	// Compile all the pixels.
	for (int i = 0; i < NUMBER_OF_COLORS; i++)
		CompilePixel(lpddsScreen, i, abyNESPalette[(3*i)], abyNESPalette[(3*i)+1], abyNESPalette[(3*i)+2]);

	return TRUE;
} // end CreateDirectDraw()


//------------------------------------------------------------------------------
// Name: DestroyDirectDraw()
// Desc: Cleans up everything that was initialized for DirectDdraw.
//------------------------------------------------------------------------------
HRESULT DestroyDirectDraw()
{
	if(lpDD != NULL)
	{
		if (lpddsPrimary != NULL)
		{
			lpddsPrimary->Release();
			lpddsPrimary = NULL;
		}
		if (lpddsBack != NULL)
		{
			lpddsBack->Release();
			lpddsBack = NULL;
		}
		if (lpddsScreen != NULL)
		{
			lpddsScreen->Release();
			lpddsScreen = NULL;
		}
		if (lpClipper != NULL)
		{
			lpClipper->Release();
			lpClipper = NULL;
		}

		lpDD->Release();
		lpDD = NULL;
	}

	return DD_OK;
} // end DestroyDirectDraw()


//------------------------------------------------------------------------------
// Name: Flip()
// Desc: Blts the nes screen surface to the back surface and then flips
//       the back surface and the primary if in full screen mode. If in
//       windowed mode, it blts the back surface to the primary surface
//       instead of fliping them.
//------------------------------------------------------------------------------
VOID Flip()
{
	//lpddsBack->Blt(NULL, lpddsScreen, NULL, DDBLT_WAIT, NULL);
	//lpddsPrimary->Blt(NULL, lpddsBack, NULL, DDBLT_WAIT, NULL);
	lpddsPrimary->Blt(&rcScreen, lpddsScreen, NULL, DDBLT_WAIT, NULL);
} // end Flip()


//------------------------------------------------------------------------------
// Name: BeginDrawing()
// Desc: Clears the surface to the background color, locks the surface and
//       sets up the pointer to the video memory.
//------------------------------------------------------------------------------
HRESULT BeginDrawing()
{
	HRESULT ddrval;
	DDSURFACEDESC2 ddsdLockedSurf; // Surface description to lock our surface.

	// Clear to the background color which is located in bits 7-5 in reg $2001
	// when bit 0 is set, otherwise is the first entry in the background palette.
	//BYTE byColorIndex = ((CPU.Memory[0x2001] & 0xE0) >> 5) * 3;
	//ClearScreen(RGB(abyNESPalette[byColorIndex], 0, 0));
	ClearScreen(RGB(0, 0, 0));
	
	// Lock the surface.
	ddsdLockedSurf.dwSize = sizeof(DDSURFACEDESC2);
	ddrval = lpddsScreen->Lock(NULL, &ddsdLockedSurf, DDLOCK_WAIT | DDLOCK_NOSYSLOCK, NULL);
	if (ddrval == DD_OK)
	{
		// Initialize the pointer to surface memory to point to the
		// beginning of our scanline.
		pSurfaceMemory = (BYTE*)ddsdLockedSurf.lpSurface;

		// Save the pitch of the surface for later use in other functions.
		lSurfacePitch = ddsdLockedSurf.lPitch;
	}

	return ddrval;
} // end BeginDrawing()


//------------------------------------------------------------------------------
// Name: EndDrawing()
// Desc: Unlocks the drawing surface and flips the surfaces.
//------------------------------------------------------------------------------
VOID EndDrawing()
{
	// Unlock the drawing surface.
	lpddsScreen->Unlock(NULL);

	// Flip the surfaces to display what we've drawn.
	Flip();
} // end EndDrawing()


//------------------------------------------------------------------------------
// Name: DrawScanline()
// Desc: The main routine for drawing a scanline. Locks and unlocks the
//       surface and calls the scanline drawing routing for the type
//       of mirroring.
//------------------------------------------------------------------------------
VOID DrawScanline()
{
	// Save the pointer to video memory so we don't permanently modify it.
	BYTE* pTemp = pSurfaceMemory;
	// Now move the pointer to surface memory to the correct scanline.
	pSurfaceMemory += (lSurfacePitch * wScanline);

	// Draw our scanline if its enabled.
	if (CPU.Memory[0x2001] & 0x08)
		DrawScanlineH();

	// Draw all the sprites that are on the scanline if they are enabled.
	//if (CPU.Memory[0x2001] & 0x10)
	//	DrawScanlineSprites();

	// Restore our pointer to memory.
	pSurfaceMemory = pTemp;
} // end DrawScanline()


//------------------------------------------------------------------------------
// Name: DrawScanlineH()
// Desc: Draws a scanline using horizontal mirroring.
//------------------------------------------------------------------------------
VOID DrawScanlineH()
{
	BYTE*  pabyNameTables;
	BYTE*  pbyNameTableTile;
	BYTE*  pbyAttributeTable;
	BYTE*  pbyCurPatternTableAddr;
	BYTE   byCounter = 8;
	BYTE   byUpperColor = 0;
	WORD   wTileNum = 0;
	WORD   wBegTileNum = 0;
	WORD   relx = byHScroll[0];
	WORD   x = 0;
	DWORD  byScanlineMOD8 = wScanline % 8;

	__asm
	{
		// Save the registers
		pushad
	
		// The pointer to the current nametable tile will always
		// start with the first name table.
		
		// Get the bits that determine what nametable to use.
		xor ebx, ebx
		mov bl, [CPU.Memory+02000h]
		and bl, 03
		
		// Get the address of the name table and save it for later.
		lea eax, [PPU.apbyNameTables+ebx*4]
		mov pabyNameTables, eax

		// Get the pointer to the current name table and store
		// it in the variables for later. Also, store the
		// the attribute table.
		mov eax, [eax]
		mov pbyNameTableTile, eax
		mov pbyAttributeTable, eax
		add eax, 03C0h
		mov pbyAttributeTable, eax

		// Get the address of the background pattern table
		xor ebx, ebx
		mov bl, BYTE PTR [CPU.Memory+02000h]   // Address of the PPU control register #1
		and bl, PATTERN_TABLE_ADDRESS
		shr bx, 2                              // Which pattern table (0,1)
		mov ebx, [PPU.apbyPatternTables+ebx]   // Move the address of the pattern table array into ebx
		mov pbyCurPatternTableAddr, ebx        // Save the address of the pattern table for later

		//--------------------------------------------------------
		// Get the address of the tile number in the ppu memory
		// by (TILENUM * 16) + PATTERNTABLE (Yoshi)
		//--------------------------------------------------------

		and eax, 000000FFh
		mov ax, wScanline
		mov cx, relx

		// TILENUM = ((Scanline / 8) * 32) + (HScroll / 8)
		shr ax, 3        // Scanline / 8
		shl ax, 5        // Multiply by 32

		// Save the beginning tile number so we don't have
		// to recalculate it when we change name tables.
		mov wBegTileNum, ax

		shr cx, 3        // HScroll / 8
		add ax, cx       // ax=TILENUM
		
		// Save the tile number so we can increment it.
		mov wTileNum, ax

		// Now add the tile offset to the base address of the
		// name table to get the address of the tile
		add pbyNameTableTile, eax

		// Get the upper color of the tile. This is done by
		// getting the attribute byte and then ANDing the byte
		// by the correct bits for the tile (see yoshi's nes doc).
		// Rather that doing a series of calculations to find the
		// attribute offset and what bits to and the byte with, 
		// all that info is stored in lookup table arrays.
  		xor ebx, ebx
		mov ecx, pbyAttributeTable
		mov bl, [AttributeBytes+eax] // bl=offset into the attribute table
		mov dl, [ecx+ebx]            // dl=attribute byte
		mov dh, [AttributeBits+eax]  // dh=attribute bits to AND the attribute byte by
		mov cl, [UpColRotateVal+eax] // cl=number of bits to rotate the upper color by
		and dl, dh                   // dl=upper color for the tile but in the right bit position
		ror dl, cl                   // dl=upper color of the tile
		mov byUpperColor, dl

		// TILEADDRESS = (TILENUM * 16) + PATTERNTABLE
		xor eax, eax
		mov ecx, pbyNameTableTile
		mov al, BYTE PTR [ecx]
		shl ax, 4
		add eax, pbyCurPatternTableAddr // eax = TILEADDRESS


		// SCANLINE_ADDRESS = TILEADDRESS + (Scanline % 8)
		add eax, byScanlineMOD8 // eax=SCANLINE_ADDRESS

		// Adjust the tile for horizontal scrolling
		xor ecx, ecx
		mov cx, relx
		and cx, 7
		mov edi, 8
		sub edi, ecx
		mov bl, [eax]
		mov dl, [eax+8]
		rcl bl, cl
		rcl dl, cl
		
		jmp dsh_SkipForFirstTile

		//-----------------------------
		// Draw the piece of the tile
		//-----------------------------

	dsh_DrawTileBeginning:	

		// Move the pattern table bytes into registers.
		mov bl, [eax]
		mov dl, [eax+8]

	dsh_SkipForFirstTile:

		// Move the x coordinate into a register to speed up things.
		mov si, x

	dsh_DrawTileLoop:

		// Calculate the color of the pixel, where eax
		// points to the memory location of the pattern table
		// tile being used.
		rcl bl, 1
		setc cl
		rcl dl, 1
		setc al
		shl al, 1
		or cl, al

		// Draw the pixel if the color is not 0
		jz dsh_DontDrawPixel
		
		// Add the upper part of the color to the color.
		// NOW bl=The index into the NES's palette.
		or cl, byUpperColor
		
		// Save all our registers so they don't get modified
		// in the put pixel routine.
		push eax
		push ebx
		push edx
		push esi

		// Put the pixel on the screen surface.
		push ecx
		call PutBackgroundPixel

		// Restore all our saved registers.
		pop esi
		pop edx
		pop ebx
		pop eax

	dsh_DontDrawPixel:

		// Advance the pointer to video memory to the next pixel.
		mov ecx, lBytesPerPixel
		add pSurfaceMemory, ecx

		// Move our position indicators to the next pixel.
		inc si
		inc relx

		// Check to see if were done with the scanline
		cmp si, SCREEN_WIDTH
		jge dsh_Done

		// Decrement our counter by 1, when its zero that means we've
		// drawn all the pixels for that tile on the current scanline.
		dec edi
		jnz dsh_DrawTileLoop
		
		// Restore the x variable.
		mov x, si

		// We may need to change name tables during the process
		cmp relx, SCREEN_WIDTH
		jge dsh_NewNameTable

		// Get the new tile by incrementing the pointer that
		// points to the current tile and then getting the 
		// new tile address into the pattern table, then get
		// the scanline address
		inc pbyNameTableTile
		inc wTileNum

	dsh_GetTileAddress:
		
		// Get the upper color of the tile. This is done by
		// getting the attribute byte and then ANDing the byte
		// by the correct bits for the tile (see yoshi's nes doc).
		// Rather that doing a series of calculations to find the
		// attribute offset and what bits to and the byte with, 
		// all that info is stored in lookup table arrays.
		xor ebx, ebx
		xor edi, edi
		mov di, wTileNum
		mov ecx, pbyAttributeTable
		mov bl, [AttributeBytes+edi] // bl=offset into the attribute table
		mov dl, [ecx+ebx]            // dl=attribute byte
		mov dh, [AttributeBits+edi]  // dh=attribute bits to AND the attribute byte by
		mov cl, [UpColRotateVal+edi] // cl=number of bits to rotate the upper color by
		and dl, dh                   // dl=upper color for the tile but in the right bit position
		ror dl, cl                   // dl=upper color of the tile
		mov byUpperColor, dl

		// Get the index into the pattern table and store it in al
		xor eax, eax
		mov ebx, pbyNameTableTile
		mov al, BYTE PTR [ebx]

		// TILEADDRESS = (TILENUM * 16) + PATTERNTABLE
		shl eax, 4
		add eax, pbyCurPatternTableAddr ; eax = TILEADDRESS

		// SCANLINE_ADDRESS = TILEADDRESS + (Scanline % 8)
		add eax, byScanlineMOD8 ; ax=SCANLINE_ADDRESS

		mov edi, 8
		jmp dsh_DrawTileBeginning

	dsh_NewNameTable:
		// Get the beggining tile number that we computed at the
		// beggining of this function since it is the same
		// it just uses a different tile number.
		// NOTE: HScroll does not play a factor in this since
		//       we will always start at the left of the new name table
		xor eax, eax
		mov ax, wBegTileNum
		mov wTileNum, ax

		// Now add the offset to the new name table address
		// to get the new tile address
		mov ebx, pabyNameTables
		add ebx, 4
		mov ebx, [ebx]
		mov pbyNameTableTile, ebx
		mov pbyAttributeTable, ebx
		add pbyAttributeTable, 03C0h
		add pbyNameTableTile, eax

		mov relx, 0

		xor eax, eax
		mov ebx, pbyNameTableTile
		mov al, byte ptr [ebx]    // al=TILENUM

		// TILEADDRESS = (TILENUM * 16) + PATTERNTABLE
		shl eax, 4
		add eax, pbyCurPatternTableAddr // eax = TILEADDRESS

		// SCANLINE_ADDRESS = TILEADDRESS + (Scanline % 8)
		add eax, byScanlineMOD8 // ax=SCANLINE_ADDRESS

		// Get the upper color of the tile. This is done by
		// getting the attribute byte and then ANDing the byte
		// by the correct bits for the tile (see yoshi's nes doc).
		// Rather that doing a series of calculations to find the
		// attribute offset and what bits to and the byte with, 
		// all that info is stored in lookup table arrays.
		xor ebx, ebx
		xor edi, edi
		mov di, wTileNum
		mov ecx, pbyAttributeTable
		mov bl, [AttributeBytes+edi] // bl=offset into the name table
		add ecx, ebx                 // now points to attribute byte for the tile
		mov dl, [ecx]                // dl=attribute byte
		mov dh, [AttributeBits+edi]  // dh=attribute bits to AND the attribute byte by
		and dl, dh                   // dl=upper color for the tile but in the right bit position
		mov cl, [UpColRotateVal+edi] // cl=number of bits to rotate the upper color by
		ror dl, cl                   // dl=upper color of the tile
		mov byUpperColor, dl

		mov edi, 8
		jmp dsh_DrawTileBeginning

	dsh_Done:
		popad
	}
	
	return;
} // end DrawScanlineH()


//------------------------------------------------------------------------------
// Name: DoSprite0()
// Desc: Sets the sprite #0 flag when the scanline equals the y coordinate
//       of sprite #0
//------------------------------------------------------------------------------
VOID DoSprite0()
{
	// If the scanline = the y coordinate of sprite #0, then
	// set bit #6 of register $2002
	if (wScanline == (abySPRRAM[0] + 8))
		CPU.Memory[0x2002] |= 0x40;
} // end DoSprite0()


//------------------------------------------------------------------------------
// Name: DrawScanlineSprites()
// Desc: Draws all the sprites that on the current scanline.
//------------------------------------------------------------------------------
VOID DrawScanlineSprites()
{
	// Draw Sprite #0 separatly from the rest of the sprites
	// becuase we have to check for the Sprite #0 hit flag.
	DoSprite0();

	// Loop through the rest of the sprites and draw each
	// scanline of the sprite.
	for (int i = 0; i < 63; i++)
		DrawSpriteLine(i);


} // end DrawScanlineSprites()


//------------------------------------------------------------------------------
// Name: DrawSprites()
// Desc: Draws all the sprites either in front or behind the background.
//------------------------------------------------------------------------------
VOID DrawSprites(BYTE byPriority)
{
	// Loop through all the sprites and display them if their
	// priority bit is equal to the bit passed in.
	if (CPU.Memory[0x2001] & 0x10)
		for (int i = 63; i >= 0; i--)
			if ((abySPRRAM[((i*4)+2)]&0x20) == byPriority)
				DrawSprite(i);
} // end DrawSprites()


//------------------------------------------------------------------------------
// Name: DrawSprite()
// Desc: Draws a 8x8 or 8x16 pixel sprite.
//------------------------------------------------------------------------------
VOID DrawSprite(BYTE bySpriteNum)
{
	BYTE  byColor;             // Index into the NES palette for the sprite.
	BYTE* pbyTileByte;         // Pointer to the pattern table byte.
	BYTE* pSaveVideoMemStart = pSurfaceMemory;

	// Save all the information about the sprite.
	BYTE byYPos = abySPRRAM[(bySpriteNum*4)] + 1;
	BYTE byTileNum = abySPRRAM[(bySpriteNum*4)+1];
	BYTE byAttributes = abySPRRAM[(bySpriteNum*4)+2];
	BYTE byXPos = abySPRRAM[(bySpriteNum*4)+3];
	BYTE byUpperColor = (byAttributes & 0x3) << 2;

	// Get the pointer to the tile byte in the pattern table for the sprite.
	pbyTileByte = (byTileNum * 16) + PPU.apbyPatternTables[(CPU.Memory[0x2000]&0x08)>>3];
	
	// Move the video memory pointer to where the sprite is supposed to be horizontally.
	if (byAttributes & 0x40)
		pSurfaceMemory += (byXPos * lBytesPerPixel) + (7 * lBytesPerPixel);
	else
        pSurfaceMemory += byXPos * lBytesPerPixel;

	// Move the video memory pointer to where the sprite is supposed to be vertically.
	if (byAttributes & 0x80)
		pSurfaceMemory += ((byYPos + 7) * lSurfacePitch);
	else
        pSurfaceMemory += byYPos * lSurfacePitch;

	// Draw the sprite by drawing each tile line then moving to the next byte.
	for (int i = 0; i < 8; i++)
	{
		BYTE  byTestBit = 0x80;    // Color testing bit for the sprite tile.
		BYTE  byColorShiftVal = 7; // Number of bits to shift the color by.

		while (byTestBit != 0)
		{
			// The color is the to lower bits from the pattern table
			// obtained in the same way as a background tile plus the
			// two uppercolor bits in the lower 2 bits of the attribute
			// byte.
			byColor = ((*pbyTileByte & byTestBit) >> byColorShiftVal) |
				(((*(pbyTileByte+8) & byTestBit) >> byColorShiftVal) << 1);

			// Draw the pixel on the screen if the color is not 0.
			if (byColor != 0)
				PutSpritePixel(byColor | byUpperColor);

			// Move to the next pixel.
			if (byAttributes & 0x40)
				pSurfaceMemory -= lBytesPerPixel;
			else
				pSurfaceMemory += lBytesPerPixel;
			
			byColorShiftVal--;
			byTestBit >>= 1;
		}

		// Reset the horizontal video position for the next line.
		if (byAttributes & 0x40)
			pSurfaceMemory += (8 * lBytesPerPixel);
		else
			pSurfaceMemory -= (8 * lBytesPerPixel);

		// Move to the next line.
		if (byAttributes & 0x80)
			pSurfaceMemory -= lSurfacePitch;
		else
			pSurfaceMemory += lSurfacePitch;

		// Move to the next byte in the sprite.
		pbyTileByte++;
	}

	// Restore the surface memory pointer to the beginning
	// of the scanline.
	pSurfaceMemory = pSaveVideoMemStart;

	//-------------------------------------------------------------------------
	// Draw the second part of the sprite if its a 8x16 sprite.
	// Thats located in in bit 5 of reg $2000.
	//-------------------------------------------------------------------------
	if (CPU.Memory[0x2000] & 0x20)
	{
		BYTE  byColor;             // Index into the NES palette for the sprite.
		BYTE* pbyTileByte;         // Pointer to the pattern table byte.
		BYTE* pSaveVideoMemStart = pSurfaceMemory;

		// Save all the information about the sprite.
		BYTE byYPos = abySPRRAM[(bySpriteNum*4)] + 8;
		BYTE byTileNum = abySPRRAM[(bySpriteNum*4)+1] + 1;
		BYTE byAttributes = abySPRRAM[(bySpriteNum*4)+2];
		BYTE byXPos = abySPRRAM[(bySpriteNum*4)+3];
		BYTE byUpperColor = (byAttributes & 0x3) << 2;

		// Get the pointer to the tile byte in the pattern table for the sprite.
		pbyTileByte = (byTileNum * 16) + PPU.apbyPatternTables[(CPU.Memory[0x2000]&0x08)>>3];
		
		// Move the video memory pointer to where the sprite is supposed to be horizontally.
		if (byAttributes & 0x40)
			pSurfaceMemory += (byXPos * lBytesPerPixel) + (8 * lBytesPerPixel);
		else
			pSurfaceMemory += byXPos * lBytesPerPixel;

		// Move the video memory pointer to where the sprite is supposed to be vertically.
		if (byAttributes & 0x80)
			pSurfaceMemory += ((byYPos + 8) * lSurfacePitch);
		else
			pSurfaceMemory += byYPos * lSurfacePitch;

		// Draw the sprite by drawing each tile line then moving to the next byte.
		for (int i = 0; i < 8; i++)
		{
			BYTE  byTestBit = 0x80;    // Color testing bit for the sprite tile.
			BYTE  byColorShiftVal = 7; // Number of bits to shift the color by.

			while (byTestBit != 0)
			{
				// The color is the to lower bits from the pattern table
				// obtained in the same way as a background tile plus the
				// two uppercolor bits in the lower 2 bits of the attribute
				// byte.
				byColor = ((*pbyTileByte & byTestBit) >> byColorShiftVal) |
					(((*(pbyTileByte+8) & byTestBit) >> byColorShiftVal) << 1);

				// Draw the pixel on the screen if the color is not 0.
				if (byColor != 0)
					PutSpritePixel(byColor | byUpperColor);

				// Move to the next pixel.
				if (byAttributes & 0x40)
					pSurfaceMemory -= lBytesPerPixel;
				else
					pSurfaceMemory += lBytesPerPixel;
				
				byColorShiftVal--;
				byTestBit >>= 1;
			}

			// Reset the horizontal video position for the next line.
			if (byAttributes & 0x40)
				pSurfaceMemory += (8 * lBytesPerPixel);
			else
				pSurfaceMemory -= (8 * lBytesPerPixel);

			// Move to the next line.
			if (byAttributes & 0x80)
				pSurfaceMemory -= lSurfacePitch;
			else
				pSurfaceMemory += lSurfacePitch;

			// Move to the next byte in the sprite.
			pbyTileByte++;
		}

		// Restore the surface memory pointer to the beginning
		// of the scanline.
		pSurfaceMemory = pSaveVideoMemStart;
	}
} // end DrawSprite()


//------------------------------------------------------------------------------
// Name: DrawSpriteLine()
// Desc: Draws a 8 pixel scanline for the sprite.
//------------------------------------------------------------------------------
VOID DrawSpriteLine(BYTE bySpriteNum)
{
	BYTE  byTestBit = 0x80;    // Color testing bit for the sprite tile.
	BYTE  byColorShiftVal = 7; // Number of bits to shift the color by.
	BYTE  byColor;             // Index into the NES palette for the sprite.
	BYTE* pbyScanlineByte;     // Pointer to the pattern table byte.
	BYTE* pSaveVideoMemStart = pSurfaceMemory;

	// Save all the information about the sprite.
	BYTE byYPos = abySPRRAM[(bySpriteNum*4)];
	BYTE byTileNum = abySPRRAM[(bySpriteNum*4)+1];
	BYTE byAttributes = abySPRRAM[(bySpriteNum*4)+2];
	BYTE byXPos = abySPRRAM[(bySpriteNum*4)+3];
	BYTE byUpperColor = (byAttributes & 0x3) << 2;

	// If one of the sprites scanlines is on this screen scaline,
	// then we have to draw that sprite scanline.
	if ((unsigned)(wScanline - byYPos) < 8)
	{
		// Get the pointer to the scanline byte in the pattern table for the sprite.
		pbyScanlineByte = (byTileNum * 16) + PPU.apbyPatternTables[(CPU.Memory[0x2000]&0x08)>>3];
		pbyScanlineByte += (wScanline - byYPos);
		
		// Move the video memory pointer to where the sprite is supposed to be.
		if (byAttributes & 0x40)
			pSurfaceMemory += (byXPos * lBytesPerPixel) + (7 * lBytesPerPixel);
		else
            pSurfaceMemory += byXPos * lBytesPerPixel;

		// Draw the sprite scanline.
		while (byTestBit != 0)
		{
			// The color is the to lower bits from the pattern table
			// obtained in the same way as a background tile plus the
			// two uppercolor bits in the lower 2 bits of the attribute
			// byte.
			byColor = ((*pbyScanlineByte & byTestBit) >> byColorShiftVal) |
				(((*(pbyScanlineByte+8) & byTestBit) >> byColorShiftVal) << 1) |
				byUpperColor;

			// Draw the pixel on the screen if the color is not 0.
			if (byColor != 0)
				PutSpritePixel(byColor);

			// Move to the next pixel.
			if (byAttributes & 0x40)
				pSurfaceMemory -= lBytesPerPixel;
			else
				pSurfaceMemory += lBytesPerPixel;
			
			byColorShiftVal--;
			byTestBit >>= 1;
		}

		// Restore the surface memory pointer to the beginning
		// of the scanline.
		pSurfaceMemory = pSaveVideoMemStart;
	}
} // end DrawSpriteLine()


//------------------------------------------------------------------------------
// Name: GetMaskInfo()
// Desc: Uses the color's bitmasks to calculate values which we 
//       will use to compile the pixel value.
//------------------------------------------------------------------------------
INT GetMaskInfo(DWORD dwBitmask, int* pnShift)
{
	int nPrecision = 0;
	int nShift     = 0;

	// Count the zeros on right hand side.
	while (!(dwBitmask & 0x01))
	{
		dwBitmask >>= 1;
		nShift++;
	}

	// Count the ones on right hand side.
	while (dwBitmask & 0x01)
	{
		dwBitmask >>= 1;
		nPrecision++;
	}

	// Save the shift value.
	*pnShift = nShift;

	// Return the number of ones.
	return nPrecision;
} // end GetMaskInfo()


//------------------------------------------------------------------------------
// Name: CompilePixel()
// Desc: Stores the four bytes corresponding to the compiled pixel 
//       value in a class variable.
//------------------------------------------------------------------------------
VOID CompilePixel(LPDIRECTDRAWSURFACE7 lpSurf, int nColorIndex, int r, int g, int b)
{
	DDPIXELFORMAT DDpf;    // DirectDraw pixel format structure.
	int rsz, gsz, bsz; 	   // Bitsize of field.
	int rsh, gsh, bsh;	   // 0’s on left (the shift value).
	DWORD dwCompiledPixel; // Our pixel with r,g,b values compiled.

	// Get the pixel format.
	ZeroMemory (&DDpf, sizeof(DDpf));
	DDpf.dwSize = sizeof(DDpf);
	lpSurf->GetPixelFormat(&DDpf);

	// Get the shift and precision values and store them.
	rsz = GetMaskInfo(DDpf.dwRBitMask, &rsh);
	gsz = GetMaskInfo(DDpf.dwGBitMask, &gsh);
	bsz = GetMaskInfo(DDpf.dwBBitMask, &bsh);

	// Keep only the MSB bits of component.
	r >>= (8-rsz);
	g >>= (8-gsz);
	b >>= (8-bsz);

	// Shift them into place.
	r <<= rsh;
	g <<= gsh;
	b <<= bsh;

	// Store the compiled pixel.
	dwCompiledPixel = (DWORD)(r | g | b);
	abyColors[nColorIndex][3] = (BYTE)(dwCompiledPixel);
	abyColors[nColorIndex][2] = (BYTE)(dwCompiledPixel >>= 8);
	abyColors[nColorIndex][1] = (BYTE)(dwCompiledPixel >>= 8);
	abyColors[nColorIndex][0] = (BYTE)(dwCompiledPixel >>= 8);

	return;
} // end CompilePixel()


//------------------------------------------------------------------------------
// Name: PutBackgroundPixel()
// Desc: Puts a background pixel independant of the bit depth 
//       on the locked surface.
//------------------------------------------------------------------------------
VOID __stdcall PutBackgroundPixel(BYTE nColor)
{
	// Copy the pointer to video memory so the main one doesn't get modified.
	BYTE *pVideoMem = pSurfaceMemory;
	// Speed up the array access in the loop by storing this here.
	BYTE byColor = PPU.abyPalettes[nColor];

	// Put the number of bytes for each pixel on the surface.
	for (int i = 3; i >= FourMinusBPP; i--)
	{
		*pVideoMem = abyColors[byColor][i];
		pVideoMem++;
	}
} // end PutBackgroundPixel()


//------------------------------------------------------------------------------
// Name: PutSpritePixel()
// Desc: Puts a background pixel independant of the bit depth 
//       on the locked surface.
//------------------------------------------------------------------------------
VOID __stdcall PutSpritePixel(BYTE nColor)
{
	// Copy the pointer to video memory so the main one doesn't get modified.
	BYTE *pVideoMem = pSurfaceMemory;
	// Speed up the array access in the loop by storing this here.
	BYTE byColor = PPU.abyPalettes[0x10+nColor];

	// Put the number of bytes for each pixel on the surface.
	for (int i = 3; i >= FourMinusBPP; i--)
	{
		*pVideoMem = abyColors[byColor][i];
		pVideoMem++;
	}
} // end PutSpritePixel()


//------------------------------------------------------------------------------
// Name: ClearScreen()
// Desc: Clears the NES screen to the background color.
//------------------------------------------------------------------------------
VOID ClearScreen(COLORREF crColor)
{
	DDBLTFX ddbltfx;

	// Fill the NES screen surface with the background color.
	ddbltfx.dwSize = sizeof(DDBLTFX);
	ddbltfx.dwFillColor = crColor;
	lpddsScreen->Blt(NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx);
} // end ClearScreen()


//------------------------------------------------------------------------------
// Name: UpdateBounds()
// Desc: Updates the rect for ......
//------------------------------------------------------------------------------
HRESULT UpdateBounds(HWND hwnd)
{
	POINT ptWindow; // Point used for converting client to window coordinates.

	// Get the client rectangle.
	GetClientRect(hwnd, &rcScreen);

	// Convert the top left coordinates.
	ptWindow.x = rcScreen.left;
	ptWindow.y = rcScreen.top;
	ClientToScreen(hwnd, &ptWindow);
	rcScreen.left = ptWindow.x;
	rcScreen.top = ptWindow.y;

	// Convert the bottom right coordinates.
	ptWindow.x = rcScreen.right;
	ptWindow.y = rcScreen.bottom;
	ClientToScreen(hwnd, &ptWindow);
	rcScreen.right = ptWindow.x;
	rcScreen.bottom = ptWindow.y;

	return S_OK;
} // end UpdateBounds()


//------------------------------------------------------------------------------
// Name: OutputText()
// Desc: Outputs text to the screen surface.
//------------------------------------------------------------------------------
VOID OutputText(LPSTR strText, INT nX, INT nY, COLORREF cfFront, COLORREF cfBack)
{
	HDC hdc;

	if (lpddsScreen->GetDC(&hdc) == DD_OK)
	{
		SetBkColor(hdc, cfBack);
		SetTextColor(hdc, cfFront);
		TextOut(hdc, nX, nY, strText, lstrlen(strText));
		lpddsScreen->ReleaseDC(hdc);
	}
} // end OutputText()