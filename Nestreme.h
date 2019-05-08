//------------------------------------------------------------------------------
// Name: Nestreme.h
// Desc: Holds all the function prototypes and other stuff for Nestreme.cpp
//------------------------------------------------------------------------------
#ifndef __NESTREME_H__
#define __NESTREME_H__


//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------
#define OPTIONS_NUM_DEBUGLINES     500
#define OPTIONS_NUM_BREAKPOINTS    10
#define OPTIONS_NUM_GFXFRAMESKIP   0
#define OPTIONS_NUM_AUDIOFRAMESKIP 0
#define NUM_SCANLINES_SCREEN    240
#define NUM_SCANLINES_VBLANK    22

#define REGISTER_LISTBOX_WIDTH 300
#define NES_PRGROM_PAGESIZE    0x4000
#define NES_CHRROM_PAGESIZE    0x1000

#define FATAL(hwnd, strError) \
	{ MessageBox(hwnd, strError, "Nestreme", MB_OK | MB_ICONSTOP); return FALSE; }
#define SAFE_DELETE_ARRAY(pArray) \
	if (pArray != NULL) delete [] pArray;
#define ListView_Check(hwndCodeLV, uListViewItem) \
	ListView_SetItemState(hwndCodeLV, uListViewItem, INDEXTOSTATEIMAGEMASK(2), LVIS_STATEIMAGEMASK);
#define ListView_UnCheck(hwndCodeLV, uListViewItem) \
	ListView_SetItemState(hwndCodeLV, uListViewItem, INDEXTOSTATEIMAGEMASK(1), LVIS_STATEIMAGEMASK);

#define ADD_VAR_TO_REG_LISTVIEW(Pos, strVar, byVar) \
	sprintf(strText, strVar); \
	InsertListViewText(hwndRegsLV, Pos, 0, strText); \
	sprintf(strText, "%02X", byVar); \
	InsertListViewText(hwndRegsLV, Pos, 1, strText); 
#define ADD_BLANK_TO_REG_LISTVIEW(Pos) \
	sprintf(strText, " "); \
	InsertListViewText(hwndRegsLV, Pos, 0, strText); 
#define UPDATE_VAR_IN_REG_LISTVIEW(Pos, strVar, byVar) \
	sprintf(strText, strVar); \
	SetListViewText(hwndRegsLV, Pos, 0, strText); \
	sprintf(strText, "%02X", byVar); \
	SetListViewText(hwndRegsLV, Pos, 1, strText);

#define PUSH_BYTE(byData) \
	CPU.Memory[0x100+CPU.S] = byData; \
	CPU.S--; 
#define POP_BYTE(byData) \
	CPU.S++; \
	byData = CPU.Memory[0x100+CPU.S]; 


//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "Cpu.h"
#include "Ppu.h"
#include "NESData.h"
#include "Joystick.h"

