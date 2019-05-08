//------------------------------------------------------------------------------
// Name: Nestreme.cpp
// Desc: 
//
// Things still left to add in the debugger.
// =========================================
// 1.  Viewing all the memory, main, stack, prg-rom, chr-rom
// 2.  Adding the memory registers to the register list view.
// 3.  Better breakpoints, adding in dialog box, setting when dissassembling.
// 4.  Step over.
// 5.  View screen shot.
// 6.  Editing memory.
// 7.  
//
// Things to add to the CPU.
// =========================
// 1.  More acurate cpu cycle emulation.
// 2.  Test all the instructions.
// 3.  Update cpu bytes and cycles array.
//
// Things to add to the PPU.
// =========================
// 1.  Accurate emulation of registers 2005/2006.
//------------------------------------------------------------------------------


// Includes
#include <windows.h>
#include <commctrl.h>
#include <afxres.h>
#include <stdio.h>
#include "resource.h"

#include "Nestreme.h"
#include "Cpu.h"
#include "Io.h"
#include "PrintInstrToString.h"
#include "Gfx.h"
#include "Sound.h"
#include "NESData.h"


//------------------------------------------------------------------------------
// Name: WinMain()
// Desc: Main entry point for a windows application.
//------------------------------------------------------------------------------
INT WINAPI WinMain(HINSTANCE hInstance,
				   HINSTANCE hPrevInst,
				   LPSTR lpCmdLine,
				   int nCmdShow)
{
	MSG    msg;
	HACCEL hAccel;

	// Save the global variables
	g_hInstance = hInstance;

	// Initialize our classes.
	if (InitializeApp() != S_OK)
		return TRUE;

	// Create our main window
	hwndMain = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW,
							  strClassName,
							  strAppName,
							  WS_OVERLAPPEDWINDOW,
							  CW_USEDEFAULT,
							  CW_USEDEFAULT,
							  CW_USEDEFAULT,
							  CW_USEDEFAULT,
							  NULL,
							  NULL,
							  hInstance,
							  NULL);

	// Make sure we could create our window.
	if (hwndMain == NULL)
		return TRUE;

	// Load the keyboard accelerators.
	hAccel = LoadAccelerators(g_hInstance, MAKEINTRESOURCE(IDR_MAINACCEL));

	// Show and update our window.
	ShowWindow(hwndMain, SW_SHOWMAXIMIZED);
	UpdateWindow(hwndMain);

	// Now enter our main windows messaging loop.
	while (GetMessage(&msg, NULL, 0, 0))
	{
	    if (!TranslateAccelerator(hwndMain, hAccel, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	} // end windows messaging loop.

	// Make sure to delete everything we allocate.
	CleanUp();

	return msg.wParam;
} // end WinMain()


//------------------------------------------------------------------------------
// Name: WndProcMain()
// Desc: Our message handling procedure for windows messages.
//------------------------------------------------------------------------------
LRESULT CALLBACK WndProcMain(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HANDLE hThreadGo;    // Handle to the thread created when go is selected.
	DWORD  dwThreadIDGo; // Thread ID of the thread created when go is selected.

	switch (uMsg)
	{
		case WM_QUIT:
		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		case WM_CREATE: 
			CreateMDIClientWindow(hwnd);
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case ID_DEBUG_BREAK:
					bBreak = TRUE;
					break;

				case ID_DEBUG_GO:
					hThreadGo = CreateThread(NULL, 0, CPU_RunUntilBreak, 
						NULL, NULL, &dwThreadIDGo);
					if (hThreadGo == NULL)
						FATAL(hwnd, "Couldn't create thread for execution.");
					break;

				case ID_DEBUG_RESTART:
					ResetNES();
					break;

				case ID_DEBUG_STEPINTO:
					CPU_Step();
					break;

				case ID_FILE_FREE:
					FreeNESFile();
					break;

				case ID_FILE_OPEN:
					LoadNESFile(hwnd);
					break;

				case ID_FILE_RUN:
					bRunning = TRUE;
					hThreadRun = CreateThread(NULL, 0, CPU_Run, 
						NULL, NULL, &dwThreadIDRun);
					if (hThreadRun == NULL)
						FATAL(hwnd, "Couldn't create thread for execution.");
					break;

				case ID_FILE_EXIT:
					PostQuitMessage(0);
					break;

				case ID_VIEW_MEMORY:
					CreateMemoryWindow(hwnd);
					break;

				case ID_VIEW_DISASSEMBLY:
					CreateDebugWindow(hwnd);
					DissassembleROM();
					UpdateDebugInfo();
					break;

				case ID_WINDOW_TILEHORIZONTALLY:
					SendMessage(hwndMDIClient, WM_MDITILE, MDITILE_HORIZONTAL, 0);
					break;

				case ID_WINDOW_TILEVERTICALLY:
					SendMessage(hwndMDIClient, WM_MDITILE, MDITILE_VERTICAL, 0);
					break;
			}

		default:
			return DefFrameProc(hwnd, hwndMDIClient, uMsg, wParam, lParam);
	}

	return 0;
} // end WndProcMain()


//------------------------------------------------------------------------------
// Name: WndProcDebug()
// Desc: Our message handling procedure for windows messages.
//------------------------------------------------------------------------------
LRESULT CALLBACK WndProcDebug(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_NOTIFY:
			// Process notification messages.            
			switch (((LPNMHDR)lParam)->code) 
			{ 
				case NM_DBLCLK:
					ToggleBreakpoint(((LPNMITEMACTIVATE)lParam)->iItem);
					break;
			}
			break;

		case WM_SIZE:
			ResizeDebugControls(hwndDebugWnd, hwndCodeLV, hwndRegsLV);

		default:
			return DefMDIChildProc(hwnd, uMsg, wParam, lParam);
	}

	return 0;
} // end WndProcDebug()


//------------------------------------------------------------------------------
// Name: WndProcMemory()
// Desc: Our message handling procedure for windows messages.
//------------------------------------------------------------------------------
LRESULT CALLBACK WndProcMemory(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HWND hwndEdit = NULL;      // Handle to the edit control to hold the memory contents.
	static HWND hwndButton = NULL;    // Handle to the refresh button.
	static HWND hwndMemoryTab = NULL; // Handle to the memory tab control.

	switch (uMsg)
	{
		case WM_COMMAND:
			if (HIWORD(wParam) == BN_CLICKED) 
			{
				// Display the memory that corresponds, on what tab is selected.
				switch (TabCtrl_GetCurSel(hwndMemoryTab))
				{
					case 0:
						// Dump the main memory to the control.
						DisplayMainMemDump(hwndEdit);
						break;
					
					case 1:
						// Dump the stack memory to the control.
						DisplayStackMemDump(hwndEdit);
						break;

					case 2:
						// Dump the PRG-ROM memory to the control.
						DisplayPRGROMMemDump(hwndEdit);
						break;

					case 3:
						// Dump the PPU memory to the control.
						DisplayPPUMemDump(hwndEdit);
						break;

					case 4:
						// Dump the Sprite memory to the control.
						DisplaySpriteMemDump(hwndEdit);
						break;

					default:
						break;
				}
			}
			break;

		case WM_CREATE:
			// Create the tab control to hold all the different types of memory.
			if ((hwndMemoryTab = CreateTabControl(hwnd, IDS_MEMORY_MAIN, 5)) == NULL)
				FATAL(hwnd, "Couldn't create tab control.");

			// Create the edit control and initialize it.
			if (CreateMemoryDump(hwnd, &hwndEdit, &hwndButton) != S_OK)
				FATAL(hwnd, "Couldn't create the edit window.")

			// Dump the memory to the control.
			DisplayMainMemDump(hwndEdit);
			break;

		case WM_NOTIFY: 
			switch (((LPNMHDR) lParam)->code)
			{ 
				case TCN_SELCHANGE: 
				{
					RECT rc;
					
					// Create the edit control and initialize it.
					if (CreateMemoryDump(hwnd, &hwndEdit, &hwndButton) != S_OK)
						FATAL(hwnd, "Couldn't create the edit window.")

					// Display the memory that corresponds, on what tab is selected.
					switch (TabCtrl_GetCurSel(hwndMemoryTab))
					{
						case 0:
							// Dump the main memory to the control.
							DisplayMainMemDump(hwndEdit);
							break;
						
						case 1:
							// Dump the stack memory to the control.
							DisplayStackMemDump(hwndEdit);
							break;

						case 2:
							// Dump the PRG-ROM memory to the control.
							DisplayPRGROMMemDump(hwndEdit);
							break;

						case 3:
							// Dump the PPU memory to the control.
							DisplayPPUMemDump(hwndEdit);
							break;

						case 4:
							// Dump the Sprite memory to the control.
							DisplaySpriteMemDump(hwndEdit);
							break;

						default:
							break;
					}

					// Update the window by calling sending the size message.
					GetClientRect(hwnd, &rc);
					SendMessage(hwnd, WM_SIZE, SIZE_RESTORED, MAKELPARAM(rc.right-rc.left, rc.bottom-rc.top));
					break; 
				}

				case TCN_SELCHANGING:
					DestroyWindow(hwndEdit);
					DestroyWindow(hwndButton);
					break;
			} 
			break; 
	
        case WM_SIZE: 
			{ 
                HDWP hdwp; 
                RECT rc; 
 
                // Calculate the display rectangle, assuming the 
                // tab control is the size of the client area. 
                SetRect(&rc, 0, 0, LOWORD(lParam), HIWORD(lParam)); 
                TabCtrl_AdjustRect(hwndMemoryTab, FALSE, &rc); 
 
                // Size the tab control to fit the client area. 
                hdwp = BeginDeferWindowPos(3); 
                DeferWindowPos(hdwp, hwndMemoryTab, NULL, 0, 0, 
                    LOWORD(lParam), HIWORD(lParam), SWP_NOMOVE | SWP_NOZORDER); 
 
                // Position and size the button control to fit at
				// the bottom of the tab control window.
                DeferWindowPos(hdwp, hwndButton, HWND_TOP, rc.left,
					rc.bottom-rc.top, 70, 20, 0); 

				// Position and size the edit control to fit the 
                // tab control's display area minus some room for
				// the button, and make sure the edit control is 
				// in front of the tab control. 
                DeferWindowPos(hdwp, hwndEdit, HWND_TOP, rc.left, rc.top, 
                    rc.right-rc.left, rc.bottom-rc.top-30, 0); 
                EndDeferWindowPos(hdwp); 
			} 

		default:
			return DefMDIChildProc(hwnd, uMsg, wParam, lParam);
	}

	return 0;
} // end WndProcMemory()


