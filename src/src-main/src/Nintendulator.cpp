/* Furbtendulator
 * Win32 (and perhaps someday cross platform) NES emulator written in C++
 * (C) 2023-24 Furbland
 * 🙄
 *
 * Based on NintendulatorNRS - Win32 NES emulator written in C++
 * Copyright (C) 2017-2023 NewRisingSun
 *
 * Based on Nintendulator
 * Copyright (C) 2002-2023 QMT Productions
 *
 * Based on NinthStar, a portable Win32 NES Emulator written in C++
 * Copyright (C) 2000  David de Regt
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "stdafx.h"
#include "Nintendulator.h"
#include "Settings.h"
#include "resource.h"
#include "MapperInterface.h"
#include "NES.h"
#include "CPU.h"
#include "PPU.h"
#include "APU.h"
#include "GFX.h"
#include "Sound.h"
#include "AVI.h"
#include "Debugger.h"
#include "Movie.h"
#include "Controllers.h"
#include "States.h"
#include "HeaderEdit.h"
#include "Tape.h"
#include "DIPSwitch.h"
#include "PlugThruDevice.hpp"
#include "Cheats.hpp"
#include <shellapi.h>
#include "plugThruDevice_SuperMagicCard_transfer.hpp"
#include "plugThruDevice_DoctorPCJr.hpp"
#include "UM6578.h"

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "winmm.lib")

#define MAX_LOADSTRING 100

namespace PlugThruDevice {
namespace ProActionReplay {
extern	bool	invoke;
}
}

// Global Variables:
int		ConfigVersion;	// current version of configuration data
HINSTANCE	hInst;		// current instance
HWND		hMainWnd;		// main window
HMENU		hMenu;		// main window menu
HACCEL		hAccelTable;	// accelerators
std::set<HWND>	hWindows; // Handles for modeless windows
TCHAR		ProgPath[MAX_PATH];	// program path
TCHAR		DataPath[MAX_PATH];	// user data path
BOOL		MaskKeyboard = FALSE;	// mask keyboard accelerators (for when Family Basic Keyboard is active)
BOOL		MaskMouse = FALSE;	// hide mouse cursor (for Arkanoid paddle and SNES Mouse)
HWND		hDebug;		// Debug Info window
HCURSOR		CursorNormal, CursorCross, *CurrentCursor =&CursorNormal;
BOOL		CursorOnOutput;
BOOL		prev_multiCanSave =TRUE; // Will be set to FALSE by hard reset

TCHAR	szTitle[MAX_LOADSTRING];	// The title bar text
TCHAR	szWindowClass[MAX_LOADSTRING];	// The title bar text

// Foward declarations of functions included in this code module:
ATOM			MyRegisterClass(HINSTANCE hInstance);
BOOL			InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	DebugWnd(HWND, UINT, WPARAM, LPARAM);

TCHAR	TitlebarBuffer[256];
int	TitlebarDelay;

namespace NES {
extern DIP::Game dipswitchDefinition;
}

int APIENTRY	_tWinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	size_t i;
	MSG msg;

	CursorNormal =LoadCursor(NULL, IDC_ARROW);
	CursorCross  =LoadCursor(NULL, IDC_CROSS);
	srand(time(NULL));
	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDS_NINTENDULATOR, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	GetModuleFileName(NULL, ProgPath, MAX_PATH);
	for (i = _tcslen(ProgPath); (i > 0) && (ProgPath[i] != _T('\\')); i--)
		ProgPath[i] = 0;

	// find our folder in Application Data, if it exists
	if (!SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, DataPath)))
	{
		// if we can't even find that, then there's a much bigger problem...
		MessageBox(NULL, _T("FATAL: unable to locate Application Data folder"), _T("Nintendulator"), MB_OK | MB_ICONERROR);
		return FALSE;
	}
	PathAppend(DataPath, _T("Nintendulator"));
	if (GetFileAttributes(DataPath) == INVALID_FILE_ATTRIBUTES)
		CreateDirectory(DataPath, NULL);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
		return FALSE;

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_NINTENDULATOR));

	timeBeginPeriod(1);

	if (lpCmdLine[0])
	{
		// grab a copy of the command line parms - we're gonna need to extraextract a single filename from it
		TCHAR *cmdline = _tcsdup(lpCmdLine);
		// and remember the pointer so we can safely free it at the end
		TCHAR *bkptr = cmdline;

		// if the filename is in quotes, strip them off (and ignore everything past it)
		if (cmdline[0] == _T('"'))
		{
			cmdline++;
			// yes, it IS possible for the second quote to not exist!
			if (_tcschr(cmdline, '"'))
				*_tcschr(cmdline, '"') = 0;
		}
		// otherwise just kill everything past the first space
		else if (_tcschr(cmdline, ' '))
			*_tcschr(cmdline, ' ') = 0;

		NES::OpenFile(cmdline);
		/*TCHAR* Filename;
		GetFullPathName(cmdline, sizeof(Settings::Path_ROM), Settings::Path_ROM, &Filename);
		*Filename = 0;*/
		// Allocated using _tcsdup()
		free(bkptr);	// free up the memory from its original pointer
	}

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		// There are several places where the emulation thread can stop itself
		// and those need to unacquire the controllers from the main thread
		if (NES::DoStop & STOPMODE_BREAK)
		{
			NES::DoStop &= ~STOPMODE_BREAK;
			Controllers::UnAcquire();
		}

		// If WM_CLOSE came in during fullscreen emulation, shut down Graphics
		// here to revert to Windowed mode, then re-post WM_CLOSE and let the
		// program exit. Without this, we'd be destroying the window out from
		// underneath DirectDraw, which causes it to crash.
		if (NES::DoStop & STOPMODE_QUIT)
		{
			NES::DoStop &= ~STOPMODE_QUIT;
			GFX::Stop();
			PostMessage(hMainWnd, WM_CLOSE, 0, 0);
		}
		// The proper way to do this is to maintain a list of modeless dialog handles and check each of them here
		// or to keep one handle and have each modeless dialog set that handle whenever they get focus (KB 71450)
		// The problem is that mapper DLLs have no way of (un)registering any modeless dialogs they create
		// or notifying me that they've acquired focus and should be the ones passed to IsDialogMessage
		// Thus, this (probably incorrect) method is being used instead, based on the (currently) valid
		// assumption that all windows other than the main one (and modal dialogs) will be modeless dialogs
		// ProcessMessages() doesn't have this because it's only used in 2 places which don't matter
		// (namely, controller configuration and the brief delay when stopping emulation)
		HWND focus = GetActiveWindow();
		if ((focus != NULL) && (focus != hMainWnd) && IsDialogMessage(focus, &msg))
			continue;
		if (Controllers::scrollLock && MaskKeyboard || !TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	timeEndPeriod(1);
	DestroyCursor(CursorCross);
	DestroyCursor(CursorNormal);
	Tape::Stop();

	return (int)msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage is only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM	MyRegisterClass (HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style		= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon		= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_NINTENDULATOR));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_NINTENDULATOR);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HANDLE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL	InitInstance (HINSTANCE hInstance, int nCmdShow)
{
	GFX::DirectDraw = NULL;	// gotta do this so we don't paint from nothing
	hInst = hInstance;
	hMenu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_NINTENDULATOR));
	hMainWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, NULL, hMenu, hInstance, NULL);
	if (!hMainWnd)
		return FALSE;
	ShowWindow(hMainWnd, nCmdShow);
	DragAcceptFiles(hMainWnd, TRUE);

	hDebug = CreateDialog(hInst, MAKEINTRESOURCE(IDD_DEBUG), hMainWnd, DebugWnd);

	NES::Init();
	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK	WndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	int wmId, wmEvent;
	TCHAR FileName[MAX_PATH];
	OPENFILENAME ofn;
	BOOL running = NES::Running;
	
	// Disable pressing ALT to open the menu, so that the ALT key can effectively be mapped as a Controller Input key.
	// This does not affect pressing e.g. ALT+F to open the File menu.
	if (message ==WM_SYSKEYDOWN && wParam ==VK_MENU) return 0;
	
	// Doctor PC Jr. keyboard
	if (NES::currentPlugThruDevice ==ID_PLUG_DRPCJR && running) switch(message) {
		case WM_KEYDOWN:
		case WM_KEYUP:		
			PlugThruDevice::DoctorPCJr::windowsMessage(message, wParam);
			break;
	}
		
	// UM6578 keyboard and mouse
	if (RI.ConsoleType ==CONSOLE_UM6578 && running && APU::APU[0]) switch(message) {
		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_MOUSEMOVE:
			dynamic_cast<APU::APU_UM6578*>(APU::APU[0])->handleWindowMessage(message, wParam, lParam);
			break;
	}
	
	switch (message) {
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case ID_FILE_EXIT:
			SendMessage(hWnd, WM_CLOSE, 0, 0);
			break;
		case ID_FILE_OPEN:
			FileName[0] = 0;
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hMainWnd;
			ofn.hInstance = hInst;
			ofn.lpstrFilter =	_T("All supported files (*.NES, *.UNIF, *.UNF, *.FDS, *.FQD, *.NSF, *.WXN, *.BIN, *.FFE, *.SMC, *.RAW, *.A)\0")
							_T("*.NES;*.UNIF;*.UNF;*.FDS;*.NSF;*.WXN;*.BIN;*.RAW;*.FQD;*.FFE;*.SMC;*.studybox;*.A\0")
						_T("iNES ROM Images (*.NES)\0")
							_T("*.NES\0")
						_T("Universal NES Interchange Format ROM files (*.UNIF, *.UNF)\0")
							_T("*.UNF;*.UNIF\0")
						_T("Famicom Disk System Disk Images (*.FDS, *.BIN, *.RAW, *.FQD, *.A)\0")
							_T("*.FDS;*.BIN;*.RAW;*.FQD;*.A\0")
						_T("NES Sound Files (*.NSF)\0")
							_T("*.NSF\0")
						_T("Waixing Encrypted Files (*.WXN)\0")
							_T("*.WXN\0")
						_T("Front FarEast Super Magic Card Files (*.FFE, *.SMC)\0")
							_T("*.FFE;*.SMC\0")
						_T("OneBus Raw Binary Files (*.BIN)\0")
							_T("*.BIN\0")
						_T("StudyBox Tape Files (*.studybox)\0")
							_T("*.studybox\0")
						_T("\0");
			ofn.lpstrCustomFilter = NULL;
			ofn.nFilterIndex = 1;
			ofn.lpstrFile = FileName;
			ofn.nMaxFile = MAX_PATH;
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;
			ofn.lpstrInitialDir = Settings::Path_ROM;
			ofn.lpstrTitle = _T("Load ROM");
			ofn.Flags = OFN_FILEMUSTEXIST;
			ofn.lpstrDefExt = NULL;
			ofn.lCustData = 0;
			ofn.lpfnHook = NULL;
			ofn.lpTemplateName = NULL;
			if (GetOpenFileName(&ofn)) {
				_tcscpy(Settings::Path_ROM, FileName);
				Settings::Path_ROM[ofn.nFileOffset-1] = 0;
				NES::Stop();
				NES::OpenFile(FileName);
			}
			break;
		case ID_FILE_CLOSE:
			if (!NES::unload28()) break;
			NES::Stop();
			NES::CloseFile();
			break;
		case ID_CURRENT_HEADER:
			HeaderEdit::Open(NES::CurrentFilename);
			break;
		case ID_FILE_HEADER:
			FileName[0] = 0;
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hMainWnd;
			ofn.hInstance = hInst;
			ofn.lpstrFilter = _T("iNES ROM Images (*.NES)\0") _T("*.NES\0") _T("\0");
			ofn.lpstrCustomFilter = NULL;
			ofn.nFilterIndex = 1;
			ofn.lpstrFile = FileName;
			ofn.nMaxFile = MAX_PATH;
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;
			ofn.lpstrInitialDir = Settings::Path_ROM;
			ofn.lpstrTitle = _T("Edit Header");
			ofn.Flags = OFN_FILEMUSTEXIST;
			ofn.lpstrDefExt = NULL;
			ofn.lCustData = 0;
			ofn.lpfnHook = NULL;
			ofn.lpTemplateName = NULL;

			if (GetOpenFileName(&ofn)) {
				HeaderEdit::Open(FileName);
				_tcscpy(Settings::Path_ROM, FileName);
				Settings::Path_ROM[ofn.nFileOffset - 1] = 0;
			}
			break;
		case ID_FILE_AUTORUN:
			Settings::AutoRun =!Settings::AutoRun;
			Settings::ApplySettingsToMenu();
			break;
		case ID_FILE_AUTOCORRECT:
			Settings::AutoCorrect =!Settings::AutoCorrect;
			Settings::ApplySettingsToMenu();
			break;
		case ID_FILE_BROWSESAVES:
			BrowseFolder(DataPath);
			break;
		case ID_LOAD_MEMORY_DUMP:
			FileName[0] = 0;
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hMainWnd;
			ofn.hInstance = hInst;
			ofn.lpstrFilter = _T("Famicom Data Format (*.FDT)\0") _T("*.FDT\0") _T("\0");
			ofn.lpstrCustomFilter = NULL;
			ofn.nFilterIndex = 1;
			ofn.lpstrFile = FileName;
			ofn.nMaxFile = MAX_PATH;
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;
			ofn.lpstrInitialDir = Settings::Path_ROM;
			ofn.lpstrTitle = _T("Load Memory Dump");
			ofn.Flags = OFN_FILEMUSTEXIST;
			ofn.lpstrDefExt = NULL;
			ofn.lCustData = 0;
			ofn.lpfnHook = NULL;
			ofn.lpTemplateName = NULL;
			if (GetOpenFileName(&ofn)) {
				NES::loadMemoryDump(FileName);
				_tcscpy(Settings::Path_ROM, FileName);
				Settings::Path_ROM[ofn.nFileOffset - 1] = 0;
			}
			break;
		case ID_SAVE_MEMORY_DUMP:
			FileName[0] = 0;
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hMainWnd;
			ofn.hInstance = hInst;
			ofn.lpstrFilter = _T("Famicom Data Format (*.FDT)\0") _T("*.FDT\0") _T("\0");
			ofn.lpstrCustomFilter = NULL;
			ofn.nFilterIndex = 1;
			ofn.lpstrFile = FileName;
			ofn.nMaxFile = MAX_PATH;
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;
			ofn.lpstrInitialDir = Settings::Path_ROM;
			ofn.lpstrTitle = _T("Save Memory Dump");
			ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;
			ofn.lpstrDefExt = _T("fdt");
			ofn.lCustData = 0;
			ofn.lpfnHook = NULL;
			ofn.lpTemplateName = NULL;
			if (GetSaveFileName(&ofn)) {
				NES::saveMemoryDump(FileName);
				_tcscpy(Settings::Path_ROM, FileName);
				Settings::Path_ROM[ofn.nFileOffset - 1] = 0;
			}
			break;
		case ID_CPU_RUN:
			NES::Start(FALSE);
			break;
		case ID_CPU_STEP:
			NES::Stop();		// need to stop first
			NES::Start(TRUE);	// so the 'start' makes it through
			break;
		case ID_CPU_STOP:
			NES::Stop();
			break;
		case ID_CPU_SOFTRESET:
			NES::Pause(FALSE);
			if (Movie::Mode)
				Movie::Stop();
			NES::Reset(RESET_SOFT);
			if (running)
				NES::Resume();
			break;
		case ID_CPU_HARDRESET:
			NES::Pause(FALSE);
			if (Movie::Mode) Movie::Stop();
			NES::Reset(RESET_HARD);
			if (running) NES::Resume();
			break;
		case ID_CPU_SAVESTATE:
			if (running)
				NES::Pause(TRUE);
			else	NES::SkipToVBlank();
			States::SaveNumberedState();
			if (running)	NES::Resume();
			break;
		case ID_CPU_LOADSTATE:
			NES::Pause(FALSE);
			States::LoadNumberedState();
			if (running)	NES::Resume();
#ifdef	ENABLE_DEBUGGER
			else if (Debugger::Enabled)
				Debugger::Update(Debugger::Mode);
#endif	/* ENABLE_DEBUGGER */
			break;
		case ID_CPU_SAVESTATE_NAMED:
			if (running)
				NES::Pause(TRUE);
			else	NES::SkipToVBlank();
			States::SaveNamedState();
			if (running)	NES::Resume();
			break;
		case ID_CPU_LOADSTATE_NAMED:
			NES::Pause(FALSE);
			States::LoadNamedState();
			if (running)	NES::Resume();
#ifdef	ENABLE_DEBUGGER
			else if (Debugger::Enabled)
				Debugger::Update(Debugger::Mode);
#endif	/* ENABLE_DEBUGGER */
			break;
		case ID_CPU_PREVSTATE:
			States::SelSlot += 9;
			States::SelSlot %= 10;
			States::SetSlot(States::SelSlot);
			break;
		case ID_CPU_NEXTSTATE:
			States::SelSlot += 1;
			States::SelSlot %= 10;
			States::SetSlot(States::SelSlot);
			break;
		case ID_CPU_BOOT_DISABLE_FRAMEIRQ:
			Settings::BootWithDisabledFrameIRQ =!Settings::BootWithDisabledFrameIRQ;
			Settings::ApplySettingsToMenu();
			break;
		case ID_IGNORE_RACE_CONDITION:
			Settings::IgnoreRaceCondition =!Settings::IgnoreRaceCondition;
			Settings::ApplySettingsToMenu();
			break;
		case ID_CPU_FRAMESTEP_ENABLED:
			NES::FrameStep = !NES::FrameStep;
			if (NES::FrameStep)
				CheckMenuItem(hMenu, ID_CPU_FRAMESTEP_ENABLED, MF_CHECKED);
			else	CheckMenuItem(hMenu, ID_CPU_FRAMESTEP_ENABLED, MF_UNCHECKED);
			break;
		case ID_CPU_FRAMESTEP_STEP:
			NES::GotStep = TRUE;
			break;
		case ID_RAMFILL_00:
			Settings::RAMInitialization =0x00;
			Settings::ApplySettingsToMenu();
			break;
		case ID_RAMFILL_FF:
			Settings::RAMInitialization =0xFF;
			Settings::ApplySettingsToMenu();
			break;
		case ID_RAMFILL_RANDOM:
			Settings::RAMInitialization =0x01;
			Settings::ApplySettingsToMenu();
			break;
		case ID_PPU_FRAMESKIP_AUTO:
			Settings::aFSkip = !Settings::aFSkip;
			GFX::SetFrameskip(-1);
			Settings::ApplySettingsToMenu();
			break;
		case ID_PPU_FRAMESKIP_0:
			GFX::SetFrameskip(0);
			break;
		case ID_PPU_FRAMESKIP_1:
			GFX::SetFrameskip(1);
			break;
		case ID_PPU_FRAMESKIP_2:
			GFX::SetFrameskip(2);
			break;
		case ID_PPU_FRAMESKIP_3:
			GFX::SetFrameskip(3);
			break;
		case ID_PPU_FRAMESKIP_4:
			GFX::SetFrameskip(4);
			break;
		case ID_PPU_FRAMESKIP_5:
			GFX::SetFrameskip(5);
			break;
		case ID_PPU_FRAMESKIP_6:
			GFX::SetFrameskip(6);
			break;
		case ID_PPU_FRAMESKIP_7:
			GFX::SetFrameskip(7);
			break;
		case ID_PPU_FRAMESKIP_8:
			GFX::SetFrameskip(8);
			break;
		case ID_PPU_FRAMESKIP_9:
			GFX::SetFrameskip(9);
			break;
		case ID_PPU_SIZE_1X:
			Settings::SizeMult =1;
			UpdateInterface();
			Settings::ApplySettingsToMenu();
			break;
		case ID_PPU_SIZE_2X:
			Settings::SizeMult =2;
			UpdateInterface();
			Settings::ApplySettingsToMenu();
			break;
		case ID_PPU_SIZE_3X:
			Settings::SizeMult =3;
			UpdateInterface();
			Settings::ApplySettingsToMenu();
			break;
		case ID_PPU_SIZE_4X:
			Settings::SizeMult =4;
			UpdateInterface();
			Settings::ApplySettingsToMenu();
			break;
		case ID_PPU_SIZE_ASPECT:
			Settings::FixAspect =!Settings::FixAspect;
			UpdateInterface();
			Settings::ApplySettingsToMenu();
			break;
		case ID_PPU_MODE_NTSC:
			NES::Stop();
			NES::SetRegion(Settings::REGION_NTSC);
			if (running) NES::Start(FALSE);
			break;
		case ID_PPU_MODE_PAL:
			NES::Stop();
			NES::SetRegion(Settings::REGION_PAL);
			if (running) NES::Start(FALSE);
			break;
		case ID_PPU_MODE_DENDY:
			NES::Stop();
			NES::SetRegion(Settings::REGION_DENDY);
			if (running) NES::Start(FALSE);
			break;
		case ID_DEFAULT_NTSC:
			Settings::DefaultRegion =Settings::REGION_NTSC;
			Settings::ApplySettingsToMenu();
			break;
		case ID_DEFAULT_PAL:
			Settings::DefaultRegion =Settings::REGION_PAL;
			Settings::ApplySettingsToMenu();
			break;
		case ID_DEFAULT_DENDY:
			Settings::DefaultRegion =Settings::REGION_DENDY;
			Settings::ApplySettingsToMenu();
			break;
		case ID_PPU_PALETTE:
			GFX::PaletteConfig();
			break;
		case ID_PPU_SLOWDOWN_ENABLED:
			GFX::SlowDown =!GFX::SlowDown;
			if (GFX::SlowDown)
				CheckMenuItem(hMenu, ID_PPU_SLOWDOWN_ENABLED, MF_CHECKED);
			else	CheckMenuItem(hMenu, ID_PPU_SLOWDOWN_ENABLED, MF_UNCHECKED);
			break;
		case ID_PPU_SLOWDOWN_2:
			GFX::SlowRate = 2;
			CheckMenuRadioItem(hMenu, ID_PPU_SLOWDOWN_2, ID_PPU_SLOWDOWN_20, ID_PPU_SLOWDOWN_2, MF_BYCOMMAND);
			break;
		case ID_PPU_SLOWDOWN_3:
			GFX::SlowRate = 3;
			CheckMenuRadioItem(hMenu, ID_PPU_SLOWDOWN_2, ID_PPU_SLOWDOWN_20, ID_PPU_SLOWDOWN_3, MF_BYCOMMAND);
			break;
		case ID_PPU_SLOWDOWN_4:
			GFX::SlowRate = 4;
			CheckMenuRadioItem(hMenu, ID_PPU_SLOWDOWN_2, ID_PPU_SLOWDOWN_20, ID_PPU_SLOWDOWN_4, MF_BYCOMMAND);
			break;
		case ID_PPU_SLOWDOWN_5:
			GFX::SlowRate = 5;
			CheckMenuRadioItem(hMenu, ID_PPU_SLOWDOWN_2, ID_PPU_SLOWDOWN_20, ID_PPU_SLOWDOWN_5, MF_BYCOMMAND);
			break;
		case ID_PPU_SLOWDOWN_10:
			GFX::SlowRate = 10;
			CheckMenuRadioItem(hMenu, ID_PPU_SLOWDOWN_2, ID_PPU_SLOWDOWN_20, ID_PPU_SLOWDOWN_10, MF_BYCOMMAND);
			break;
		case ID_PPU_SLOWDOWN_20:
			GFX::SlowRate = 20;
			CheckMenuRadioItem(hMenu, ID_PPU_SLOWDOWN_2, ID_PPU_SLOWDOWN_20, ID_PPU_SLOWDOWN_20, MF_BYCOMMAND);
			break;
		case ID_PPU_FULLSCREEN:
			//if (!GFX::Fullscreen && Settings::VSDualScreens !=0) break;
			NES::Stop();
			GFX::Stop();

			Settings::GetWindowPosition();

			GFX::Fullscreen =!GFX::Fullscreen;
			GFX::Start();
			Settings::SetWindowPosition();

			if (running) NES::Start(FALSE);
			break;
		case ID_VT03_NTSC:
			Settings::VT03Palette =0;
			Settings::ApplySettingsToMenu();
			break;
		case ID_VT03_PAL:
			Settings::VT03Palette =1;
			Settings::ApplySettingsToMenu();
			break;
		case ID_VT03_MIXED:
			Settings::VT03Palette =2;
			Settings::ApplySettingsToMenu();
			break;
		case ID_VT32_REMOVE_BLUE_CAST:
			Settings::VT32RemoveBlueishCast =!Settings::VT32RemoveBlueishCast;
			Settings::ApplySettingsToMenu();
			break;
		case ID_PPU_SOFT_RESET:
			Settings::PPUSoftReset =!Settings::PPUSoftReset;
			Settings::ApplySettingsToMenu();
			break;
		case ID_PPU_NEVER_CLIP:
			Settings::PPUNeverClip =!Settings::PPUNeverClip;
			Settings::ApplySettingsToMenu();
			break;
		case ID_DENDY_NTSC_ASPECT:
			Settings::DendyNTSCAspect =!Settings::DendyNTSCAspect;
			Settings::ApplySettingsToMenu();
			if (NES::CurRegion ==Settings::REGION_DENDY) {
				NES::Stop();
				GFX::Stop();

				UpdateInterface();

				GFX::Start();
				if (running) NES::Start(FALSE);
			}
			break;
		case ID_PPU_DENDY_SWAP_EMPHASIS:
			Settings::DendySwapEmphasis =!Settings::DendySwapEmphasis;
			Settings::ApplySettingsToMenu();
			break;
		case ID_DENDY_60_HZ:
			Settings::Dendy60Hz =!Settings::Dendy60Hz;
			Settings::ApplySettingsToMenu();
			if (NES::CurRegion ==Settings::REGION_DENDY) {

				NES::Stop();
				GFX::Stop();
				NES::SetRegion(NES::CurRegion);

				GFX::Start();
				if (running) NES::Start(FALSE);
			}
			break;
		case ID_PPU_VSYNC:
			Settings::VSync =!Settings::VSync;
			Settings::ApplySettingsToMenu();
			break;
		case ID_PPU_NO_SPRITE_LIMIT:
			Settings::NoSpriteLimit =!Settings::NoSpriteLimit;
			Settings::ApplySettingsToMenu();
			break;
		case ID_PPU_SCROLL_GLITCH:
			Settings::ScrollGlitch =!Settings::ScrollGlitch;
			Settings::ApplySettingsToMenu();
			break;
		case ID_PPU_UP_BY_ONE:
			//if (MessageBox(hMainWnd, _T("Changing this setting causes a reset. Continue?"), _T("Y position up by one"), MB_YESNO) !=IDYES) break;
			Settings::upByOne =!Settings::upByOne;
			Settings::ApplySettingsToMenu();
			/*NES::Pause(FALSE);
			if (Movie::Mode) Movie::Stop();
			NES::Reset(RESET_HARD);
			if (running) NES::Resume();*/
			break;
		case ID_PPU_APERTURE:
			GFX::apertureDialog();
			break;
		case ID_PPU_SCANLINES:
			NES::Stop();
			GFX::Stop();
			Settings::Scanlines =!Settings::Scanlines;
			GFX::Start();
			Settings::ApplySettingsToMenu();
			if (running) NES::Start(FALSE);
			break;
		case ID_PPU_DISABLE_EMPHASIS:
			Settings::DisableEmphasis =!Settings::DisableEmphasis;
			PPU::SetRegion(); // Causes color emphasis to be reevaluated
			Settings::ApplySettingsToMenu();
			break;
		case ID_PPU_SHOW_BG:
			PPU::showBG =!PPU::showBG;
			Settings::ApplySettingsToMenu();
			break;
		case ID_PPU_SHOW_OBJ:
			PPU::showOBJ =!PPU::showOBJ;
			Settings::ApplySettingsToMenu();
			break;
		case ID_PPU_DISABLE_OAMDATA:
			Settings::DisableOAMData =!Settings::DisableOAMData;
			Settings::ApplySettingsToMenu();
			break;
		case ID_SOUND_ENABLED:
			Settings::SoundEnabled =!Settings::SoundEnabled;
			if (Settings::SoundEnabled) {
				if (running) Sound::SoundON();
			} else {
				if (running) Sound::SoundOFF();
			}
			Settings::ApplySettingsToMenu();
			break;
		case ID_SOUND_LPF_NONE:
			Settings::LowPassFilterAPU =0;
			Settings::ApplySettingsToMenu();
			break;
		case ID_SOUND_LPF_PULSE:
			Settings::LowPassFilterAPU =1;
			Settings::ApplySettingsToMenu();
			break;
		case ID_SOUND_LPF_APU:
			Settings::LowPassFilterAPU =2;
			Settings::ApplySettingsToMenu();
			break;
		case ID_SOUND_LPF_RF:
			Settings::LowPassFilterAPU =3;
			Settings::ApplySettingsToMenu();
			break;
		case ID_SOUND_LPF_ONEBUS:
			Settings::LowPassFilterOneBus =!Settings::LowPassFilterOneBus;
			Settings::ApplySettingsToMenu();
			break;
		case ID_SOUND_SWAPDUTY:
			Settings::SwapDutyCycles =!Settings::SwapDutyCycles;
			Settings::ApplySettingsToMenu();
			break;
		case ID_SOUND_EXPAUDIO_BOOTLEG:
			Settings::BootlegExpansionAudio =!Settings::BootlegExpansionAudio;
			Settings::ApplySettingsToMenu();
			break;
		case ID_SOUND_REMOVE_BUZZ:
			Settings::RemoveDPCMBuzz =!Settings::RemoveDPCMBuzz;
			Settings::ApplySettingsToMenu();
			break;
		case ID_SOUND_REMOVE_POPS:
			Settings::RemoveDPCMPops =!Settings::RemoveDPCMPops;
			Settings::ApplySettingsToMenu();
			break;
		case ID_REVERSE_DPCM:
			Settings::ReverseDPCM =!Settings::ReverseDPCM;
			Settings::ApplySettingsToMenu();
			break;
		case ID_DMC_JOYPAD_CORRUPTION:
			Settings::DMCJoypadCorruption =!Settings::DMCJoypadCorruption;
			Settings::ApplySettingsToMenu();
			break;
		case ID_DISABLE_PERIODIC_NOISE:
			Settings::DisablePeriodicNoise =!Settings::DisablePeriodicNoise;
			Settings::ApplySettingsToMenu();
			break;
		case ID_SOUND_CLEAN_N163:
			Settings::CleanN163 =!Settings::CleanN163;
			Settings::ApplySettingsToMenu();
			break;
		case ID_NONLINEAR_MIXING:
			Settings::NonlinearMixing =!Settings::NonlinearMixing;
			Settings::ApplySettingsToMenu();
			break;
		case ID_EXPAUDIO_NONE:
			Settings::ExpansionAudioVolume =0;
			Settings::ApplySettingsToMenu();
			break;
		case ID_EXPAUDIO_ORIG:
			Settings::ExpansionAudioVolume =75;
			Settings::ApplySettingsToMenu();
			break;
		case ID_EXPAUDIO_MIDDLE:
			Settings::ExpansionAudioVolume =80;
			Settings::ApplySettingsToMenu();
			break;
		case ID_EXPAUDIO_LATER:
			Settings::ExpansionAudioVolume =133;
			Settings::ApplySettingsToMenu();
			break;
		case ID_INPUT_SETUP:
			NES::Stop();
			Controllers::OpenConfig();
			if (running) NES::Start(FALSE);
			break;
#ifdef	ENABLE_DEBUGGER
		case ID_DEBUG_CPU:
			Debugger::SetMode(Debugger::Mode ^ DEBUG_MODE_CPU);
			break;
		case ID_DEBUG_PPU:
			Debugger::SetMode(Debugger::Mode ^ DEBUG_MODE_PPU);
			break;
#endif	/* ENABLE_DEBUGGER */
		case ID_DEBUG_STATWND:
			Settings::dbgVisible = !Settings::dbgVisible;
			Settings::ApplySettingsToMenu();
			ShowWindow(hDebug, Settings::dbgVisible ? SW_SHOW : SW_HIDE);
			break;
		case ID_GAME:
			NES::MapperConfig();
			break;
		case ID_CHEATS:
			Cheats::cheatsWindow(hMainWnd);
			break;
		case ID_GAME_DIPSWITCHES:
			if (DIP::windowDIP(hMainWnd, NES::dipswitchDefinition, RI.dipValue)) {
				NES::Pause(FALSE);
				if (Movie::Mode) Movie::Stop();
				NES::Reset(RESET_SOFT);
				if (running) NES::Resume();
			}
			break;
		case ID_GAME_INSERT_COIN1:
			if (NES::coinDelay1 ==0) {	// Coin slot is blocked for a while
				NES::coin1 =0x20;
				NES::coinDelay1 =222222;
			}
			break;
		case ID_GAME_INSERT_COIN2:
			if (NES::coinDelay2 ==0) {	// Coin slot is blocked for a while
				NES::coin2 =0x20;
				NES::coinDelay2 =222222;
			}
			break;
		case ID_INPUT_MICROPHONE:
			NES::micDelay =222222;
			break;
		case ID_MISC_STARTAVICAPTURE:
			AVI::Start();
			break;
		case ID_MISC_STOPAVICAPTURE:
			AVI::End();
			break;
		case ID_MISC_PLAYMOVIE:
			Movie::Play();
			break;
		case ID_MISC_RECORDMOVIE:
			Movie::Record();
			break;
		case ID_MISC_STOPMOVIE:
			Movie::Stop();
			break;
		case ID_MISC_PLAYTAPE:
			Tape::Play();
			break;
		case ID_MISC_RECORDTAPE:
			Tape::Record();
			break;
		case ID_MISC_STOPTAPE:
			Tape::Stop();
			break;
		case ID_HELP_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case ID_MISC_SCREENSHOT:
			if (running)
				GFX::ScreenshotRequested =TRUE;
			else
				GFX::SaveScreenshot();
			break;
		case ID_VSDUAL_LEFT:
			NES::Stop();
			GFX::Stop();
			Settings::VSDualScreens =0;
			Settings::ApplySettingsToMenu();
			if (running && RI.ConsoleType ==CONSOLE_VS && RI.INES2_VSFlags ==VS_DUAL) NES::WhichScreenToShow =0;
			GFX::Start();
			UpdateInterface();
			if (running) NES::Start(FALSE);
			break;
		case ID_VSDUAL_RIGHT:
			NES::Stop();
			GFX::Stop();
			Settings::VSDualScreens =1;
			Settings::ApplySettingsToMenu();
			if (running && RI.ConsoleType ==CONSOLE_VS && RI.INES2_VSFlags ==VS_DUAL) NES::WhichScreenToShow =1;
			GFX::Start();
			UpdateInterface();
			if (running) NES::Start(FALSE);
			break;
		case ID_VSDUAL_BOTH:
			NES::Stop();
			GFX::Stop();
			Settings::VSDualScreens =2;
			Settings::ApplySettingsToMenu();
			if (running && RI.ConsoleType ==CONSOLE_VS && RI.INES2_VSFlags ==VS_DUAL) NES::WhichScreenToShow =0;
			GFX::Start();
			UpdateInterface();
			if (running) NES::Start(FALSE);
			break;
		case ID_SAVE_FROM_MULTI:
			FileName[0] = 0;
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hMainWnd;
			ofn.hInstance = hInst;
			ofn.lpstrFilter =	_T("iNES-format ROM images (*.NES)\0")
							_T("*.nes\0")
						_T("\0");
			ofn.lpstrCustomFilter = NULL;
			ofn.nFilterIndex = 1;
			ofn.lpstrFile = FileName;
			ofn.nMaxFile = MAX_PATH;
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;
			ofn.lpstrInitialDir = Settings::Path_ROM;
			ofn.lpstrTitle = _T("Save current game from multicart");
			ofn.Flags = OFN_OVERWRITEPROMPT;
			ofn.lpstrDefExt = _T("nes");
			ofn.lCustData = 0;
			ofn.lpfnHook = NULL;
			ofn.lpTemplateName = NULL;
			if (EI.multiCanSave && GetSaveFileName(&ofn)) {
				_tcscpy(Settings::Path_ROM, FileName);
				Settings::Path_ROM[ofn.nFileOffset-1] = 0;

				FILE *F =_tfopen(FileName, _T("wb"));
				if (F) {
					uint8_t header[16];
					memset(header, 0x00, 16);
					header[0] ='N';
					header[1] ='E';
					header[2] ='S';
					header[3] =0x1A;

					int n =*EI.multiPRGSize;
					header[4] = (n /16384) & 0xFF;
					header[9] &= 0xF0;
					header[9] |= ((n /16384) & 0xF00) >> 8;
					if (n >= 64*1024*1024 || n %16384) {
						header[9] |=0xF;
						int multiplier =1;
						if ((n %3) ==0) multiplier =3;
						if ((n %5) ==0) multiplier =5;
						if ((n %7) ==0) multiplier =7;
						int exponent =log2 (n /multiplier);
						header[4] =(exponent <<2) | ((multiplier -1) >>1);
					}

					if (RI.ConsoleType ==CONSOLE_VT369) *EI.multiMapper =256;
					
					n =*EI.multiCHRSize;
					header[5] = (n /8192) & 0xFF;
					header[9] &= 0x0F;
					header[9] |= ((n /8192) & 0xF00) >> 4;
					if (n >= 32*1024*1024 || n %8192) {
						header[9] |=0xF0;
						int multiplier =1;
						if ((n %3) ==0) multiplier =3;
						if ((n %5) ==0) multiplier =5;
						if ((n %7) ==0) multiplier =7;
						int exponent =log2 (n /multiplier);
						header[5] =(exponent <<2) | ((multiplier -1) >>1);
					}
					if (n ==0 && *EI.multiMapper <256) header[11] =7;

					n =*EI.multiMapper;
					header[6] &= 0x0F;
					header[6] |= (n & 0x00F) << 4;
					header[7]  = 0x08; // indicate NES 2.0
					header[7] |= (n & 0x0F0) << 0;
					header[8] |= (n & 0xF00) >> 8;
					header[8] |= (*EI.multiSubmapper) <<4;
					if (*EI.multiMapper <4) {
						if (*EI.multiMirroring ==0) header[6] |=1; // Vertical mirroring
						if (*EI.multiMirroring ==4) header[6] |=8; // 4-Screen
					}
					if (RI.ConsoleType >2) {
						header[7] |=3;
						header[13] =RI.ConsoleType;
					} else
						header[7] |=RI.ConsoleType;
					
					header[15] =1;
					
					//if (RI.MiscROMSize) header[14] =1;
					
					fwrite(header, 1, 16, F);
					fwrite(*EI.multiPRGStart, 1, *EI.multiPRGSize, F);
					fwrite(*EI.multiCHRStart, 1, *EI.multiCHRSize, F);
					//if (RI.MiscROMSize) fwrite(RI.MiscROMData, 1, RI.MiscROMSize, F);
					fclose(F);
				}
			}
			break;
		case ID_PLUG_NONE:
		case ID_PLUG_GAME_GENIE:
		case ID_PLUG_PRO_ACTION_REPLAY:
		case ID_PLUG_BUNG_GD1M:
		case ID_PLUG_BUNG_SGD2M:
		case ID_PLUG_BUNG_SGD4M:
		case ID_PLUG_BUNG_GM19:
		case ID_PLUG_BUNG_GM20:
		case ID_PLUG_BUNG_GMK:
		case ID_PLUG_BUNG_GMB:
		case ID_PLUG_QJ_GAR:
		case ID_PLUG_DRPCJR:
		case ID_PLUG_VENUS_MD:
		case ID_PLUG_VENUS_GC1M:
		case ID_PLUG_VENUS_GC2M:
		case ID_PLUG_VENUS_TGD_4P:
		case ID_PLUG_VENUS_TGD_6P:
		case ID_PLUG_VENUS_TGD_6M:
		case ID_PLUG_FFE_MC2M:
		case ID_PLUG_FFE_MC4M:
		case ID_PLUG_FFE_SMC:
		case ID_PLUG_TC_MC2T:
		case ID_PLUG_SB97:
		case ID_PLUG_SOUSEIKI_FAMMY:
		case ID_PLUG_BIT79:
		{
			if (Settings::plugThruDevice ==wmId) break;
			if (MessageBox(hMainWnd, _T("Changing this setting causes a reset. Continue?"), _T("Machine Type/Plug-through Device"), MB_YESNO) !=IDYES) break;
			if (!NES::unload28()) break;
			TCHAR CurrentFilename[MAX_PATH];
			memcpy(CurrentFilename, NES::CurrentFilename, sizeof(CurrentFilename));
			NES::Stop();
			NES::CloseFile();

			Settings::plugThruDevice =wmId;
			Settings::ApplySettingsToMenu();

			if (wcslen(CurrentFilename)) NES::OpenFile(CurrentFilename);
			break;
		}
		case ID_HARD_GAME_SAVER:
			if (NES::currentPlugThruDevice ==ID_PLUG_VENUS_TGD_4P || NES::currentPlugThruDevice ==ID_PLUG_VENUS_TGD_6P || NES::currentPlugThruDevice ==ID_PLUG_VENUS_TGD_6M) {
				if (MessageBox(hMainWnd, _T("Changing this setting causes a reset. Continue?"), _T("Venus Hard Game Saver"), MB_YESNO) !=IDYES) break;
				if (!NES::unload28()) break;
				TCHAR CurrentFilename[MAX_PATH];
				memcpy(CurrentFilename, NES::CurrentFilename, sizeof(CurrentFilename));
				NES::Stop();
				NES::CloseFile();
				
				Settings::HardGameSaver =!Settings::HardGameSaver;
				Settings::ApplySettingsToMenu();
				if (wcslen(CurrentFilename)) NES::OpenFile(CurrentFilename);
				break;
			}
			Settings::HardGameSaver =!Settings::HardGameSaver;
			Settings::ApplySettingsToMenu();
			break;
		case ID_COMMIT35_DISCARD:
			Settings::CommitMode35 =Settings::COMMIT_DISCARD;
			Settings::ApplySettingsToMenu();
			break;
		case ID_COMMIT35_APPDATA:
			Settings::CommitMode35 =Settings::COMMIT_APPDATA;
			Settings::ApplySettingsToMenu();
			break;
		case ID_COMMIT35_DIRECTLY:
			Settings::CommitMode35 =Settings::COMMIT_DIRECTLY;
			Settings::ApplySettingsToMenu();
			break;
		case ID_GAME_BUTTON:
			PlugThruDevice::pressButton();
			break;
		case ID_FILE_REPLACE:
			if (!NES::unload28()) break;
			FileName[0] = 0;
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hMainWnd;
			ofn.hInstance = hInst;
			ofn.lpstrFilter =	_T("Famicom Disk System Disk Images (*.FDS, *.BIN, *.RAW, *.FQD, *.A)\0")
							_T("*.FDS;*.BIN;*.RAW;*.FQD;*.A\0")
						_T("\0");
			ofn.lpstrCustomFilter = NULL;
			ofn.nFilterIndex = 1;
			ofn.lpstrFile = FileName;
			ofn.nMaxFile = MAX_PATH;
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;
			ofn.lpstrInitialDir = Settings::Path_ROM;
			ofn.lpstrTitle = _T("Load new FDS file");
			ofn.Flags = OFN_FILEMUSTEXIST;
			ofn.lpstrDefExt = NULL;
			ofn.lCustData = 0;
			ofn.lpfnHook = NULL;
			ofn.lpTemplateName = NULL;
			if (GetOpenFileName(&ofn)) {
				_tcscpy(Settings::Path_ROM, FileName);
				Settings::Path_ROM[ofn.nFileOffset-1] = 0;
				NES::Stop();
				NES::unload28();
				FILE *F =_tfopen(FileName, L"rb");
				NES::load28(F, FileName);
				fclose(F);
				if (running) NES::Start(FALSE);
			}
			break;
		case ID_FDS_INSERT:
			NES::insert28();
			EI.DbgOut(_T("Inserted 2.8 side %d"), NES::side28);
			break;
		case ID_FDS_EJECT:
			NES::eject28();
			EI.DbgOut(_T("Ejected 2.8"));
			break;
		case ID_FDS_NEXT:
			NES::next28();
			EI.DbgOut(_T("Selected 2.8 side %d"), NES::side28);
			break;
		case ID_LOAD_35:
			FileName[0] = 0;
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hMainWnd;
			ofn.hInstance = hInst;
			ofn.lpstrFilter =	_T("MS-DOS Sector Images (*.IMA, *.IMG)\0")
							_T("*.IMA;*.IMG;\0")
						_T("\0");
			ofn.lpstrCustomFilter = NULL;
			ofn.nFilterIndex = 1;
			ofn.lpstrFile = FileName;
			ofn.nMaxFile = MAX_PATH;
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;
			ofn.lpstrInitialDir = Settings::Path_ROM;
			ofn.lpstrTitle = _T("Insert 3.5\" disk");
			ofn.Flags = OFN_FILEMUSTEXIST;
			ofn.lpstrDefExt = NULL;
			ofn.lCustData = 0;
			ofn.lpfnHook = NULL;
			ofn.lpTemplateName = NULL;
			if (GetOpenFileName(&ofn)) {
				_tcscpy(Settings::Path_ROM, FileName);
				Settings::Path_ROM[ofn.nFileOffset-1] = 0;
				NES::load35(FileName);
			}
			break;
		case ID_FASTLOAD:
			if (MessageBox(hMainWnd, _T("Changing this setting causes a reset. Continue?"), _T("Fast-load disk images"), MB_YESNO) !=IDYES) break;
			if (!NES::unload28()) break;
			TCHAR CurrentFilename[MAX_PATH];
			memcpy(CurrentFilename, NES::CurrentFilename, sizeof(CurrentFilename));
			NES::Stop();
			NES::CloseFile();

			Settings::FastLoad =!Settings::FastLoad;
			Settings::ApplySettingsToMenu();

			if (wcslen(CurrentFilename)) NES::OpenFile(CurrentFilename);
			break;
		case ID_FILE_SKIP_SRAM_SAVE:
			Settings::SkipSRAMSave =!Settings::SkipSRAMSave;
			Settings::ApplySettingsToMenu();
			break;
		case ID_MISC_COPYNES:
			FileName[0] = 0;
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hMainWnd;
			ofn.hInstance = hInst;
			ofn.lpstrFilter =	_T("CopyNES Plugins (*.BIN)\0")
							_T("*.BIN;\0")
						_T("\0");
			ofn.lpstrCustomFilter = NULL;
			ofn.nFilterIndex = 1;
			ofn.lpstrFile = FileName;
			ofn.nMaxFile = MAX_PATH;
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;
			ofn.lpstrInitialDir = Settings::Path_CopyNES;
			ofn.lpstrTitle = _T("CopyNES Plugin");
			ofn.Flags = OFN_FILEMUSTEXIST;
			ofn.lpstrDefExt = NULL;
			ofn.lCustData = 0;
			ofn.lpfnHook = NULL;
			ofn.lpTemplateName = NULL;
			if (GetOpenFileName(&ofn)) {
				_tcscpy(Settings::Path_CopyNES, FileName);
				Settings::Path_CopyNES[ofn.nFileOffset-1] = 0;
				NES::Pause(FALSE);
				if (Movie::Mode) Movie::Stop();
				NES::Reset(RESET_HARD);
				NES::runCopyNES(FileName);
				if (running) NES::Resume();
			}
			break;
		case ID_VT369_SOUND_HLE:
			if (MessageBox(hMainWnd, _T("Changing this setting causes a reset. Continue?"), _T("VT369 Sound HLE"), MB_YESNO) !=IDYES) break;
			NES::Pause(FALSE);
			if (Movie::Mode) Movie::Stop();
			NES::DestroyMachine();
			
			Settings::VT369SoundHLE =!Settings::VT369SoundHLE;
			Settings::ApplySettingsToMenu();

			NES::CreateMachine();
			NES::Reset(RESET_HARD);
			if (running) NES::Resume();
			break;
		default:return DefWindowProc(hWnd, message, wParam, lParam);
			break;
		}
		break;
	case WM_DROPFILES:
		DragQueryFile((HDROP)wParam, 0, FileName, MAX_PATH);
		DragFinish((HDROP)wParam);
		NES::Stop();
		NES::OpenFile(FileName);
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		if (!NES::Running) GFX::Repaint();
		EndPaint(hWnd, &ps);
		break;
	case WM_CLOSE:
		if (!NES::unload28()) break;
		NES::Stop();
		// Cannot safely shutdown DirectDraw while in fullscreen mode,
		// so defer it to the message loop
		if (GFX::DirectDraw && GFX::Fullscreen)
			NES::DoStop |= STOPMODE_QUIT;
		else
			NES::Destroy();
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_SETCURSOR:
		CursorOnOutput =false;
		if (LOWORD(lParam) == HTCLIENT && !GFX::Fullscreen) {
			CursorOnOutput =true;
			SetCursor(*CurrentCursor);
			if (MaskMouse && Controllers::scrollLock)
				while(ShowCursor(FALSE) >=0);
			else
				while(ShowCursor(TRUE) <0);
			return true;
		}
		break;
	case WM_COPYDATA:
		if (NES::currentPlugThruDevice ==ID_PLUG_FFE_SMC) PlugThruDevice::SuperMagicCard::addToReceiveQueue(wParam);		
		return TRUE;
	case WM_SYSCOMMAND:
		// disallow screen saver while emulating (doesn't work if password protected)
		if (running && (((wParam & 0xFFF0) == SC_SCREENSAVE) || ((wParam & 0xFFF0) == SC_MONITORPOWER)))
			return 0;
		// else fall through
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	}
	return 0;
}

