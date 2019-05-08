//------------------------------------------------------------------------------
// Name: Gfx.h
// Desc: Header file for Gfx.cpp
//------------------------------------------------------------------------------

#ifndef __GFX_H__
#define __GFX_H__

//------------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------
#define SPRITE_BEHIND         0x20
#define SPRITE_INFRONT        0
#define TILE_X_SIZE           8
#define TILE_Y_SIZE           8
#define NUM_X_TILES           32
#define NUM_Y_TILES           32
#define SCREEN_WIDTH          256
#define SCREEN_HEIGHT         240
#define PATTERN_TABLE_ADDRESS 00010000b
#define NAME_TABLE_ADDRESS_H  00000011b
#define NUMBER_OF_COLORS      64        // Set to however many colors we need.


// Includes
#include <ddraw.h>
#include "Cpu.h"
#include "Ppu.h"


// Functions prototypes.
HRESULT BeginDrawing();
VOID    ClearScreen(COLORREF crColor);
VOID    CompilePixel(LPDIRECTDRAWSURFACE7 lpSurf, int nColorIndex, int r, int g, int b);
HRESULT CreateDirectDraw(HWND hwnd);
HRESULT DestroyDirectDraw();
VOID    DoSprite0();
VOID    DrawScanline();
VOID    DrawScanlineH();
VOID    DrawScanlineSprites();
VOID    DrawSpriteLine(BYTE bySpriteNum);
VOID    DrawSprite(BYTE bySpriteNum);
VOID    DrawSprites(BYTE byPriority);
VOID    EndDrawing();
VOID    Flip();
INT     GetMaskInfo(DWORD dwBitmask, int* pnShift);
VOID    __stdcall PutBackgroundPixel(BYTE nColor);
VOID    __stdcall PutSpritePixel(BYTE nColor);

HRESULT UpdateBounds(HWND hwnd);
VOID    OutputText(LPSTR strText, INT nX, INT nY, COLORREF cfFront, COLORREF cfBack);

// External variables needed.
extern NES6502 CPU;
extern NESPPU  PPU;
extern WORD    wScanline;
extern BYTE    abySPRRAM[256];
extern BYTE    byHScroll[300];
extern BYTE    byVScroll[300];

#endif