//------------------------------------------------------------------------------
// Name: WndProcRun()
// Desc: Our message handling procedure for windows messages.
//------------------------------------------------------------------------------
LRESULT CALLBACK WndProcRun(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_DESTROY:
		case WM_CLOSE:
			// Tell the thread that runs the program to exit.
			bRunning = FALSE;
			break;

		case WM_KEYDOWN:
			switch (wParam)
			{
				case VK_ESCAPE:
					bRunning = FALSE;
					break;

				case 0x41: // A
					Joy1.SetButton(JOYSELECT);
					break;

				case 0x53: // S
					Joy1.SetButton(JOYSTART);
					break;

				case 0x5A: // Z
					Joy1.SetButton(JOYB);
					break;

				case 0x58: // X
					Joy1.SetButton(JOYA);
					break;

				case VK_UP:
					Joy1.SetButton(JOYUP);
					break;

				case VK_DOWN:
					Joy1.SetButton(JOYDOWN);
					break;

				case VK_LEFT:
					Joy1.SetButton(JOYLEFT);
					break;

				case VK_RIGHT:
					Joy1.SetButton(JOYRIGHT);
					break;
					
				case 0x31: // 1
					Options_bBackgroundEnabled ^= TRUE;
					break;

				case 0x32: // 2
					Options_bSpritesEnabled ^= TRUE;				
					break;

				default:
					break;
			}
			break;

		case WM_KEYUP:
			switch (wParam)
			{
				case 0x41: // A
					Joy1.ClearButton(JOYSELECT);
					break;

				case 0x53: // S
					Joy1.ClearButton(JOYSTART);
					break;

				case 0x5A: // Z
					Joy1.ClearButton(JOYB);
					break;

				case 0x58: // X
					Joy1.ClearButton(JOYA);
					break;

				case VK_UP:
					Joy1.ClearButton(JOYUP);
					break;

				case VK_DOWN:
					Joy1.ClearButton(JOYDOWN);
					break;

				case VK_LEFT:
					Joy1.ClearButton(JOYLEFT);
					break;

				case VK_RIGHT:
					Joy1.ClearButton(JOYRIGHT);
					break;
					
				default:
					break;
			}
			break;

		case WM_SIZE:
			UpdateBounds(hwnd);

		default:
			return DefMDIChildProc(hwnd, uMsg, wParam, lParam);
	}

	return 0;
} // end WndProcRun()


//------------------------------------------------------------------------------
// Name: AddBreakpoint()
// Desc: Adds a breakpoint to the array of breakpoints.
//------------------------------------------------------------------------------
HRESULT AddBreakpoint(WORD wAddress, UINT uListViewItem)
{
	// As long as we have room for another breakpoint add it to the array,
	// and check the list view check box.
	if (dwNumBreakpoints < OPTIONS_NUM_BREAKPOINTS)
	{
		awBreakpoints[dwNumBreakpoints++] = wAddress;
		ListView_Check(hwndCodeLV, uListViewItem);
	}
	else
		FATAL(hwndMain, "No more breakpoints allowed.");

	return S_OK;
} // end AddBreakpoint()


//------------------------------------------------------------------------------
// Name: AddRegistersToListView()
// Desc: Adds all the registers to the registers list view.
//------------------------------------------------------------------------------
HRESULT AddRegistersToListView()
{
	char strText[128]; // Text to hold the info to be added.

	// Add the A register.
	ADD_VAR_TO_REG_LISTVIEW(0, "A", CPU.A);
	// Add the X register.
	ADD_VAR_TO_REG_LISTVIEW(1, "X", CPU.X);
	// Add the Y register.
	ADD_VAR_TO_REG_LISTVIEW(2, "Y", CPU.Y);
	// Add the SP register.
	ADD_VAR_TO_REG_LISTVIEW(3, "SP", CPU.S);
	// Add the PC register.
	ADD_VAR_TO_REG_LISTVIEW(4, "PC", CPU.P);	
	// Add the flags register.
	ADD_VAR_TO_REG_LISTVIEW(5, "Flags", CPU.F);
	// Add a blank space in the list view.
	ADD_BLANK_TO_REG_LISTVIEW(6);

	// Add the Nintendo registers to the list view.
	ADD_VAR_TO_REG_LISTVIEW(7, "$2000", CPU.Memory[0x2000]);
	ADD_VAR_TO_REG_LISTVIEW(8, "$2001", CPU.Memory[0x2001]);
	ADD_VAR_TO_REG_LISTVIEW(9, "$2002", CPU.Memory[0x2002]);
	ADD_VAR_TO_REG_LISTVIEW(10, "$2003", CPU.Memory[0x2003]);
	ADD_VAR_TO_REG_LISTVIEW(11, "$2004", CPU.Memory[0x2004]);
	ADD_VAR_TO_REG_LISTVIEW(12, "$2005", CPU.Memory[0x2005]);
	ADD_VAR_TO_REG_LISTVIEW(13, "$2006", CPU.Memory[0x2006]);
	ADD_VAR_TO_REG_LISTVIEW(14, "$2007", CPU.Memory[0x2007]);
	ADD_VAR_TO_REG_LISTVIEW(15, "$4000", CPU.Memory[0x4000]);
	ADD_VAR_TO_REG_LISTVIEW(16, "$4001", CPU.Memory[0x4001]);
	ADD_VAR_TO_REG_LISTVIEW(17, "$4002", CPU.Memory[0x4002]);
	ADD_VAR_TO_REG_LISTVIEW(18, "$4003", CPU.Memory[0x4003]);
	ADD_VAR_TO_REG_LISTVIEW(19, "$4004", CPU.Memory[0x4004]);
	ADD_VAR_TO_REG_LISTVIEW(20, "$4005", CPU.Memory[0x4005]);
	ADD_VAR_TO_REG_LISTVIEW(21, "$4006", CPU.Memory[0x4006]);
	ADD_VAR_TO_REG_LISTVIEW(22, "$4007", CPU.Memory[0x4007]);
	ADD_VAR_TO_REG_LISTVIEW(23, "$4008", CPU.Memory[0x4008]);
	ADD_VAR_TO_REG_LISTVIEW(24, "$4009", CPU.Memory[0x4009]);
	ADD_VAR_TO_REG_LISTVIEW(25, "$4010", CPU.Memory[0x4010]);
	ADD_VAR_TO_REG_LISTVIEW(26, "$4011", CPU.Memory[0x4011]);
	ADD_VAR_TO_REG_LISTVIEW(17, "$4012", CPU.Memory[0x4012]);
	ADD_VAR_TO_REG_LISTVIEW(18, "$4013", CPU.Memory[0x4013]);
	ADD_VAR_TO_REG_LISTVIEW(19, "$4014", CPU.Memory[0x4014]);
	ADD_VAR_TO_REG_LISTVIEW(30, "$4015", CPU.Memory[0x4015]);
	ADD_VAR_TO_REG_LISTVIEW(31, "$4016", CPU.Memory[0x4016]);
	ADD_VAR_TO_REG_LISTVIEW(32, "$4017", CPU.Memory[0x4017]);

	// Add a blank space in the list view.
	ADD_BLANK_TO_REG_LISTVIEW(33);
	// Add the cycles to the list view.
	sprintf(strText, "Cycles");
	InsertListViewText(hwndRegsLV, 34, 0, strText);
	sprintf(strText, "%d", CPU.byCycles);
	InsertListViewText(hwndRegsLV, 34, 1, strText);
	// Add the scanlines to the list view.
	sprintf(strText, "Scanline");
	InsertListViewText(hwndRegsLV, 35, 0, strText);
	sprintf(strText, "%d", wScanline);
	InsertListViewText(hwndRegsLV, 35, 1, strText);
	// Add the VRAM address to the list view.
	ADD_VAR_TO_REG_LISTVIEW(36, "VRAM Address", wVRAMAddress);

	return S_OK;
} // end AddRegistersToListView()