INT_PTR CALLBACK	About (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		return TRUE;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}

INT_PTR CALLBACK	DebugWnd (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		SendDlgItemMessage(hDlg, IDC_DEBUGTEXT, EM_SETLIMITTEXT, 0, 0);
		SetWindowPos(hDlg, hMainWnd, 0, 0, 0, 0, SWP_HIDEWINDOW | SWP_NOOWNERZORDER | SWP_NOSIZE);
		return FALSE;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDCANCEL) {
			Settings::dbgVisible = FALSE;
			ShowWindow(hDebug, SW_HIDE);
			CheckMenuItem(hMenu, ID_DEBUG_STATWND, MF_UNCHECKED);
			return TRUE;
		}
		break;
	case WM_SIZE:
		MoveWindow(GetDlgItem(hDlg, IDC_DEBUGTEXT), 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
		return TRUE;
	}
	return FALSE;
}
void	AddDebug (const TCHAR *txt)
{
	int dbglen = GetWindowTextLength(GetDlgItem(hDebug, IDC_DEBUGTEXT));
//	if (!dbgVisible)
//		return;
	SendDlgItemMessage(hDebug, IDC_DEBUGTEXT, EM_SETSEL, dbglen, dbglen);
	SendDlgItemMessage(hDebug, IDC_DEBUGTEXT, EM_REPLACESEL, FALSE, (LPARAM)txt);
	dbglen += (int)_tcslen(txt);
	SendDlgItemMessage(hDebug, IDC_DEBUGTEXT, EM_SETSEL, dbglen, dbglen);
	SendDlgItemMessage(hDebug, IDC_DEBUGTEXT, EM_REPLACESEL, FALSE, (LPARAM)_T("\r\n"));
}