//-----------------------------------------------------------------------------
// Function prototypes
//-----------------------------------------------------------------------------
LRESULT CALLBACK WndProcMain(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProcDebug(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProcMemory(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

HRESULT      AddBreakpoint(WORD wAddress, UINT uListViewItem);
HRESULT      AddRegistersToListView();
HRESULT      CleanUp();
HRESULT      CPU_DoCyclicTasks();
DWORD WINAPI CPU_Run(LPVOID lpParam);
DWORD WINAPI CPU_RunUntilBreak(LPVOID lpParam);
HRESULT      CPU_Step();
HRESULT      CreateMemoryDump(HWND hwndParent, HWND* phwndEdit, HWND* phwndButton);
HRESULT      CreateDebugWindow(HWND hwndParent);
HWND         CreateListView(HWND hWndParent, UINT uFirstColRsc, UINT uNumColumns);
HRESULT      CreateMDIClientWindow(HWND hwndFrame);
HRESULT      CreateMemoryWindow(HWND hwndParent);
HWND WINAPI  CreateTabControl(HWND hwndParent, UINT uTextResource, UINT uNumTabs);
HRESULT      CreateTooltip(HWND hwnd, LPSTR strToolTip);
HRESULT      DisableBreakpoint(UINT uBreakpointIndex, UINT uListViewItem);
HRESULT      DisplayMainMemDump(HWND hwndEditCtrl);
HRESULT      DisplayPPUMemDump(HWND hwndEditCtrl);
HRESULT      DisplayPRGROMMemDump(HWND hwndEditCtrl);
HRESULT      DisplaySpriteMemDump(HWND hwndEditCtrl);
HRESULT      DisplayStackMemDump(HWND hwndEditCtrl);
HRESULT      DissassembleROM();
HRESULT      FreeNESFile();
HRESULT      InitializeApp();
HRESULT      InsertListViewText(HWND hwndLV, UINT uRow, UINT uCol, LPSTR strText);
HRESULT      LoadNESFile(HWND hwnd);
HRESULT      LoadROMToMem(LPSTR strRomFileName);
HRESULT      MemoryDumpToString(char** pstrMemory, BYTE* pMemory, UINT uStartByte, UINT uEndByte);
HRESULT      PromptForFileOpen(HWND hwndOwner);
HRESULT      ResetNES();
HRESULT      ResizeDebugControls(HWND hwndChild, HWND hwndCodeLB, HWND hwndRegLB);
HRESULT      ToggleBreakpoint(UINT uItem);
HRESULT      UpdateInstrCursorPos();
HRESULT      UpdateDebugInfo();
HRESULT      UpdateRegisters();
VOID         Wait(DWORD dwTime);

// Mapper .dll functions.
typedef BOOL (*MAPPERLOAD)(NESDATA*);
typedef BYTE (*MAPPERREAD)(WORD);
typedef BOOL (*MAPPERWRITE)(WORD, BYTE);

MAPPERLOAD  MapperOnLoad;
MAPPERREAD  MapperOnRead;
MAPPERWRITE MapperOnWrite;

//-----------------------------------------------------------------------------
// External functions needed in this file.
//-----------------------------------------------------------------------------
extern BYTE    PrintInstrToString(LPSTR strInstr, WORD wPC);


//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------
HINSTANCE g_hInstance;
HINSTANCE hinstMapperLib;


LPSTR strClassName       = "Nestreme!";
LPSTR strAppName         = "Nestreme";
LPSTR strDebugClassName  = "Nestreme_Debug!";
LPSTR strDebugTitleName  = "Nestreme code and registers";
LPSTR strMemoryClassName = "Nestreme_Memory!";
LPSTR strMemoryTitleName = "Memory Contents";
LPSTR strRunClassName    = "Nestreme_Run!";
LPSTR strRunTitleName    = "Run";


HWND hwndMain;      // Handle to the main window.
HWND hwndMDIClient; // Handle to the MDI Client window.
HWND hwndDebugWnd;  // Handle to the child window.
HWND hwndMemoryWnd; // Handle to the memory window.
HWND hwndRunWnd;    // Handle to the run window.
HWND hwndCodeLV;    // Code disassembly list view.
HWND hwndRegsLV;    // Register list view.
HWND hwndTT;        // Tooltip window.

HANDLE hThreadGo;     // Handle to the thread created when go is selected.
DWORD  dwThreadIDGo;  // Thread ID of the thread created when go is selected.
HANDLE hThreadRun;    // Handle to the thread created when go is selected.
DWORD  dwThreadIDRun; // Thread ID of the thread created when go is selected.

NES6502 CPU = {0};        // Our global CPU.
NESPPU  PPU = {0};        // Our global PPU.
BYTE*   abyPRGROM;        // Global array of program-rom.
WORD    wNumPRGROMPages;  // Number of program-rom pages.
BYTE*   abyCHRROM;        // Global array of character-rom.
WORD    wNumCHRROMPages;  // Number of character-rom pages.
BYTE    abySPRRAM[256];   // Sprite memory.
BYTE    strFileName[512]; // File name of the rom.
BYTE    abyRomHeader[16]; // The header of the rom.
WORD    wScanline;        // The current scanline that is being drawn.

// Array of addresses to break execution on,
// the number of breakpoints in the array, and a 
// boolean variable to tell us to halt execution.
WORD  awBreakpoints[OPTIONS_NUM_BREAKPOINTS+1] = {0}; 
DWORD dwNumBreakpoints = 0;
BOOL  bBreak = FALSE;

// Is our app running.
BOOL  bRunning = FALSE;

// Our options for our program.
BOOL  Options_bBackgroundEnabled = TRUE;
BOOL  Options_bSpritesEnabled    = TRUE;

// Used to toggle the first and second write of registers $2005/$2006
BYTE byVRAMAddrRegToggle = 0; 
// Address of the VRAM write register.
WORD wVRAMAddress = 0;

// The scrolling values for the scanline.
BYTE byHScroll[300];
BYTE byVScroll[300];

// The joysticks.
Joystick Joy1(0x4016);

#endif