//------------------------------------------------------------------------------
// Name: CleanUp()
// Desc: Deletes everthing that has been dynamically created.
//------------------------------------------------------------------------------
HRESULT CleanUp()
{
	// Delete our rom that we allocated.
	FreeNESFile();

	return S_OK;
} // end CleanUp()


//------------------------------------------------------------------------------
// Name: CreateMemoryDump()
// Desc: Creates an edit control to hold our memory contents and
//       a button for refreshing them.
//------------------------------------------------------------------------------
HRESULT CreateMemoryDump(HWND hwndParent, HWND* phwndEdit, HWND* phwndButton)
{
	// Create the edit control.
	*phwndEdit = CreateWindow("EDIT", NULL,
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER | ES_LEFT | ES_MULTILINE | ES_READONLY, 
		20, 20, 200, 200, hwndParent, NULL, g_hInstance, NULL);
	
	*phwndButton = CreateWindow( 
		"BUTTON",   // predefined class 
		"Refresh",       // button text 
		WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // styles 
		0,         // starting x position 
		0,         // starting y position 
		40,        // button width 
		20,        // button height 
		hwndParent, // parent window 
		NULL,       // No menu 
		g_hInstance, 
		NULL);      // pointer not needed 

	// Set the text to a fixed width font and the background color to white.
	SendMessage(*phwndEdit, WM_SETFONT, (WPARAM)GetStockObject(ANSI_FIXED_FONT), TRUE);
	SendMessage(*phwndButton, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);

	return S_OK;
} // CreateMemoryDump()


//------------------------------------------------------------------------------
// Name: CreateDebugWindow()
// Desc: Creates the window to hold our disassembly and register contents.
//       This is a MDI child window with a splitter to separate the registers
//       and the code disassembly.
//------------------------------------------------------------------------------
HRESULT CreateDebugWindow(HWND hwndParent)
{
	// Create the child window for the code and registers.
	hwndDebugWnd = CreateWindowEx(WS_EX_MDICHILD,
								  strDebugClassName,
								  strDebugTitleName,
								  WS_CHILD,
								  CW_USEDEFAULT,
								  CW_USEDEFAULT,
								  CW_USEDEFAULT,
								  CW_USEDEFAULT,
								  hwndMDIClient,
								  NULL,
								  g_hInstance,
								  NULL);
	if (hwndDebugWnd == NULL)
		FATAL(hwndParent, "Couldn't Create the debug window");

	ShowWindow(hwndDebugWnd, SW_SHOWMAXIMIZED);

	// Create the code list view box
	hwndCodeLV = CreateListView(hwndDebugWnd, IDS_CODE_COLUMN0, 4);
	if (hwndCodeLV == NULL)
		FATAL(hwndParent, "Couldn't create the code list view. Make sure IE3 is installed.");
	
	// Create the registers list view.
	hwndRegsLV = CreateListView(hwndDebugWnd, IDS_REGS_COLUMN0, 2);
	if (hwndCodeLV == NULL)
		FATAL(hwndParent, "Couldn't create the registers list view. Make sure IE3 is installed.");

	// Set the font to a fixed width font
	SendMessage(hwndCodeLV, WM_SETFONT, (WPARAM)GetStockObject(ANSI_FIXED_FONT), TRUE);
	SendMessage(hwndRegsLV, WM_SETFONT, (WPARAM)GetStockObject(ANSI_FIXED_FONT), TRUE);

	// Set the extended styles for the list views.
	ListView_SetExtendedListViewStyle(hwndCodeLV, 
		LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_CHECKBOXES);

	// Create and resize the code and registers list boxes
	// to fit the child window.
	ResizeDebugControls(hwndDebugWnd, hwndCodeLV, hwndRegsLV);

	// Add the registers to the registers list view.
	AddRegistersToListView();

	// Create the tooltip window for viewing our memory contents.
	//CreateTooltip(hwndCodeLV, "Hello!");
	//SendMessage(hwndCodeLV, LVM_SETTOOLTIPS, 0, (LPARAM)hwndTT);

	return S_OK;
} // end CreateDebugWindow()


//------------------------------------------------------------------------------
// Name: CreateListView()
// Desc: Creates a list view control, initializes the columns, and
//       returns the handle to the window if successful.
//------------------------------------------------------------------------------
HWND CreateListView(HWND hWndParent, UINT uFirstColRsc, UINT uNumColumns)
{
	HWND      hwndListView;
	LV_COLUMN lvC;
	char      strCol[50];

	// Create a list view control in report view.
	hwndListView = CreateWindowEx(0L, WC_LISTVIEW, "", WS_VISIBLE | WS_CHILD |
							  WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
							  0, 0, 0, 0, hWndParent, 
							  NULL, g_hInstance, NULL);
	if (hwndListView == NULL)
	  return NULL;

	// Create columns.
	lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvC.fmt = LVCFMT_LEFT;
	lvC.cx = 150;
	lvC.pszText = strCol;

	// Add the columns.
	for (UINT i = 0; i < uNumColumns; i++)
	{
		lvC.iSubItem = i;
		LoadString(g_hInstance, uFirstColRsc+i, strCol, sizeof(strCol));
		if (ListView_InsertColumn(hwndListView, i, &lvC) == -1)
			return NULL;
	}

	return hwndListView;
} // end CreateListView()


//------------------------------------------------------------------------------
// Name: CreateMDIClientWindow()
// Desc: Creates the client window for our frame window.
//------------------------------------------------------------------------------
HRESULT CreateMDIClientWindow(HWND hwndFrame)
{
	CLIENTCREATESTRUCT ccs; 

	// Retrieve the handle to the window menu and assign the 
	// first child window identifier. 
	ccs.hWindowMenu = GetSubMenu(GetMenu(hwndFrame), WINDOWMENU);
	ccs.idFirstChild = IDM_WINDOWCHILD;

	// Create the MDI client window.
	hwndMDIClient = CreateWindow( "MDICLIENT", (LPCTSTR)NULL,
		WS_CHILD | WS_CLIPCHILDREN | WS_VSCROLL | WS_HSCROLL,
		0, 0, 0, 0, hwndFrame, (HMENU)0xCAC, g_hInstance, (LPSTR)&ccs);

	ShowWindow(hwndMDIClient, SW_SHOW);

	return S_OK;
} // end CreateMDIClientWindow()


//------------------------------------------------------------------------------
// Name: CreateMemoryWindow()
// Desc: Creates the window hold our memory contents.
//------------------------------------------------------------------------------
HRESULT CreateMemoryWindow(HWND hwndParent)
{
	// Create the child window for the code and registers.
	hwndMemoryWnd = CreateWindowEx(WS_EX_MDICHILD,
								   strMemoryClassName,
								   strMemoryTitleName,
								   WS_CHILD,
								   CW_USEDEFAULT,
								   CW_USEDEFAULT,
								   CW_USEDEFAULT,
								   CW_USEDEFAULT,
								   hwndMDIClient,
								   NULL,
								   g_hInstance,
								   NULL);
	if (hwndMemoryWnd == NULL)
		FATAL(hwndParent, "Couldn't Create the debug window");

	return S_OK;
} // end CreateMemoryWindow()


//------------------------------------------------------------------------------
// Name: CreateTabControl()
// Desc: Creates a tab control, sized to fit the specified parent window's 
//       client area, and adds the number of tabs passed into the function. 
//       All the strings in the string table must be in order.
//------------------------------------------------------------------------------
HWND WINAPI CreateTabControl(HWND hwndParent, UINT uTextResource, UINT uNumTabs)
{ 
	RECT   rcClient;     // Client rectangle coordinates.
	HWND   hwndTab;      // Window handle to the tab control.
	TCITEM tie;          // Tab item struction
	char   strText[128]; // Text to hold the string of the tab

	// Get the dimensions of the parent window's client area, and 
	// create a tab control child window of that size. 
	GetClientRect(hwndParent, &rcClient); 
	hwndTab = CreateWindow(WC_TABCONTROL, "", 
		WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE, 
		0, 0, rcClient.right, rcClient.bottom, 
		hwndParent, NULL, g_hInstance, NULL); 
	if (hwndTab == NULL) 
		return NULL; 

	// Add the number of tabs passed to the function.
	tie.mask = TCIF_TEXT | TCIF_IMAGE; 
	tie.iImage = -1; 
	tie.pszText = strText; 

	for (UINT i = 0; i < uNumTabs; i++) 
	{ 
		LoadString(g_hInstance, uTextResource+i, strText, sizeof(strText)); 
		if (TabCtrl_InsertItem(hwndTab, i, &tie) == -1) 
		{ 
			DestroyWindow(hwndTab); 
			return NULL; 
		} 
	} 
	
	// Set the font of the control.
	SendMessage(hwndTab, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);

	return hwndTab; 
} // CreateTabControl()