// Shortcut for browsing to folders (though it could be used to run anything else, it's only ever called with folder names)
void	BrowseFolder (const TCHAR *dir)
{
	ShellExecute(hMainWnd, NULL, dir, NULL, NULL, SW_SHOWNORMAL);
}

BOOL	ProcessMessages (void)
{
	MSG msg;
	BOOL gotMessage = PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		// See message loop in WinMain for explanation on why this is here
		if (NES::DoStop & STOPMODE_BREAK)
		{
			NES::DoStop &= ~STOPMODE_BREAK;
			Controllers::UnAcquire();
		}
		if (Controllers::scrollLock && MaskKeyboard || !TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return gotMessage;
}

void	UpdateTitlebar (void)
{
	TCHAR titlebar[256];
	if (NES::Running)
		_stprintf(titlebar, _T("Furbtendulator - %i FPS (%i %sFSkip)"), GFX::FPSnum, Settings::FSkip, Settings::aFSkip?_T("Auto"):_T(""));
		//_stprintf(titlebar, _T("NintendulatorNRS"));
	else
		_tcscpy(titlebar, _T("Furbtendulator - Stopped"));
	if (TitlebarDelay > 0)
	{
		TitlebarDelay--;
		_tcscat(titlebar, _T(" - "));
		_tcscat(titlebar, TitlebarBuffer);
	}
	SetWindowText(hMainWnd, titlebar);
}
void	__cdecl	PrintTitlebar (const TCHAR *Text, ...)
{
	va_list marker;
	va_start(marker, Text);
	_vstprintf(TitlebarBuffer, Text, marker);
	va_end(marker);
	TitlebarDelay = 15;
	UpdateTitlebar();
}

void	UpdateInterface (void) {
	HWND hWnd = hMainWnd;

	int w = GFX::SIZEX * Settings::SizeMult;
	int h = GFX::SIZEY * Settings::SizeMult;

	if (Settings::FixAspect) {
		if (NES::CurRegion ==Settings::REGION_NTSC || (Settings::DendyNTSCAspect && NES::CurRegion==Settings::REGION_DENDY))
			w = w * 8 / 7;
		else	w = w * 355 / 256; // 2950000 / 2128137
	}
	if (RI.ConsoleType ==CONSOLE_VS && RI.INES2_VSFlags ==VS_DUAL && Settings::VSDualScreens ==2) w *=2;
	if (RI.ROMType ==ROM_NSF) {
		w =GFX::SIZEX;
		h =GFX::SIZEY;
	}

	RECT client, window, desktop;
	SetWindowPos(hWnd, hWnd, 0, 0, w, h, SWP_NOMOVE | SWP_NOZORDER);
	GetClientRect(hWnd, &client);
	SetWindowPos(hWnd, hWnd, 0, 0, 2 * w - client.right, 2 * h - client.bottom, SWP_NOMOVE | SWP_NOZORDER);

	// try to make sure the window is completely visible on the desktop
	desktop.left = GetSystemMetrics(SM_XVIRTUALSCREEN);
	desktop.top = GetSystemMetrics(SM_YVIRTUALSCREEN);
	desktop.right = desktop.left + GetSystemMetrics(SM_CXVIRTUALSCREEN);
	desktop.bottom = desktop.top + GetSystemMetrics(SM_CYVIRTUALSCREEN);

	bool moved = false;
	GetWindowRect(hWnd, &window);
	// check right/bottom first, then left/top
	if (window.right > desktop.right)
	{
		window.left -= window.right - desktop.right;
		window.right = desktop.right;
		moved = true;
	}
	if (window.bottom > desktop.bottom)
	{
		window.top -= window.bottom - desktop.bottom;
		window.bottom = desktop.bottom;
		moved = true;
	}
	// that way, if the window is bigger than the desktop, the bottom/right will be cut off instead of the top/left
	if (window.left < desktop.left)
	{
		window.right += desktop.left - window.left;
		window.left = desktop.left;
		moved = true;
	}
	if (window.top < desktop.top)
	{
		window.bottom += desktop.top - window.top;
		window.top = desktop.top;
		moved = true;
	}
	// it's still technically possible for the window to get lost if the desktop is not rectangular (e.g. one monitor is rotated)
	// but this should take care of most of the complaints about the window getting lost
	if (moved)
		SetWindowPos(hWnd, hWnd, window.left, window.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}