//------------------------------------------------------------------------------
// Name: CreateTooltip()
// Desc: Creates a tooltip window adding the string as the text.
//------------------------------------------------------------------------------
HRESULT CreateTooltip(HWND hwnd, LPSTR strToolTip)
{
	TOOLINFO ti;      // struct specifying info about tool in tooltip control.
	UINT     uid = 0; // for ti initialization.
	RECT     rect;    // for client area coordinates.

	// Create the tooltip window.
	hwndTT = CreateWindowEx(WS_EX_TOPMOST,
		TOOLTIPS_CLASS,
		NULL,
		WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,		
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		hwnd,
		NULL,
		g_hInstance,
		NULL);

	// Set the window position.
	SetWindowPos(hwndTT,
		HWND_TOPMOST,
		0,
		0,
		0,
		0,
		SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	// Get coordinates of the main window.
	GetClientRect(hwnd, &rect);

	// Initialize the members fo the tool tip info structure.
	ti.cbSize = sizeof(TOOLINFO);
	ti.uFlags = TTF_SUBCLASS;
	ti.hwnd = hwnd;
	ti.hinst = g_hInstance;
	ti.uId = uid;
	ti.lpszText = strToolTip;
	
	// Tooltip control will cover the whole window
	ti.rect.left = rect.left;    
	ti.rect.top = rect.top;
	ti.rect.right = rect.right;
	ti.rect.bottom = rect.bottom;

	// Send a message to add the tooltip
	SendMessage(hwndTT, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);
	SendMessage(hwndTT, TTM_ACTIVATE, (WPARAM)TRUE, 0);

	return S_OK;
} // end CreateTooltip()


//------------------------------------------------------------------------------
// Name: DisableBreakpoint()
// Desc: Disables a breakpoint by removing it from the breakpoint array.
//------------------------------------------------------------------------------
HRESULT DisableBreakpoint(UINT uBreakpointIndex, UINT uListViewItem)
{
	// Remove the breakpoint from the array by setting it 0.
	awBreakpoints[uBreakpointIndex] = 0;

	// Copy all the old breakpoints into the old position.
	for (int i = uBreakpointIndex; i < OPTIONS_NUM_BREAKPOINTS-1; i++)
		awBreakpoints[i] = awBreakpoints[i+1];

	// Uncheck the breakpoint in the list view.
	ListView_UnCheck(hwndCodeLV, uListViewItem);

	// Finally decrement the number of breakpoints.
	dwNumBreakpoints--;

	return S_OK;
} // end DisableBreakpoint()


//------------------------------------------------------------------------------
// Name: DisplayMainMemDump()
// Desc: Dumps the main memory to a string and sets the text of an edit
//       control to that string.
//------------------------------------------------------------------------------
HRESULT DisplayMainMemDump(HWND hwndEditCtrl)
{
	char  strMemDump[0x1FFFF];       // String to hold the memory dump.
	char* pstrMemDump = strMemDump; // Pointer to the memory dump string.

	// Dump the first part of main memory to the string.
	sprintf(pstrMemDump, "Main Memory: %c%c%c%c%c%c", 13, 13, 10, 13, 13, 10);
	pstrMemDump += strlen(strMemDump);
	MemoryDumpToString(&pstrMemDump, &CPU.Memory[0], 0, 0x8000);

	// Set the text in the edit control.
	SendMessage(hwndEditCtrl, WM_SETTEXT, 0, (LPARAM) strMemDump);

	return S_OK;
} // DisplayMainMemDump()


//------------------------------------------------------------------------------
// Name: DisplayPPUMemDump()
// Desc: Dumps the PPU memory to a string and sets the text of an edit
//       control to that string.
//------------------------------------------------------------------------------
HRESULT DisplayPPUMemDump(HWND hwndEditCtrl)
{
	char  strMemDump[0x1FFFF];       // String to hold the memory dump.
	char* pstrMemDump = strMemDump; // Pointer to the memory dump string.

	// Dump the first bank of CHR-ROM to the string.
	sprintf(pstrMemDump, "CHR-ROM Bank 1: %c%c%c%c%c%c", 13, 13, 10, 13, 13, 10);
	pstrMemDump += 22;
	MemoryDumpToString(&pstrMemDump, &(*(PPU.apbyPatternTables[0])), 0, 0x1000);

	// Dump the second bank of CHR-ROM to the string.
	sprintf(pstrMemDump, "%c%c%c%c%c%cCHR-ROM Bank 2: %c%c%c%c%c%c", 
		13, 13, 10, 13, 13, 10, 13, 13, 10, 13, 13, 10);
	pstrMemDump += 28;
	MemoryDumpToString(&pstrMemDump, &(*(PPU.apbyPatternTables[1])), 0, 0x1000);

	// Dump the first name and attribute table to the string.
	sprintf(pstrMemDump, "%c%c%c%c%c%cName Table 1: %c%c%c%c%c%c", 
		13, 13, 10, 13, 13, 10, 13, 13, 10, 13, 13, 10);
	pstrMemDump += 26;
	MemoryDumpToString(&pstrMemDump, &(*(PPU.apbyNameTables[0])), 0, 0x400);

	// Dump the second name and attribute table to the string.
	sprintf(pstrMemDump, "%c%c%c%c%c%cName Table 2: %c%c%c%c%c%c", 
		13, 13, 10, 13, 13, 10, 13, 13, 10, 13, 13, 10);
	pstrMemDump += 26;
	MemoryDumpToString(&pstrMemDump, &(*(PPU.apbyNameTables[1])), 0, 0x400);

	// Dump the third name and attribute table to the string.
	sprintf(pstrMemDump, "%c%c%c%c%c%cName Table 3: %c%c%c%c%c%c", 
		13, 13, 10, 13, 13, 10, 13, 13, 10, 13, 13, 10);
	pstrMemDump += 26;
	MemoryDumpToString(&pstrMemDump, &(*(PPU.apbyNameTables[2])), 0, 0x400);

	// Dump the fourth name and attribute table to the string.
	sprintf(pstrMemDump, "%c%c%c%c%c%cName Table 4: %c%c%c%c%c%c", 
		13, 13, 10, 13, 13, 10, 13, 13, 10, 13, 13, 10);
	pstrMemDump += 26;
	MemoryDumpToString(&pstrMemDump, &(*(PPU.apbyNameTables[3])), 0, 0x400);

	// Dump the palettes to the string.
	sprintf(pstrMemDump, "%c%c%c%c%c%cPalettes: %c%c%c%c%c%c", 
		13, 13, 10, 13, 13, 10, 13, 13, 10, 13, 13, 10);
	pstrMemDump += 22;
	MemoryDumpToString(&pstrMemDump, &(PPU.abyPalettes[0]), 0, 0x20);

	// Set the text in the edit control.
	SendMessage(hwndEditCtrl, WM_SETTEXT, 0, (LPARAM) strMemDump);

	return S_OK;
} // DisplayPPUMemDump()


//------------------------------------------------------------------------------
// Name: DisplayPRGROMMemDump()
// Desc: Dumps the stack memory to a string and sets the text of an edit
//       control to that string.
//------------------------------------------------------------------------------
HRESULT DisplayPRGROMMemDump(HWND hwndEditCtrl)
{
	char  strMemDump[0x1FFFF];       // String to hold the memory dump.
	char* pstrMemDump = strMemDump; // Pointer to the memory dump string.

	// Dump the first bank of memory to the string.
	sprintf(pstrMemDump, "PRG-ROM Bank 1: %c%c%c%c%c%c", 13, 13, 10, 13, 13, 10);
	pstrMemDump += 22;
	MemoryDumpToString(&pstrMemDump, &(*CPU.pbyPRGROMBank1), 0, 0x4000);

	// Dump the second bank of memory to the string.
	sprintf(pstrMemDump, "%c%c%c%c%c%cPRG-ROM Bank 2: %c%c%c%c%c%c", 
		13, 13, 10, 13, 13, 10, 13, 13, 10, 13, 13, 10);
	pstrMemDump += 28;
	MemoryDumpToString(&pstrMemDump, &(*CPU.pbyPRGROMBank2), 0, 0x4000);

	// Set the text in the edit control.
	SendMessage(hwndEditCtrl, WM_SETTEXT, 0, (LPARAM) strMemDump);

	return S_OK;
} // DisplayPRGROMMemDump()


//------------------------------------------------------------------------------
// Name: DisplaySpriteMemDump()
// Desc: Dumps the sprite memory to a string and sets the text of an edit
//       control to that string.
//------------------------------------------------------------------------------
HRESULT DisplaySpriteMemDump(HWND hwndEditCtrl)
{
	char  strMemDump[0xFFFF];       // String to hold the memory dump.
	char* pstrMemDump = strMemDump; // Pointer to the memory dump string.

	// Dump the sprite memory to the string.
	sprintf(pstrMemDump, "Sprite Memory: %c%c%c%c%c%c", 13, 13, 10, 13, 13, 10);
	pstrMemDump += strlen(strMemDump);
	MemoryDumpToString(&pstrMemDump, abySPRRAM, 0, 0x100);

	// Set the text in the edit control.
	SendMessage(hwndEditCtrl, WM_SETTEXT, 0, (LPARAM) strMemDump);

	return S_OK;
} // DisplaySpriteMemDump()


//------------------------------------------------------------------------------
// Name: DisplayStackMemDump()
// Desc: Dumps the stack memory to a string and sets the text of an edit
//       control to that string.
//------------------------------------------------------------------------------
HRESULT DisplayStackMemDump(HWND hwndEditCtrl)
{
	char  strMemDump[0xFFFF];       // String to hold the memory dump.
	char* pstrMemDump = strMemDump; // Pointer to the memory dump string.

	// Dump the stack memory to the string.
	sprintf(pstrMemDump, "Stack Memory: %c%c%c%c%c%c", 13, 13, 10, 13, 13, 10);
	pstrMemDump += strlen(strMemDump);
	MemoryDumpToString(&pstrMemDump, &CPU.Memory[0x100], 0, 0x100);

	// Set the text in the edit control.
	SendMessage(hwndEditCtrl, WM_SETTEXT, 0, (LPARAM) strMemDump);

	return S_OK;
} // DisplayStackMemDump()


//------------------------------------------------------------------------------
// Name: DissassembleROM()
// Desc: Dissassembles the rom from the current PC position and displays
//       the results in the code dissassemble list box.
//------------------------------------------------------------------------------
HRESULT DissassembleROM()
{
	char strInstr[128]; // String to hold the instruction output.
	WORD wPC = CPU.P;   // Temporary PC register.
	BYTE byNumBytes;    // Number of bytes to add to the temp PC register.

	// Clear the list view.
	ListView_DeleteAllItems(hwndCodeLV);

	// Display all the instructions.
	for (int i = 0; i < OPTIONS_NUM_DEBUGLINES; i++)
	{
		// Reset the instruction string.
		memset(strInstr, '\0', 128);

		// Print the instruction to our string.
		byNumBytes = PrintInstrToString(strInstr, wPC);

		// Add the strings to the code list view.
		InsertListViewText(hwndCodeLV, i, 0, &strInstr[DISASM_PC_OFFSET]);
		InsertListViewText(hwndCodeLV, i, 1, &strInstr[DISASM_MACHINECODE_OFFSET]);
		InsertListViewText(hwndCodeLV, i, 2, &strInstr[DISASM_INSTRUCTION_OFFSET]);
		InsertListViewText(hwndCodeLV, i, 3, &strInstr[DISASM_MEMCONTENTS_OFFSET]);

		// TODO: Make sure to add the breakpoint stuff here.

		// Move to the next instruction.
		wPC += byNumBytes;
	}

	return S_OK;
} // end DissassembleROM()


//------------------------------------------------------------------------------
// Name: FreeNESFile()
// Desc: Unloads the ROM that has been loaded so a new one can be loaded.
//------------------------------------------------------------------------------
HRESULT FreeNESFile()
{
	// Free the mapper library.
	FreeLibrary(hinstMapperLib);

	// Delete our rom that we allocated.
	SAFE_DELETE_ARRAY(abyPRGROM);
	SAFE_DELETE_ARRAY(abyCHRROM);

	return S_OK;
} // end FreeNESFile()


//------------------------------------------------------------------------------
// Name: InitializeApp()
// Desc: Does any initialization that needs to be done before the
//       main windows loop begins.
//------------------------------------------------------------------------------
HRESULT InitializeApp()
{
	WNDCLASS wc;
	INITCOMMONCONTROLSEX icex;

	// Register the frame windows class.
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hbrBackground = (HBRUSH)GetStockObject(DKGRAY_BRUSH);
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
	wc.hInstance     = g_hInstance;
	wc.lpfnWndProc   = (WNDPROC)WndProcMain;
	wc.lpszClassName = strClassName;
	wc.lpszMenuName  = MAKEINTRESOURCE(IDR_MAINMENU);
	wc.style         = CS_HREDRAW | CS_VREDRAW;

	if (!RegisterClass(&wc))
		return ERROR;

	// Register the debug window class. 
    wc.hIcon         = LoadIcon(g_hInstance, IDI_APPLICATION); 
    wc.lpfnWndProc   = (WNDPROC)WndProcDebug; 
    wc.lpszClassName = strDebugClassName; 
    wc.lpszMenuName  = (LPSTR)NULL; 
 
    if (!RegisterClass(&wc)) 
        return ERROR;

	// Register the memory window class.
    wc.hIcon         = LoadIcon(g_hInstance, IDI_APPLICATION); 
    wc.lpfnWndProc   = (WNDPROC)WndProcMemory; 
    wc.lpszClassName = strMemoryClassName; 
    wc.lpszMenuName  = (LPSTR)NULL; 

    if (!RegisterClass(&wc)) 
        return ERROR;

	// Register the run window class.
    wc.hIcon         = LoadIcon(g_hInstance, IDI_APPLICATION); 
    wc.lpfnWndProc   = (WNDPROC)WndProcRun; 
    wc.lpszClassName = strRunClassName; 
    wc.lpszMenuName  = (LPSTR)NULL; 

    if (!RegisterClass(&wc)) 
        return ERROR;

	// Ensure that the common control DLL is loaded. 
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC  = ICC_LISTVIEW_CLASSES;
	InitCommonControlsEx(&icex); 

	// Initialize the global variables
	abyPRGROM = NULL;
	abyCHRROM = NULL;

	return S_OK;
} // InitializeApp()


//------------------------------------------------------------------------------
// Name: InsertListViewText()
// Desc: Inserts text into an item of a list view control.
//------------------------------------------------------------------------------
HRESULT InsertListViewText(HWND hwndLV, UINT uRow, UINT uCol, LPSTR strText)
{
	LV_ITEM lvI; // List view item structure.

	// Fill out the item structure.
	lvI.mask = LVIF_TEXT;
	lvI.iItem = uRow;
	lvI.iSubItem = uCol;
	lvI.pszText = strText;
	lvI.state = 0;
	lvI.stateMask = 0;
    
	// Add the item. First try adding it as an item. If that doesn't work,
	// then try adding the item as a subitem. If no good, return an error.
	if (ListView_InsertItem(hwndLV, &lvI) == -1)
		if (ListView_SetItem(hwndLV, &lvI) == FALSE)
			return ERROR;

	return S_OK;
} // end InsertListViewText()


//------------------------------------------------------------------------------
// Name: LoadNESFile()
// Desc: Called when the user selected Open from the File menu. Loads
//       the ROM into memory and sets the program up for debugging or
//       runs the game depending on the option set.
//------------------------------------------------------------------------------
HRESULT LoadNESFile(HWND hwnd)
{
	char strMapperLibName[512]; // FileName for the mapper .dll.

	// Popup the open file dialog box.
	PromptForFileOpen(hwnd);

	// Load the ROM into memory.
	LoadROMToMem((LPSTR)strFileName);

	// Save the number of program and character rom pages.
	wNumPRGROMPages = abyRomHeader[4];
	wNumCHRROMPages = abyRomHeader[5];

	// Some games only hold one (1) 16K bank of PRG-ROM, 
	// which should be loaded into both $C000 and $8000.
	// Otherwise initialize the banks to the first 2 pages.
	if (abyRomHeader[4] == 1)
	{
		CPU.pbyPRGROMBank1 = &abyPRGROM[0];
		CPU.pbyPRGROMBank2 = &abyPRGROM[0];
	}
	else
	{
		CPU.pbyPRGROMBank1 = &abyPRGROM[0];
		CPU.pbyPRGROMBank2 = &abyPRGROM[NES_PRGROM_PAGESIZE];
	}

	// Initialize the pattern tables to point to the first and
	// second banks of CHR-ROM.
	PPU.apbyPatternTables[0] = &abyCHRROM[0];
	PPU.apbyPatternTables[1] = &abyCHRROM[NES_CHRROM_PAGESIZE];

	// Initialize the pointers to the name tables depending on the
	// mirroring of the ROM. Bit 1 of the rom header offset 6
	// is 1 for vertical mirroring and 0 for horizontal mirroring.
	if (abyRomHeader[6] & 1)
	{
		// For vertical mirroring, name tables 0 and 2 point to
		// the first name table and name tables 1 and 3 point to
		// the second name table.
		PPU.apbyNameTables[0] = &PPU.abyNameTables[0];
		PPU.apbyNameTables[1] = &PPU.abyNameTables[0x400];
		PPU.apbyNameTables[2] = &PPU.abyNameTables[0];
		PPU.apbyNameTables[3] = &PPU.abyNameTables[0x400];
	}
	else
	{
		// For horizontal mirroring, name tables 0 and 1 point to
		// the first name table and name tables 2 and 3 point to
		// the second name table.
		PPU.apbyNameTables[0] = &PPU.abyNameTables[0];
		PPU.apbyNameTables[1] = &PPU.abyNameTables[0];
		PPU.apbyNameTables[2] = &PPU.abyNameTables[0x400];
		PPU.apbyNameTables[3] = &PPU.abyNameTables[0x400];
	}

	// Get the string for the file of the mapper .dll
	sprintf(strMapperLibName, ".\\mappers\\mapper%d.dll", (abyRomHeader[6]>>4)|(abyRomHeader[7]&0xF0));

	// Load the mapper .dll
	if ((hinstMapperLib = LoadLibrary(strMapperLibName)) == NULL)
		FATAL(hwnd, "Could not find the .dll for the mapper.");

	// Get all the function addresses and save them.
	if (hinstMapperLib != NULL)
	{
		MapperOnLoad = (MAPPERLOAD)GetProcAddress(hinstMapperLib, "?OnLoad@@YAHPAUtagNESData@@@Z");
		MapperOnRead = (MAPPERREAD)GetProcAddress(hinstMapperLib, "?OnRead@@YAEG@Z");
		MapperOnWrite = (MAPPERWRITE)GetProcAddress(hinstMapperLib, "?OnWrite@@YAHGE@Z");
	}

	// Make a call to the mapper to do anything that may have to 
	// happen during a load. This usually involves setting up the
	// pointers to PRG-ROM and CHR-ROM. Also use this as a time to
	// pass the pointers to our variables that are needed by the
	// memory mapper.

	NESDATA NesData;
	NesData.pCPU = &CPU;
	NesData.pPPU = &PPU;
	NesData.pabyPRGROM = abyPRGROM;
	NesData.wNumPRGROMPages = wNumPRGROMPages;
	NesData.pabyCHRROM = abyCHRROM;
	NesData.wNumCHRROMPages = wNumCHRROMPages;

	MapperOnLoad(&NesData);

	// Reset the NES.
	ResetNES();

	// Now if debug is set in the options then create the window 
	// to hold our disassembly and register contents disassemble
	// the rom starting at the current PC position.
	CreateDebugWindow(hwnd);

	// Dissassemble the beginning of the NES Rom.
	DissassembleROM();

	// Update the debugging information.
	UpdateDebugInfo();

	return S_OK;
} // end LoadNESFile()


//------------------------------------------------------------------------------
// Name: LoadROMToMem()
// Desc: Loads the ROM into memory for use. The .NES file is 
//       structure as follow (copied from Yoshi's nes doc.
//
//    +--------+------+------------------------------------------+
//    | Offset | Size | Content(s)                               |
//    +--------+------+------------------------------------------+
//    |   0    |  3   | 'NES'                                    |
//    |   3    |  1   | $1A                                      |
//    |   4    |  1   | 16K PRG-ROM page count                   |
//    |   5    |  1   | 8K CHR-ROM page count                    |
//    |   6    |  1   | ROM Control Byte #1                      |
//    |        |      |   %####vTsM                              |
//    |        |      |    |  ||||+- 0=Horizontal mirroring      |
//    |        |      |    |  ||||   1=Vertical mirroring        |
//    |        |      |    |  |||+-- 1=SRAM enabled              |
//    |        |      |    |  ||+--- 1=512-byte trainer present  |
//    |        |      |    |  |+---- 1=Four-screen mirroring     |
//    |        |      |    |  |                                  |
//    |        |      |    +--+----- Mapper # (lower 4-bits)     |
//    |   7    |  1   | ROM Control Byte #2                      |
//    |        |      |   %####0000                              |
//    |        |      |    |  |                                  |
//    |        |      |    +--+----- Mapper # (upper 4-bits)     |
//    |  8-15  |  8   | $00                                      |
//    | 16-..  |      | Actual 16K PRG-ROM pages (in linear      |
//    |  ...   |      | order). If a trainer exists, it precedes |
//    |  ...   |      | the first PRG-ROM page.                  |
//    | ..-EOF |      | CHR-ROM pages (in ascending order).      |
//    +--------+------+------------------------------------------+
//
//------------------------------------------------------------------------------
HRESULT LoadROMToMem(LPSTR strRomFileName)
{
	FILE* pRomFile = NULL;  // File pointer for NES Rom.

	// Open the .NES file for reading.
	if ((pRomFile = fopen(strRomFileName, "rb")) == NULL)
		FATAL(hwndMain, "Couldn't open nes file for reading.");

	// Read the rom's header.
	fread(abyRomHeader, 16, 1, pRomFile);

	// For now skip the trainer if it exists.
	if (abyRomHeader[6] & 0x04)
		fseek(pRomFile, 512, SEEK_CUR);

	// Allocate our memory for PRG-ROM and load the pages into memory.
	abyPRGROM = new BYTE[abyRomHeader[4]*NES_PRGROM_PAGESIZE];
	fread(abyPRGROM, NES_PRGROM_PAGESIZE, abyRomHeader[4], pRomFile);

	// Allocate our memory for CHR-ROM and load the pages into memory.
	abyCHRROM = new BYTE[abyRomHeader[5]*(NES_CHRROM_PAGESIZE*2)];
	fread(abyCHRROM, (NES_CHRROM_PAGESIZE*2), abyRomHeader[5], pRomFile);

	// Close the file.
	fclose(pRomFile);

	return S_OK;
} // end LoadROMToMem()


//------------------------------------------------------------------------------
// Name: MemoryDumpToString()
// Desc: Writes the memory dump of the passed variable in bytes. It places
//       the memory text in the string passed to the function.
//------------------------------------------------------------------------------
HRESULT MemoryDumpToString(char** pstrMemory, BYTE* pMemory, UINT uStartByte, 
						   UINT uEndByte)
{
	// Start at the beginning byte passed to this function and begin
	// disassembling. Every 16 bytes put the address at the left corner.
	for (UINT i = 0; i < uEndByte - uStartByte; i++)
	{
		// Display the current address.
		if ((i % 16) == 0)
		{
			if (i != 0)
			{
				sprintf(*pstrMemory, "%c%c%c", 13, 13, 10);
				*pstrMemory += 3;
			}

			sprintf(*pstrMemory, "%04X: ", i + uStartByte);
			*pstrMemory += 6;
		}

		// Print the memory byte and move on in the string.
		sprintf(*pstrMemory, "%02X ", pMemory[i+uStartByte]);
		*pstrMemory += 3;
	}

	return S_OK;
} // MemoryDumpToString()


//------------------------------------------------------------------------------
// Name: PromptForFileOpen()
// Desc: Prompts the user with the open file common dialog and 
//       returns the file name and path of the file the user selected.
//       If the user presses cancel the this function returns FALSE.
//------------------------------------------------------------------------------
HRESULT PromptForFileOpen(HWND hwndOwner)
{
	OPENFILENAME ofn;

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hwndOwner;
	ofn.hInstance = g_hInstance;
	ofn.lpstrFile = (LPSTR)strFileName;
	ofn.nMaxFile = 512;
	ofn.lpstrFilter = "NES Files (*.nes)\0*.nes;\0\0\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;

	// Prompt the user to select the file.
	if (GetOpenFileName(&ofn) == FALSE)
		return FALSE;

	// Save the filename string for the user.
	strcpy((LPSTR)strFileName, ofn.lpstrFile);

	return TRUE;
} // end PromptForFileOpen()


//------------------------------------------------------------------------------
// Name: ResetNES()
// Desc: Pretty self explainitory, resets the NES.
//------------------------------------------------------------------------------
HRESULT ResetNES()
{
	// Initialize the registers to their values.
	CPU.A = 0;
	CPU.X = 0;
	CPU.Y = 0;
	CPU.S = 0;
	CPU.F = 0;
	CPU.P = MAKEWORD(GetMemoryByte(0xFFFC), GetMemoryByte(0xFFFD));

	// Zero all the memory.
	ZeroMemory(CPU.Memory, 0x8000);

	// Reset the number of cpu cycles until the next scanline.
	CPU.byCycles = NUM_CYCLES_PER_SCANLINE;

	// Now display the updated information.
	UpdateDebugInfo();

	return S_OK;
} // end ResetNES()


//------------------------------------------------------------------------------
// Name: ResizeDebugControls()
// Desc: Resizes the code dissassembly list box as well as the 
//       registers list box to fit the child window.
//------------------------------------------------------------------------------
HRESULT ResizeDebugControls(HWND hwndChild, HWND hwndCodeLB, HWND hwndRegLB)
{
	RECT rcChild;    // RECT for the window the controls are in.

	// The the client area of the window these controls are to occupy
	GetClientRect(hwndChild, &rcChild);

	// Compute the coordinates and dimensions that the
	// list boxes will ocupy in the child window. We 
	// want them to fill the whole window.
	int nWidth = rcChild.right - rcChild.left;
	int nHeight = rcChild.bottom - rcChild.top;

	// Now move and resize the listboxes.
	MoveWindow(hwndCodeLB, 0, 0, nWidth-REGISTER_LISTBOX_WIDTH, nHeight, TRUE);
	MoveWindow(hwndRegLB, nWidth-REGISTER_LISTBOX_WIDTH, 0, 
		REGISTER_LISTBOX_WIDTH, nHeight, TRUE);

	return S_OK;
} // end ResizeDebugControls()


//------------------------------------------------------------------------------
// Name: CPU_DoCyclicTasks()
// Desc: Does all the tasks that must be done at the end of a scanline.
//------------------------------------------------------------------------------
HRESULT CPU_DoCyclicTasks()
{
	// If the cpu cycles are less than 0 then we must do
	// everything that must happen at the end of a scanline.
	if ((signed char)CPU.byCycles < 0)
	{
		// Add the number of cycles per scanline to the cycles.
		CPU.byCycles += NUM_CYCLES_PER_SCANLINE;
		// Increment which scanline are on.
		wScanline++;

		// Check for the sprite #0 hit.
		DoSprite0();

		// If we are passed the number of scanlines on the screen then
		// we are in vblank and we must update register $2002 and 
		// execute an NMI interrupt if bit 7 of $2000 is set.
		if (wScanline == NUM_SCANLINES_SCREEN)
		{
			// Set bit 7 of the PPU status register.
			CPU.Memory[0x2002] |= 0x80;

			// Do the NMI if we need to.
			if (CPU.Memory[0x2000] & 0x80)
			{
				// Push the PC onto the stack high byte first.
				PUSH_BYTE((BYTE)(CPU.P >> 8));
				PUSH_BYTE((BYTE)(CPU.P & 0xFF));
				
				// Push the flags register onto the stack.
				PUSH_BYTE(CPU.F);

				// Set the interrupt flag.
				CPU.F |= 0x04;

				// Set the PC equal to the address specified in the 
				// vector table for the NMI interrupt.
				CPU.P = MAKEWORD(GetMemoryByte(0xFFFA), GetMemoryByte(0xFFFB));

				// Subtract the interrupt latency from the CPU cycles.
				CPU.byCycles -= 7;

				// Break on the NMI interrupt.
				bBreak = TRUE;
			}
		}

		// If we are done with Vblank then we must update register
		// $2002 and reset the scanline counter.
		if (wScanline == NUM_SCANLINES_VBLANK+NUM_SCANLINES_SCREEN)
		{
			// Clear bit 7 and 6 of the PPU status register.
			CPU.Memory[0x2002] &= 0x3F;
			// Reset the scanline counter.
			wScanline = 0;
		}
	}

	return S_OK;
} // end CPU_DoCyclicTasks()


//------------------------------------------------------------------------------
// Name: CPU_Run()
// Desc: Runs the NES in run mode.
//------------------------------------------------------------------------------
DWORD WINAPI CPU_Run(LPVOID lpParam)
{
	DWORD dwBeginTime = 0; // The begining time of our frame.
	DWORD dwEndTime   = 0; // The ending time of our frame.
	DWORD dwTotalTime = 0; // The total time of our frame.
	DWORD dwFrames    = 0; // The number of frames drawn in a second.
	MSG   msg;             // Message structure.
	
	DWORD dwSoundFrameSkip = OPTIONS_NUM_AUDIOFRAMESKIP;
	DWORD dwGfxFrameSkip = OPTIONS_NUM_GFXFRAMESKIP;

	char strFrameCount[128] = {0};
	

	// Create the child window for running the program.
	hwndRunWnd = CreateWindowEx(WS_EX_MDICHILD,
							    strRunClassName,
								strRunTitleName,
								WS_CHILD,
								CW_USEDEFAULT,
								CW_USEDEFAULT,
								CW_USEDEFAULT,
								CW_USEDEFAULT,
								hwndMDIClient,
								NULL,
								g_hInstance,
								NULL);
	if (hwndRunWnd == NULL)
		FATAL(hwndMDIClient, "Couldn't Create the run window");

	// Create DirectDraw for the window.
	CreateDirectDraw(hwndRunWnd);

	// Create and initialize DirectSound.
	CreateSound(hwndRunWnd);

	// Keep running the program until the user has chosen to stop.
	while (bRunning == TRUE)
	{
		// If there is a message for this window, then we need
		// to process it. Otherwise, we are free to run then 
		// NES ROM until the user has chosen to stop. (bRunning == FALSE)
		if (PeekMessage(&msg, hwndRunWnd, 0, 0, PM_REMOVE))
		{
			DispatchMessage(&msg);
			TranslateMessage(&msg);
		}
		else
		{	
			// If we are back at the beginning of the screen
			// then clear the screen to the background color.
			if (wScanline == 0)
			{
				// Start our FPS counter.
				dwBeginTime = GetTickCount();

				// Initialize the surface to do our drawing.
				if (!dwGfxFrameSkip)
				{
					BeginDrawing();
				}

				// Draw all the sprites behind the background.
				if (!dwGfxFrameSkip)
				{
					if (Options_bSpritesEnabled)
						DrawSprites(SPRITE_BEHIND);
				}
			}

			// Single step the cpu.
			RunCPU(RUNCPU_RUN);

			// Do all the cyclic tasks.
			CPU_DoCyclicTasks();

			// Draw the scanline if its one of the scanlines on the screen.
			// i.e Don't draw the scanlines in VBlank.
			if (!dwGfxFrameSkip)
			{
				if ((wScanline < 240) && (CPU.Memory[0x2001] & 0x08) && Options_bBackgroundEnabled)
					DrawScanline();
			}

			// Flip the surfaces if we are done drawing the frame.
			if (wScanline == NUM_SCANLINES_SCREEN)
			{
				// Draw all the sprites in front of the background.
				if (!dwGfxFrameSkip)
				{
					if (Options_bSpritesEnabled)
						DrawSprites(SPRITE_INFRONT);
				}

				// Output the number of frames per second.
				OutputText(strFrameCount, 0, 0, RGB(255, 255, 255), 0);

				// Now that everything has been drawn.
				if (!dwGfxFrameSkip)
				{
					EndDrawing();
				}

				// Update our sound for the frame.
				if (!dwSoundFrameSkip)
					APU_DoFrame();

				if ((dwSoundFrameSkip--) == 0)
					dwSoundFrameSkip = OPTIONS_NUM_AUDIOFRAMESKIP;

				if ((dwGfxFrameSkip--) == 0)
					dwGfxFrameSkip = OPTIONS_NUM_GFXFRAMESKIP;
			}
			else if (wScanline == (NUM_SCANLINES_VBLANK+NUM_SCANLINES_SCREEN-1))
			{
				// Wait so we only go 60 FPS.
				//DWORD dwTemp = GetTickCount();
				//if ((dwTemp - dwBeginTime) < 16)
				//	Wait(16 - (dwTemp - dwBeginTime));

				// Calculate the FPS.
				dwFrames++;
				dwEndTime = GetTickCount();
				dwTotalTime += dwEndTime - dwBeginTime;

				// Display the FPS every 1 second.
				if (dwTotalTime >= 1000)
				{
					sprintf(strFrameCount, "Fps = %d", dwFrames);
					dwFrames = 0;
					dwTotalTime = 0;
				}
			}
		}
	}

	// Clean up DirectDraw, DirectSound and destroy the window.
	DestroyDirectDraw();
	DestroySound();
	DestroyWindow(hwndRunWnd);

	return 0;
} // end CPU_Run()


//------------------------------------------------------------------------------
// Name: CPU_RunUntilBreak()
// Desc: Runs the NES in debug mode until a breakpoint is encountered or
//       the user selects the break command.
//------------------------------------------------------------------------------
DWORD WINAPI CPU_RunUntilBreak(LPVOID lpParam)
{
	// As long as the user hasn't pressed break or we havn't
	// encountered a breakpoint, keep executing.
	while (bBreak == FALSE)
	{
		// Single step the cpu.
		RunCPU(RUNCPU_STEP);

		// Do all the cyclic tasks.
		CPU_DoCyclicTasks();

		// Check for breakpoints and tell the loop to break
		// if the PC is at a breakpoint.
		for (int i = 0; i < OPTIONS_NUM_BREAKPOINTS; i++)
		{
			if (CPU.P == awBreakpoints[i])
			{
				bBreak = TRUE;
				break;
			}
		}
	}

	// Update all the debugging information and reset the break command.
	UpdateDebugInfo();
	bBreak = FALSE;

	return 0;
} // end CPU_RunUntilBreak()


//------------------------------------------------------------------------------
// Name: CPU_Step()
// Desc: Single steps through instructions in debug mode.
//------------------------------------------------------------------------------
HRESULT CPU_Step()
{
	// Single step the cpu.
	RunCPU(RUNCPU_STEP);

	// Do all the cyclic tasks.
	CPU_DoCyclicTasks();

	// Update all the debugging information.
	UpdateDebugInfo();

	return S_OK;
} // end CPU_Step()


//------------------------------------------------------------------------------
// Name: SetListViewText()
// Desc: Sets the text of an item in a list view control.
//------------------------------------------------------------------------------
HRESULT SetListViewText(HWND hwndLV, UINT uRow, UINT uCol, LPSTR strText)
{
	LV_ITEM lvI; // List view item structure.

	// Fill out the item structure.
	lvI.mask = LVIF_TEXT;
	lvI.iItem = uRow;
	lvI.iSubItem = uCol;
	lvI.pszText = strText;
	lvI.state = 0;
	lvI.stateMask = 0;
    
	// Set the text of the item
	if (ListView_SetItem(hwndLV, &lvI) == FALSE)
		return ERROR;

	return S_OK;
} // end SetListViewText()


//------------------------------------------------------------------------------
// Name: ToggleBreakpoint()
// Desc: Turns a breakpoint on or off.
//------------------------------------------------------------------------------
HRESULT ToggleBreakpoint(UINT uItem)
{
	char strPC[5];      // Text to hold the PC address of the item.
	char strAddress[5]; // Text to hold the address of the breakpoint.

	// Get the address that the user has selected to add a breakpoint.
	ListView_GetItemText(hwndCodeLV, uItem, 0, strPC, 5);
	
	// Loop through and see if a breakpoint is already added on this line.
	for (int i = 0; i < OPTIONS_NUM_BREAKPOINTS; i++)
	{
		// Convert the breakpoint address to a string.
		sprintf(strAddress, "%04X", awBreakpoints[i]);

		// If the breakpoint is already on this line, then we
		// need to disable it.
		if (strcmp(strAddress, strPC) == 0)
			return DisableBreakpoint(i, uItem);
	}

	// If the breakpoint does not exist on this line, 
	// then we must add a breakpoint.
	return AddBreakpoint((WORD)strtoul(strPC, NULL, 16), uItem);
} // end ToggleBreakpoint()


//------------------------------------------------------------------------------
// Name: UpdateInstrCursorPos()
// Desc: Places the selection marker of the code list view on the
//       current instruction.
//------------------------------------------------------------------------------
HRESULT UpdateInstrCursorPos()
{
	LVFINDINFO lvfi;      // List view find item info.
	DWORD      dwItem;    // Current item to be selected.
	char       strPC[10]; // String of the PC's value.

	// Convert the PC's value to a hexadecimal string
	sprintf(strPC, "%04X:", CPU.P);

	// Initialize the structure to find our PC string.
	lvfi.flags = LVFI_STRING;
	lvfi.psz   = strPC;
	
	// Find the item in the list view.
	dwItem = ListView_FindItem(hwndCodeLV, -1, &lvfi);

	// If the item is not found then we need to dissassemble the rom
	// again because the program counter is out of range, then select
	// the first instruction.
	if (dwItem == -1)
	{
		DissassembleROM();
		dwItem = 0;
	}

	// Select the item.
	SetFocus(hwndCodeLV);
	ListView_SetItemState(hwndCodeLV, dwItem, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);

	return S_OK;
} // end UpdateInstrCursorPos()


//------------------------------------------------------------------------------
// Name: UpdateDebugInfo()
// Desc: Updates all the debugging information in the Registers List View.
//------------------------------------------------------------------------------
HRESULT UpdateDebugInfo()
{
	// Update all the registers.
	UpdateRegisters();

	// Select the current instruction line.
	UpdateInstrCursorPos();

	return S_OK;
} // end UpdateDebugInfo()


//------------------------------------------------------------------------------
// Name: UpdateRegisters()
// Desc: Updates all the register values in the register list view.
//------------------------------------------------------------------------------
HRESULT UpdateRegisters()
{
	char strText[128]; // Text to hold the info to be added.

	// Update the A register.
	UPDATE_VAR_IN_REG_LISTVIEW(0, "A", CPU.A);
	// Update the X register.
	UPDATE_VAR_IN_REG_LISTVIEW(1, "X", CPU.X);
	// Update the Y register.
	UPDATE_VAR_IN_REG_LISTVIEW(2, "Y", CPU.Y);
	// Update the SP register.
	UPDATE_VAR_IN_REG_LISTVIEW(3, "SP", CPU.S);
	// Update the PC register.
	UPDATE_VAR_IN_REG_LISTVIEW(4, "PC", CPU.P);	
	// Update the flags register.
	sprintf(strText, "Flags");
	SetListViewText(hwndRegsLV, 5, 0, strText);
	sprintf(&strText[0], "%c", (CPU.F & 0x80) ? 'S' : 's');
	sprintf(&strText[1], "%c", (CPU.F & 0x40) ? 'V' : 'v');
	sprintf(&strText[2], "%c", ' ');
	sprintf(&strText[3], "%c", (CPU.F & 0x10) ? 'B' : 'b');
	sprintf(&strText[4], "%c", (CPU.F & 0x08) ? 'D' : 'd');
	sprintf(&strText[5], "%c", (CPU.F & 0x04) ? 'I' : 'i');
	sprintf(&strText[6], "%c", (CPU.F & 0x02) ? 'Z' : 'z');
	sprintf(&strText[7], "%c", (CPU.F & 0x01) ? 'C' : 'c');
	SetListViewText(hwndRegsLV, 5, 1, strText);

	// Update the Nintendo registers.
	UPDATE_VAR_IN_REG_LISTVIEW(7, "$2000", CPU.Memory[0x2000]);
	UPDATE_VAR_IN_REG_LISTVIEW(8, "$2001", CPU.Memory[0x2001]);
	UPDATE_VAR_IN_REG_LISTVIEW(9, "$2002", CPU.Memory[0x2002]);
	UPDATE_VAR_IN_REG_LISTVIEW(10, "$2003", CPU.Memory[0x2003]);
	UPDATE_VAR_IN_REG_LISTVIEW(11, "$2004", CPU.Memory[0x2004]);
	UPDATE_VAR_IN_REG_LISTVIEW(12, "$2005", CPU.Memory[0x2005]);
	UPDATE_VAR_IN_REG_LISTVIEW(13, "$2006", CPU.Memory[0x2006]);
	UPDATE_VAR_IN_REG_LISTVIEW(14, "$2007", CPU.Memory[0x2007]);
	UPDATE_VAR_IN_REG_LISTVIEW(15, "$4000", CPU.Memory[0x4000]);
	UPDATE_VAR_IN_REG_LISTVIEW(16, "$4001", CPU.Memory[0x4001]);
	UPDATE_VAR_IN_REG_LISTVIEW(17, "$4002", CPU.Memory[0x4002]);
	UPDATE_VAR_IN_REG_LISTVIEW(18, "$4003", CPU.Memory[0x4003]);
	UPDATE_VAR_IN_REG_LISTVIEW(19, "$4004", CPU.Memory[0x4004]);
	UPDATE_VAR_IN_REG_LISTVIEW(20, "$4005", CPU.Memory[0x4005]);
	UPDATE_VAR_IN_REG_LISTVIEW(21, "$4006", CPU.Memory[0x4006]);
	UPDATE_VAR_IN_REG_LISTVIEW(22, "$4007", CPU.Memory[0x4007]);
	UPDATE_VAR_IN_REG_LISTVIEW(23, "$4008", CPU.Memory[0x4008]);
	UPDATE_VAR_IN_REG_LISTVIEW(24, "$4009", CPU.Memory[0x4009]);
	UPDATE_VAR_IN_REG_LISTVIEW(25, "$4010", CPU.Memory[0x4010]);
	UPDATE_VAR_IN_REG_LISTVIEW(26, "$4011", CPU.Memory[0x4011]);
	UPDATE_VAR_IN_REG_LISTVIEW(27, "$4012", CPU.Memory[0x4012]);
	UPDATE_VAR_IN_REG_LISTVIEW(28, "$4013", CPU.Memory[0x4013]);
	UPDATE_VAR_IN_REG_LISTVIEW(29, "$4014", CPU.Memory[0x4014]);
	UPDATE_VAR_IN_REG_LISTVIEW(30, "$4015", CPU.Memory[0x4015]);
	UPDATE_VAR_IN_REG_LISTVIEW(31, "$4016", CPU.Memory[0x4016]);
	UPDATE_VAR_IN_REG_LISTVIEW(32, "$4017", CPU.Memory[0x4017]);

	// Update the number of cpu cycles.
	sprintf(strText, "Cycles");
	SetListViewText(hwndRegsLV, 34, 0, strText);
	sprintf(strText, "%d", CPU.byCycles);
	SetListViewText(hwndRegsLV, 34, 1, strText);
	// Update the scanline number.
	sprintf(strText, "Scanline");
	SetListViewText(hwndRegsLV, 35, 0, strText);
	sprintf(strText, "%d", wScanline);
	SetListViewText(hwndRegsLV, 35, 1, strText);
	// Update the VRAM Address.
	UPDATE_VAR_IN_REG_LISTVIEW(36, "VRAM Address", wVRAMAddress);
	return S_OK;
} // end UpdateRegisters()


//------------------------------------------------------------------------------
// Name: Wait()
// Desc: Waits the number of milliseconds passed into the function.
//------------------------------------------------------------------------------
__inline VOID Wait(DWORD dwTime)
{
	DWORD dwBegin = GetTickCount();
	while ((GetTickCount() - dwBegin) <= dwTime);
} // end Wait()
