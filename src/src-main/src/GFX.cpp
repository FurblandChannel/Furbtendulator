#include "stdafx.h"
#include "Nintendulator.h"
#include "Settings.h"
#include "resource.h"
#include "MapperInterface.h"
#include "Controllers.h"
#include "NES.h"
#include "GFX.h"
#include "PPU.h"
#include "AVI.h"
#include "CPU.h"
#include "Sound.h"
#include "APU.h"
#include "OneBus.h"
#include "OneBus_VT369.h"
#include <commctrl.h>

//#define SHOW_COLOR_NUM

#if (_MSC_VER < 1400)
// newer versions of the DirectX SDK helpfully fail to include ddraw.lib
// and those newer versions can only be used in Visual Studio 2005 and later
// If we're using .NET 2003 or earlier, it's definitely available
// Otherwise, we need to do LoadLibrary/GetProcAddress
#pragma comment(lib, "ddraw.lib")
#endif
#pragma comment(lib, "dxguid.lib")

#define	_USE_MATH_DEFINES
#include <math.h>

#ifndef M_PI
#define M_PI	3.14159265358979323846
#endif	/* !M_PI */

namespace GFX {
bool    apertureChanged =false;
bool	smallLCDMode =false;
bool	verticalMode =false;
bool	verticalFlip =false;
int	OFFSETX =0;
int	OFFSETY =0;
int	SIZEX =256;
int	SIZEY =240;
uint32_t frameCount =0;

unsigned char RawPalette[8][64][3];
unsigned short Palette15[PALETTE_END];
unsigned short Palette16[PALETTE_END];
unsigned long Palette32[PALETTE_END];
Settings::PaletteRegion PaletteRegion;
Settings::PALETTE CurrentPalNum =Settings::PALETTE_MAX;
char Depth;
BOOL Fullscreen =FALSE;
BOOL ScreenshotRequested =FALSE;

LARGE_INTEGER ClockFreq;
LARGE_INTEGER LastClockVal;
int FPSnum, FPSCnt;

int Pitch;
int WantFPS;
int aFPScnt;
LONGLONG aFPSnum;

BOOL SlowDown;
int SlowRate;
int FullscreenBorderH;

BOOL InError, RepaintInProgress =FALSE;

LPDIRECTDRAW7		DirectDraw;
LPDIRECTDRAWSURFACE7	PrimarySurf, SecondarySurf;
LPDIRECTDRAWCLIPPER	Clipper;
DDSURFACEDESC2		SurfDesc;

#if (_MSC_VER >= 1400)
typedef HRESULT (WINAPI *LPDIRECTDRAWCREATEEX)(GUID FAR *, LPVOID *, REFIID,IUnknown FAR *);
HINSTANCE dDrawInst;
LPDIRECTDRAWCREATEEX DirectDrawCreateEx;
#endif

#define	Try(action,errormsg) do {\
	if (FAILED(action))\
	{\
		InError = TRUE;\
		RepaintInProgress =FALSE;\
		Stop();\
		MessageBox(hMainWnd, errormsg _T(", retrying"), _T("Nintendulator"), MB_OK | MB_ICONWARNING);\
		Fullscreen = FALSE;\
		Start();\
		InError = FALSE;\
		if (FAILED(action))\
		{\
			MessageBox(hMainWnd, _T("Error: ") errormsg, _T("Nintendulator"), MB_OK | MB_ICONERROR);\
			return;\
		}\
	}\
} while (false)

void	Init (void) {
	ZeroMemory(&SurfDesc, sizeof(SurfDesc));
	ZeroMemory(Palette15, sizeof(Palette15));
	ZeroMemory(Palette16, sizeof(Palette16));
	ZeroMemory(Palette32, sizeof(Palette32));
	DirectDraw = NULL;
	PrimarySurf = NULL;
	SecondarySurf = NULL;
	Clipper = NULL;
#if (_MSC_VER >= 1400)
	dDrawInst = NULL;
	DirectDrawCreateEx = NULL;
#endif
	Pitch = 0;
	WantFPS = 0;
	FPSCnt = 0;
	FPSnum = 0;
	aFPScnt = 0;
	aFPSnum = 0;
	Depth = 0;
	ClockFreq.QuadPart = 0;
	LastClockVal.QuadPart = 0;
	//Fullscreen = FALSE;
	InError = FALSE;

	if (!QueryPerformanceFrequency(&ClockFreq))
	{
		MessageBox(hMainWnd, _T("Failed to determine performance counter frequency!"), _T("Nintendulator"), MB_OK | MB_ICONERROR);
		Stop();
		return;
	}

#if (_MSC_VER >= 1400)
	dDrawInst = LoadLibrary(_T("ddraw.dll"));
	if (!dDrawInst)
	{
		MessageBox(hMainWnd, _T("Fatal error: unable to load DirectDraw DLL!"), _T("Nintendulator"), MB_OK | MB_ICONERROR);
		return;
	}
	DirectDrawCreateEx = (LPDIRECTDRAWCREATEEX)GetProcAddress(dDrawInst, "DirectDrawCreateEx");
	if (!DirectDrawCreateEx)
	{
		MessageBox(hMainWnd, _T("Fatal error: unable to locate entry point for DirectDrawCreateEx!"), _T("Nintendulator"), MB_OK | MB_ICONERROR);
		Destroy();
		return;
	}
#endif
}

void	Destroy (void) {
	Stop();
#if (_MSC_VER >= 1400)
	DirectDrawCreateEx = NULL;
	if (dDrawInst) {
		FreeLibrary(dDrawInst);
		dDrawInst = NULL;
	}
#endif
}

void	SetRegion (void) {
	PaletteRegion =Settings::PALREGION_NONE;
	switch (NES::CurRegion) {
	case Settings::REGION_NTSC:
		WantFPS = 60;
		PaletteRegion =Settings::PALREGION_NTSC;
		break;
	case Settings::REGION_PAL:
		WantFPS = 50;
		PaletteRegion =Settings::PALREGION_PAL;
		break;
	case Settings::REGION_DENDY:
		WantFPS = (Settings::Dendy60Hz)? 60: 50;
		PaletteRegion =Settings::PALREGION_DENDY;
		break;
	default:
		EI.DbgOut(_T("Invalid GFX region selected!"));
		return;
	}
	if (RI.ConsoleType ==CONSOLE_VS)   PaletteRegion =Settings::PALREGION_VS;
	if (RI.ConsoleType ==CONSOLE_PC10) PaletteRegion =Settings::PALREGION_PC10;
	CurrentPalNum =Settings::PALETTE_MAX;
	LoadPalette(Settings::Palette[PaletteRegion]);
}

#include "dderr.h"
void	Start (void) {
	HRESULT	result;
	TCHAR strResult [256];

	if (Settings::Xstart <-16) Settings::Xstart =-16;
	if (Settings::Xstart >266) Settings::Xstart =266;
	if (Settings::Ystart <  0) Settings::Ystart =  0;
	if (Settings::Ystart >241) Settings::Ystart =241;
	if (Settings::Xend <Settings::Xstart) Settings::Xend =Settings::Xstart;
	if (Settings::Xend >266) Settings::Xend =266;
	if (Settings::Yend <Settings::Ystart) Settings::Yend =Settings::Ystart;
	if (Settings::Yend >241) Settings::Yend =241;
	OFFSETX =Settings::Xstart +16;
	OFFSETY =Settings::Ystart;
	SIZEX =Settings::Xend -Settings::Xstart +1;
	SIZEY =Settings::Yend -Settings::Ystart +1;
	if (smallLCDMode && !verticalMode) {
		OFFSETX =reg2000[0x43] -64 +16;
		OFFSETY =reg2000[0x46];
		SIZEX =160;
		SIZEY =128;
	}
	if (smallLCDMode && verticalMode) {
		OFFSETX =reg2000[0x46] +5;
		OFFSETY =reg2000[0x43] -64 -17;
		SIZEX =160;
		SIZEY =128;
	}
	if (!smallLCDMode && verticalMode) {
		OFFSETX =133;
		OFFSETY =-1;
		SIZEX =240;
		SIZEY =256;
	}
	if (FAILED(DirectDrawCreateEx(NULL, (LPVOID *)&DirectDraw, IID_IDirectDraw7, NULL))) {
		MessageBox(hMainWnd, _T("Failed to initialize DirectDraw 7"), _T("Nintendulator"), MB_OK | MB_ICONERROR);
		return;
	}
	if (Fullscreen) {
		OFFSETX =16;
		OFFSETY =0;
		SIZEX =256;
		SIZEY =240;
		// Examine the current screen resolution and try to figure out the current aspect ratio
		// We'll support 4:3, 16:10, and 16:9
		double ratio = (double)GetSystemMetrics(SM_CXSCREEN) / (double)GetSystemMetrics(SM_CYSCREEN);
		// Not all of the widescreen resolutions will work
		// The code below will try each resolution until it finds one that works
		const int widths[] = {
			640,		// 4:3 - last offset 0
			720, 768,	// 16:10 - last offset 2
			848, 856, 864	// 16:9 - last offset 5
		};
		// only try switching to a particular resolution once, and remember it as long as the program is running
		static BOOL widths_ok[] = {
			TRUE,
			TRUE, TRUE,
			TRUE, TRUE, TRUE
		};
		// pick a resolution with a decent aspect ratio
		int i;
		if (ratio < 1.4)
			i = 0;
		else if (ratio < 1.7)
			i = 2;
		else	i = 5;
		// if the final resolution failed, then don't even allow fullscreen at all
		if (!widths_ok[0]) {
			// no need to Stop() here, since we didn't start anything yet
			MessageBox(hMainWnd, _T("No fullscreen resolutions are supported on your display device!"), _T("Nintendulator"), MB_OK | MB_ICONERROR);
			Fullscreen = FALSE;
			Start();
			return;
		}

		if (FAILED(DirectDraw->SetCooperativeLevel(hMainWnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_NOWINDOWCHANGES | DDSCL_MULTITHREADED))) {
			Stop();
			MessageBox(hMainWnd, _T("Failed to set fullscreen cooperative level! Returning to Windowed mode..."), _T("Nintendulator"), MB_OK | MB_ICONERROR);
			Fullscreen = FALSE;
			Start();
			return;
		}
		if (Settings::dbgVisible) ShowWindow(hDebug, SW_MINIMIZE);
		while (1) {
			FullscreenBorderH = (widths[i] - 512) / 2;
			if (!widths_ok[i] || FAILED(DirectDraw->SetDisplayMode(widths[i], 480, 32, 0, 0))) {
				widths_ok[i] = FALSE;
				if (i == 0)
				{
					// not even 640x480 worked!
					Stop();
					MessageBox(hMainWnd, _T("No fullscreen resolutions are supported on your display device! Reverting to Windowed mode..."), _T("Nintendulator"), MB_OK | MB_ICONERROR);
					Fullscreen = FALSE;
					// drop to windowed
					Start();
					return;
				}
				else	i--;
			}
			else	break;
		}
		SetWindowLongPtr(hMainWnd, GWL_STYLE, WS_POPUP);
		SetMenu(hMainWnd, NULL);
		ShowWindow(hMainWnd, SW_MAXIMIZE);
	}
	else
	{
		if (FAILED(DirectDraw->SetCooperativeLevel(hMainWnd, DDSCL_NORMAL))) {
			Stop();
			MessageBox(hMainWnd, _T("Failed to set DirectDraw cooperative level"), _T("Nintendulator"), MB_OK | MB_ICONERROR);
			return;
		}
	}

	ZeroMemory(&SurfDesc, sizeof(SurfDesc));
	SurfDesc.dwSize = sizeof(SurfDesc);

	if (Fullscreen) {
		SurfDesc.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
		SurfDesc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
		SurfDesc.dwBackBufferCount = 1;

		result = DirectDraw->CreateSurface(&SurfDesc, &PrimarySurf, NULL);
		if (FAILED(result)) {
			_stprintf_s(strResult, 256, _T("Failed to create primary surface: %s"), DirectDrawErrorCode(result)),
			Stop();
			MessageBox(hMainWnd, strResult, _T("Nintendulator"), MB_OK | MB_ICONERROR);
			return;
		}
		if (PrimarySurf ==NULL) {
			Stop();
			return;
		}

		SurfDesc.ddsCaps.dwCaps = DDSCAPS_BACKBUFFER;
		if (FAILED(PrimarySurf->GetAttachedSurface(&SurfDesc.ddsCaps, &SecondarySurf)))
		{
			Stop();
			MessageBox(hMainWnd, _T("Failed to get secondary surface"), _T("Nintendulator"), MB_OK | MB_ICONERROR);
			return;
		}
		while(ShowCursor(FALSE) >= 0);
	} else 	{
		SurfDesc.dwFlags = DDSD_CAPS;
		SurfDesc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

		if (FAILED(DirectDraw->CreateSurface(&SurfDesc, &PrimarySurf, NULL)))
		{
			Stop();
			MessageBox(hMainWnd, _T("Failed to create primary surface"), _T("Nintendulator"), MB_OK | MB_ICONERROR);
			return;
		}

		SurfDesc.dwWidth = SIZEX*2;
		SurfDesc.dwHeight = SIZEY*2;
		if (RI.ConsoleType ==CONSOLE_VS && RI.INES2_VSFlags ==VS_DUAL && Settings::VSDualScreens ==2) SurfDesc.dwWidth *=2;

		SurfDesc.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
		SurfDesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN ;

		if (FAILED(DirectDraw->CreateSurface(&SurfDesc, &SecondarySurf, NULL)))
		{
			Stop();
			MessageBox(hMainWnd, _T("Failed to create secondary surface"), _T("Nintendulator"), MB_OK | MB_ICONERROR);
			return;
		}
	}

	if (!Fullscreen) {
		if (FAILED(DirectDraw->CreateClipper(0, &Clipper, NULL))) {
			Stop();
			MessageBox(hMainWnd, _T("Failed to create clipper"), _T("Nintendulator"), MB_OK | MB_ICONERROR);
			return;
		}

		if (FAILED(Clipper->SetHWnd(0, hMainWnd))) {
			Stop();
			MessageBox(hMainWnd, _T("Failed to set clipper window"), _T("Nintendulator"), MB_OK | MB_ICONERROR);
			return;
		}

		if (FAILED(PrimarySurf->SetClipper(Clipper))) {
			Stop();
			MessageBox(hMainWnd, _T("Failed to assign clipper to primary surface"), _T("Nintendulator"), MB_OK | MB_ICONERROR);
			return;
		}
	}

	ZeroMemory(&SurfDesc, sizeof(SurfDesc));
	SurfDesc.dwSize = sizeof(SurfDesc);

	if (FAILED(SecondarySurf->GetSurfaceDesc(&SurfDesc)))
	{
		Stop();
		MessageBox(hMainWnd, _T("Failed to retrieve surface description"), _T("Nintendulator"), MB_OK | MB_ICONERROR);
		return;
	}

	Pitch = SurfDesc.lPitch;
	FPSCnt = Settings::FSkip;

	switch (SurfDesc.ddpfPixelFormat.dwRGBBitCount)
	{
	case 16:if (SurfDesc.ddpfPixelFormat.dwRBitMask == 0xF800)
			Depth = 16;
		else	Depth = 15;	break;
	case 32:Depth = 32;		break;
	default:
		Stop();
		MessageBox(hMainWnd, _T("Invalid bit depth detected!"), _T("Nintendulator"), MB_OK | MB_ICONERROR);
		return;			break;
	}

	// this will automatically call Update()
	//EI.DbgOut(_T("Created %ix%i %i-bit display surface (%s)"), SurfDesc.dwWidth, SurfDesc.dwHeight, Depth, Fullscreen ? _T("fullscreen") : _T("windowed"));
	LoadPalette(Settings::PALETTE_MAX);
}

void	Stop (void) {
	while (RepaintInProgress);

	if (!DirectDraw)
		return;
	if (Clipper) {
		if (PrimarySurf) PrimarySurf->SetClipper(NULL);
		Clipper->Release();
		Clipper = NULL;
	}
	if (SecondarySurf) {
		SecondarySurf->Release();
		SecondarySurf = NULL;
	}
	if (PrimarySurf) {
		PrimarySurf->Release();
		PrimarySurf = NULL;
	}
	if (Fullscreen) {
		DirectDraw->RestoreDisplayMode();
		SetWindowLongPtr(hMainWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
		SetMenu(hMainWnd, hMenu);
		ShowWindow(hMainWnd, SW_RESTORE);
		if (Settings::dbgVisible) ShowWindow(hDebug, SW_RESTORE);
		UpdateInterface();
	}
	if (DirectDraw) {
		DirectDraw->Release();
		DirectDraw = NULL;
	}
	ShowCursor(TRUE);
}

int TitleDelay = 0;
void	DrawScreen (void) {
	LARGE_INTEGER TmpClockVal;
	frameCount++;

	if (AVI::handle) AVI::AddVideo();
	if (SlowDown) Sleep(SlowRate *1000 /WantFPS);
	if (++FPSCnt >(Controllers::capsLock && !Controllers::scrollLock? 9: Settings::FSkip)) {
		Update();
		FPSCnt = 0;
	}
	QueryPerformanceCounter(&TmpClockVal);
	aFPSnum += TmpClockVal.QuadPart - LastClockVal.QuadPart;
	LastClockVal = TmpClockVal;
	if (++aFPScnt >=20) {
		FPSnum =(int)((ClockFreq.QuadPart * aFPScnt) / aFPSnum);
		if (Settings::aFSkip) {
			if ((Settings::FSkip < 9) && (FPSnum <= (WantFPS * 9 / 10))) Settings::FSkip++;
			if ((Settings::FSkip > 0) && (FPSnum >= (WantFPS - 1))) Settings::FSkip--;
			SetFrameskip(-1);
		}
		aFPScnt = 0;
		aFPSnum = 0;
	}
	if (!TitleDelay--) {
		UpdateTitlebar();
		TitleDelay =30;
	}
	if (prev_multiCanSave !=*EI.multiCanSave)
		EnableMenuItem(hMenu, ID_SAVE_FROM_MULTI, *EI.multiCanSave? MF_ENABLED: MF_GRAYED);
	prev_multiCanSave =*EI.multiCanSave;

}

void	SetFrameskip (int skip)
{
	if (skip >= 0)
		Settings::FSkip = skip;

	if ((skip == -2) ||
		(Controllers::Port1->Type == Controllers::STD_ZAPPER) || (Controllers::Port1->Type == Controllers::STD_VSZAPPER) ||
		(Controllers::Port2->Type == Controllers::STD_ZAPPER) || (Controllers::Port2->Type == Controllers::STD_VSZAPPER))
	{	// if Zapper, force it to zero frameskip, otherwise it won't work
		Settings::FSkip = 0;
		Settings::aFSkip = 0;
	}

	if (Settings::aFSkip)
		CheckMenuItem(hMenu, ID_PPU_FRAMESKIP_AUTO, MF_CHECKED);
	else	CheckMenuItem(hMenu, ID_PPU_FRAMESKIP_AUTO, MF_UNCHECKED);
	switch (Settings::FSkip)
	{
	case 0:	CheckMenuRadioItem(hMenu, ID_PPU_FRAMESKIP_0, ID_PPU_FRAMESKIP_9, ID_PPU_FRAMESKIP_0, MF_BYCOMMAND);	break;
	case 1:	CheckMenuRadioItem(hMenu, ID_PPU_FRAMESKIP_0, ID_PPU_FRAMESKIP_9, ID_PPU_FRAMESKIP_1, MF_BYCOMMAND);	break;
	case 2:	CheckMenuRadioItem(hMenu, ID_PPU_FRAMESKIP_0, ID_PPU_FRAMESKIP_9, ID_PPU_FRAMESKIP_2, MF_BYCOMMAND);	break;
	case 3:	CheckMenuRadioItem(hMenu, ID_PPU_FRAMESKIP_0, ID_PPU_FRAMESKIP_9, ID_PPU_FRAMESKIP_3, MF_BYCOMMAND);	break;
	case 4:	CheckMenuRadioItem(hMenu, ID_PPU_FRAMESKIP_0, ID_PPU_FRAMESKIP_9, ID_PPU_FRAMESKIP_4, MF_BYCOMMAND);	break;
	case 5:	CheckMenuRadioItem(hMenu, ID_PPU_FRAMESKIP_0, ID_PPU_FRAMESKIP_9, ID_PPU_FRAMESKIP_5, MF_BYCOMMAND);	break;
	case 6:	CheckMenuRadioItem(hMenu, ID_PPU_FRAMESKIP_0, ID_PPU_FRAMESKIP_9, ID_PPU_FRAMESKIP_6, MF_BYCOMMAND);	break;
	case 7:	CheckMenuRadioItem(hMenu, ID_PPU_FRAMESKIP_0, ID_PPU_FRAMESKIP_9, ID_PPU_FRAMESKIP_7, MF_BYCOMMAND);	break;
	case 8:	CheckMenuRadioItem(hMenu, ID_PPU_FRAMESKIP_0, ID_PPU_FRAMESKIP_9, ID_PPU_FRAMESKIP_8, MF_BYCOMMAND);	break;
	case 9:	CheckMenuRadioItem(hMenu, ID_PPU_FRAMESKIP_0, ID_PPU_FRAMESKIP_9, ID_PPU_FRAMESKIP_9, MF_BYCOMMAND);	break;
	}
}

void	SaveScreenshot (void) {
static unsigned char   bmpData[(341*3*2 +1)*242*2 +0x36] = {
                      0x42, 0x4D, 0x36, 0xB8, 0x0B, 0x00, 0x00, 0x00,
                      0x00, 0x00, 0x36, 0x00, 0x00, 0x00, 0x28, 0x00,
                      0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0xF0, 0x00,
                      0x00, 0x00, 0x01, 0x00, 0x18, 0x00, 0x00, 0x00,
                      0x00, 0x00, 0x00, 0xB8, 0x0B, 0x00, 0x00, 0x00,
                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
	if (PPU::PPU[0] ==NULL) return;
	TCHAR FileName[MAX_PATH] = {0};
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hMainWnd;
	ofn.hInstance = hInst;
	ofn.lpstrFilter = _T("BMP File (*.BMP)\0") _T("*.BMP\0") _T("\0");
	ofn.lpstrCustomFilter = NULL;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = FileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = Settings::Path_BMP;
	ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = _T("BMP");
	ofn.lCustData = 0;
	ofn.lpfnHook = NULL;
	ofn.lpTemplateName = NULL;

	if (!GetSaveFileName(&ofn)) return;
	_tcscpy(Settings::Path_BMP, FileName);
	Settings::Path_BMP[ofn.nFileOffset - 1] = 0;

	unsigned char  *dst =bmpData +0x36;

	memset(bmpData +0x36, 0x00, sizeof(bmpData) -0x36);

	if (RI.ConsoleType ==CONSOLE_VT369 && reg2000[0x1C] &0x04) {
		uint16_t* srcEven =dynamic_cast<PPU::PPU_VT369*>(PPU::PPU[0])->DrawArrayEven  +341*2*OFFSETY +2*OFFSETX;
		uint16_t* srcOdd  =dynamic_cast<PPU::PPU_VT369*>(PPU::PPU[0])->DrawArrayOdd   +341*2*OFFSETY +2*OFFSETX;
		int paddedSizeX =SIZEX*2*3;
		paddedSizeX +=(4 -(paddedSizeX &3)) &3;
		for (int y =0; y <SIZEY*2; y++) {
			int bmpLine = SIZEY*2 -1 -y;
			for (int x =0; x <SIZEX*2; x++) {
				unsigned short *src =y &1? srcOdd: srcEven;

				int rgb = Palette32[src[y/2*341*2 +x]];
				dst[bmpLine*paddedSizeX +x*3 +0] = (rgb >> 0) &0xFF;
				dst[bmpLine*paddedSizeX +x*3 +1] = (rgb >> 8) &0xFF;
				dst[bmpLine*paddedSizeX +x*3 +2] = (rgb >>16) &0xFF;
			}
		}
		FILE *F = _tfopen(FileName, TEXT("wb"));
		if (F) {
			size_t bmpSize =paddedSizeX*SIZEY*2;
			bmpData [0x12] = (SIZEX*2) & 0xff;
			bmpData [0x13] = (SIZEX*2) >> 8;
			bmpData [0x16] = (SIZEY*2) & 0xff;
			bmpData [0x17] = (SIZEY*2) >> 8;
			bmpData [0x22] = (bmpSize     ) & 0xff;
			bmpData [0x23] = (bmpSize >> 8) & 0xff;
			bmpData [0x24] = (bmpSize >>16) & 0xff;
			bmpData [0x25] = (bmpSize >>24) & 0xff;
			bmpSize += 0x36;
			bmpData [0x02] = (bmpSize     ) & 0xff;
			bmpData [0x03] = (bmpSize >> 8) & 0xff;
			bmpData [0x04] = (bmpSize >>16) & 0xff;
			bmpData [0x05] = (bmpSize >>24) & 0xff;

			fwrite(bmpData, 1, bmpSize, F);
			fclose(F);
		} else
			MessageBox(hMainWnd, _T("File open failed!"), _T("Nintendulator"), MB_OK);
	} else {
		uint16_t* src =PPU::PPU[0]->DrawArray;
		int paddedSizeX =SIZEX*3;
		paddedSizeX +=(4 -(paddedSizeX &3)) &3;
		for (int y =0; y <SIZEY; y++) {
			int bmpLine = SIZEY -1 -y;
			for (int x =0; x <SIZEX; x++) {
				int rgb = Palette32[src[(y +OFFSETY)*341 +x +OFFSETX]];
				dst[bmpLine*paddedSizeX +x*3 +0] = (rgb >> 0) &0xFF;
				dst[bmpLine*paddedSizeX +x*3 +1] = (rgb >> 8) &0xFF;
				dst[bmpLine*paddedSizeX +x*3 +2] = (rgb >>16) &0xFF;
			}
		}
		FILE *F = _tfopen(FileName, TEXT("wb"));
		if (F) {
			size_t bmpSize =paddedSizeX*SIZEY;
			bmpData [0x12] = SIZEX & 0xff;
			bmpData [0x13] = SIZEX >> 8;
			bmpData [0x16] = SIZEY & 0xff;
			bmpData [0x17] = SIZEY >> 8;
			bmpData [0x22] = (bmpSize     ) & 0xff;
			bmpData [0x23] = (bmpSize >> 8) & 0xff;
			bmpData [0x24] = (bmpSize >>16) & 0xff;
			bmpData [0x25] = (bmpSize >>24) & 0xff;
			bmpSize += 0x36;
			bmpData [0x02] = (bmpSize     ) & 0xff;
			bmpData [0x03] = (bmpSize >> 8) & 0xff;
			bmpData [0x04] = (bmpSize >>16) & 0xff;
			bmpData [0x05] = (bmpSize >>24) & 0xff;

			fwrite(bmpData, 1, bmpSize, F);
			fclose(F);
		} else
			MessageBox(hMainWnd, _T("File open failed!"), _T("Nintendulator"), MB_OK);
	}
}

void	Draw2x (int which) {
	int x, y;
	if (PPU::PPU[0] ==NULL) return;
	register unsigned short *src = PPU::PPU[which]->DrawArray +341*OFFSETY +OFFSETX;
	if (Depth == 32) {
		register unsigned long *dst;
		for (y = 0; y < SIZEY*2; y++) {
			dst = (unsigned long *)((unsigned char *)SurfDesc.lpSurface + y*Pitch);
			if (Settings::VSDualScreens ==2) dst +=which *512;
			if (Fullscreen) {
				for (x = 0; x < FullscreenBorderH; x++)	*dst++ = 0x000000;
			}
			if (Settings::Scanlines && y &1) {
				for (x = 0; x < SIZEX; x++) {
					*dst++ = 0x000000;
					*dst++ = 0x000000;
					src++;
				}
			} else {
				for (x = 0; x < SIZEX; x++) {
					*dst++ = Palette32[*src];
					*dst++ = Palette32[*src];
					src++;
				}
			}
			src +=341 -SIZEX;
			if (Fullscreen) {
				for (x = 0; x < FullscreenBorderH; x++)	*dst++ = 0x000000;
			}
			if (~y &1) src -=341;
		}
	} else if (Depth == 16) {
		register unsigned short *dst;
		for (y = 0; y < SIZEY*2; y++) {
			dst = (unsigned short *)((unsigned char *)SurfDesc.lpSurface + y*Pitch);
			if (Settings::VSDualScreens ==2) dst +=which *512;
			if (Fullscreen) {
				for (x = 0; x < FullscreenBorderH; x++)	*dst++ = 0x0000;
			}
			if (Settings::Scanlines && (y & 1)) {
				for (x = 0; x < SIZEX; x++) {
					*dst++ = 0x0000;
					*dst++ = 0x0000;
				}
				src += 341;
			} else {
				for (x = 0; x < SIZEX; x++) {
					*dst++ = Palette16[*src];
					*dst++ = Palette16[*src];
					src++;
				}
				src += 341 -SIZEX;
			}
			if (Fullscreen) {
				for (x = 0; x < FullscreenBorderH; x++)	*dst++ = 0x0000;
			}
			if (~y &1) src -= 341;
		}
	} else {
		register unsigned short *dst;
		for (y = 0; y < SIZEY; y++) {
			dst = (unsigned short *)((unsigned char *)SurfDesc.lpSurface + y*Pitch);
			if (Settings::VSDualScreens ==2) dst +=which *512;
			if (Fullscreen) {
				for (x = 0; x < FullscreenBorderH; x++)	*dst++ = 0x0000;
			}
			if (Settings::Scanlines && (y & 1)) {
				for (x = 0; x < SIZEX; x++) {
					*dst++ = 0x0000;
					*dst++ = 0x0000;
				}
				src +=SIZEX;
			} else {
				for (x = 0; x < SIZEX; x++) {
					*dst++ = Palette15[*src];
					*dst++ = Palette15[*src];
					src++;
				}
			}
			src += 341 -SIZEX;
			if (Fullscreen) {
				for (x = 0; x < FullscreenBorderH; x++)	*dst++ = 0x0000;
			}
			if (~y &1) src -=SIZEX;
		}
	}
}

void	Draw2xVertical (int which) {
	int x, y;
	if (PPU::PPU[0] ==NULL) return;
	register unsigned short *src = PPU::PPU[which]->DrawArray +341*OFFSETY +OFFSETX;
	if (Depth == 32) {
		register unsigned long *dst;
		src +=480;
		for (y = 0; y < SIZEY*2; y++) {
			src--;
			dst = (unsigned long *)((unsigned char *)SurfDesc.lpSurface + y*Pitch);
			if (Fullscreen) {
				for (x = 0; x < FullscreenBorderH; x++)	*dst++ = 0x000000;
			}
			if (Settings::Scanlines && y &1) {
				for (x = 0; x < SIZEX; x++) {
					*dst++ = 0x000000;
					*dst++ = 0x000000;
					src +=341;
				}
			} else {
				for (x = 0; x < SIZEX; x++) {
					*dst++ = Palette32[*src];
					*dst++ = Palette32[*src];
					src +=341;
				}
			}
			src -=341*SIZEX;
			if (Fullscreen) {
				for (x = 0; x < FullscreenBorderH; x++)	*dst++ = 0x000000;
			}
			if (~y &1) src++;
		}
	} else if (Depth == 16) {
		register unsigned short *dst;
		for (y = 0; y < SIZEY*2; y++) {
			dst = (unsigned short *)((unsigned char *)SurfDesc.lpSurface + y*Pitch);
			if (Fullscreen) {
				for (x = 0; x < FullscreenBorderH; x++)	*dst++ = 0x0000;
			}
			if (Settings::Scanlines && (y & 1)) {
				for (x = 0; x < SIZEX; x++) {
					*dst++ = 0x0000;
					*dst++ = 0x0000;
				}
				src += 341;
			} else {
				for (x = 0; x < SIZEX; x++) {
					*dst++ = Palette16[*src];
					*dst++ = Palette16[*src];
					src++;
				}
				src += 341 -SIZEX;
			}
			if (Fullscreen) {
				for (x = 0; x < FullscreenBorderH; x++)	*dst++ = 0x0000;
			}
			if (~y &1) src -= 341;
		}
	} else {
		register unsigned short *dst;
		for (y = 0; y < SIZEY*2; y++) {
			dst = (unsigned short *)((unsigned char *)SurfDesc.lpSurface + y*Pitch);
			if (Fullscreen) {
				for (x = 0; x < FullscreenBorderH; x++)	*dst++ = 0x0000;
			}
			if (Settings::Scanlines && (y & 1)) {
				for (x = 0; x < SIZEX; x++) {
					*dst++ = 0x0000;
					*dst++ = 0x0000;
				}
				src +=SIZEX;
			} else {
				for (x = 0; x < SIZEX; x++) {
					*dst++ = Palette15[*src];
					*dst++ = Palette15[*src];
					src++;
				}
			}
			src += 341 -SIZEX;
			if (Fullscreen) {
				for (x = 0; x < FullscreenBorderH; x++)	*dst++ = 0x0000;
			}
			if (~y &1) src -=SIZEX;
		}
	}
}

void	Draw2xVerticalFlip (int which) {
	int x, y;
	if (PPU::PPU[0] ==NULL) return;
	register unsigned short *src = PPU::PPU[which]->DrawArray +341*OFFSETY +OFFSETX;
	if (Depth == 32) {
		register unsigned long *dst;
		src +=223 +341*SIZEX;
		for (y = 0; y < SIZEY*2; y++) {
			src++;
			dst = (unsigned long *)((unsigned char *)SurfDesc.lpSurface + y*Pitch);
			if (Fullscreen) {
				for (x = 0; x < FullscreenBorderH; x++)	*dst++ = 0x000000;
			}
			if (Settings::Scanlines && y &1) {
				for (x = 0; x < SIZEX; x++) {
					*dst++ = 0x000000;
					*dst++ = 0x000000;
					src -=341;
				}
			} else {
				for (x = 0; x < SIZEX; x++) {
					*dst++ = Palette32[*src];
					*dst++ = Palette32[*src];
					src -=341;
				}
			}
			src +=341*SIZEX;
			if (Fullscreen) {
				for (x = 0; x < FullscreenBorderH; x++)	*dst++ = 0x000000;
			}
			if (~y &1) src--;
		}
	} else if (Depth == 16) {
		register unsigned short *dst;
		for (y = 0; y < SIZEY*2; y++) {
			dst = (unsigned short *)((unsigned char *)SurfDesc.lpSurface + y*Pitch);
			if (Fullscreen) {
				for (x = 0; x < FullscreenBorderH; x++)	*dst++ = 0x0000;
			}
			if (Settings::Scanlines && (y & 1)) {
				for (x = 0; x < SIZEX; x++) {
					*dst++ = 0x0000;
					*dst++ = 0x0000;
				}
				src += 341;
			} else {
				for (x = 0; x < SIZEX; x++) {
					*dst++ = Palette16[*src];
					*dst++ = Palette16[*src];
					src++;
				}
				src += 341 -SIZEX;
			}
			if (Fullscreen) {
				for (x = 0; x < FullscreenBorderH; x++)	*dst++ = 0x0000;
			}
			if (~y &1) src -= 341;
		}
	} else {
		register unsigned short *dst;
		for (y = 0; y < SIZEY*2; y++) {
			dst = (unsigned short *)((unsigned char *)SurfDesc.lpSurface + y*Pitch);
			if (Fullscreen) {
				for (x = 0; x < FullscreenBorderH; x++)	*dst++ = 0x0000;
			}
			if (Settings::Scanlines && (y & 1)) {
				for (x = 0; x < SIZEX; x++) {
					*dst++ = 0x0000;
					*dst++ = 0x0000;
				}
				src +=SIZEX;
			} else {
				for (x = 0; x < SIZEX; x++) {
					*dst++ = Palette15[*src];
					*dst++ = Palette15[*src];
					src++;
				}
			}
			src += 341 -SIZEX;
			if (Fullscreen) {
				for (x = 0; x < FullscreenBorderH; x++)	*dst++ = 0x0000;
			}
			if (~y &1) src -=SIZEX;
		}
	}
}

void	Draw1H2V (int which) {
	int x, y;
	if (PPU::PPU[0] ==NULL) return;
	register unsigned short *src = PPU::PPU[which]->DrawArray +341*OFFSETY +OFFSETX;
	if (Depth == 32) {
		register unsigned long *dst;
		for (y = 0; y < 480; y++) {
			dst = (unsigned long *)((unsigned char *)SurfDesc.lpSurface + y*Pitch);
			dst +=which *(SIZEX +FullscreenBorderH);
			if (Fullscreen && which ==0) {
				for (x = 0; x < FullscreenBorderH; x++)
					*dst++ = Palette32[PPU::PPU[which]->Palette[0]];
			}
			if (y <120 || y >=360) {
				for (x = 0; x < SIZEX; x++) {
					*dst++ = Palette32[PPU::PPU[which]->Palette[0]];
				}
			} else {
				for (x = 0; x < SIZEX; x++) {
					*dst++ = Palette32[*src];
					src++;
				}
			}
			if (Fullscreen && which ==1) {
				for (x = 0; x < FullscreenBorderH; x++)
					*dst++ = Palette32[PPU::PPU[which]->Palette[0]];
			}
		}
	} else if (Depth == 16) {
		register unsigned long *dst;
		for (y = 0; y < 480; y++) {
			dst = (unsigned long *)((unsigned char *)SurfDesc.lpSurface + y*Pitch);
			dst +=which *(SIZEX +FullscreenBorderH);
			if (Fullscreen && which ==0) {
				for (x = 0; x < FullscreenBorderH; x++)
					*dst++ = Palette16[PPU::PPU[which]->Palette[0]];
			}
			if (y <120 || y >=360) {
				for (x = 0; x < SIZEX; x++) {
					*dst++ = Palette16[PPU::PPU[which]->Palette[0]];
				}
			} else {
				for (x = 0; x < SIZEX; x++) {
					*dst++ = Palette16[*src];
					src++;
				}
			}
			if (Fullscreen && which ==1) {
				for (x = 0; x < FullscreenBorderH; x++)
					*dst++ = Palette16[PPU::PPU[which]->Palette[0]];
			}
		}
	} else {
		register unsigned long *dst;
		for (y = 0; y < 480; y++) {
			dst = (unsigned long *)((unsigned char *)SurfDesc.lpSurface + y*Pitch);
			dst +=which *(SIZEX +FullscreenBorderH);
			if (Fullscreen && which ==0) {
				for (x = 0; x < FullscreenBorderH; x++)
					*dst++ = Palette15[PPU::PPU[which]->Palette[0]];
			}
			if (y <120 || y >=360) {
				for (x = 0; x < SIZEX; x++) {
					*dst++ = Palette15[PPU::PPU[which]->Palette[0]];
				}
			} else {
				for (x = 0; x < SIZEX; x++) {
					*dst++ = Palette15[*src];
					src++;
				}
			}
			if (Fullscreen && which ==1) {
				for (x = 0; x < FullscreenBorderH; x++)
					*dst++ = Palette15[PPU::PPU[which]->Palette[0]];
			}
		}
	}
}

void	Draw1x (int which) {
	int x, y;
	if (PPU::PPU[0] ==NULL) return;
	register unsigned short *src = PPU::PPU[which]->DrawArray +341*OFFSETY +OFFSETX;
	if (Depth == 32) {
		register unsigned long *dst;
		for (y = 0; y < SIZEY; y++) {
			dst = (unsigned long *)((unsigned char *)SurfDesc.lpSurface + y*Pitch);
			dst +=which *SIZEX;
			for (x = 0; x < SIZEX; x++) *dst++ = Palette32[*src++];
			src +=341 -SIZEX;

		}
	} else
	if (Depth == 16) {
		register unsigned short *dst;
		for (y = 0; y < SIZEY; y++) 	{
			dst = (unsigned short *)((unsigned char *)SurfDesc.lpSurface + y*Pitch);
			dst +=which *SIZEX;
			for (x = 0; x < SIZEX; x++) *dst++ = Palette16[*src++];
			src +=341 -SIZEX;
		}
	} else {
		register unsigned short *dst;
		for (y = 0; y < SIZEY; y++) {
			dst = (unsigned short *)((unsigned char *)SurfDesc.lpSurface + y*Pitch);
			dst +=which *SIZEX;
			for (x = 0; x < SIZEX; x++) *dst++ = Palette15[*src++];
			src +=341 -SIZEX;
		}
	}
}

void	DrawHiRes (int which) {
	int x, y;
	if (PPU::PPU[0] ==NULL) return;

	uint16_t *srcEven =dynamic_cast<PPU::PPU_VT369*>(PPU::PPU[which])->DrawArrayEven +341*2*OFFSETY +2*OFFSETX;
	uint16_t *srcOdd  =dynamic_cast<PPU::PPU_VT369*>(PPU::PPU[which])->DrawArrayOdd  +341*2*OFFSETY +2*OFFSETX;
	if (Depth == 32) {
		for (y = 0; y <SIZEY; y++) {
			unsigned long *dstEven =(unsigned long *)((unsigned char *)SurfDesc.lpSurface +(y*2 +0)*Pitch);
			unsigned long *dstOdd  =(unsigned long *)((unsigned char *)SurfDesc.lpSurface +(y*2 +1)*Pitch);

			for (x =0; x <SIZEX; x++) {
				*dstEven++ =Palette32[*srcEven++];
				*dstEven++ =Palette32[*srcEven++];
				*dstOdd++  =Palette32[*srcOdd++];
				*dstOdd++  =Palette32[*srcOdd++];
			}
			srcEven +=341*2 -SIZEX*2;
			srcOdd  +=341*2 -SIZEX*2;
		}
	} /*else
	if (Depth == 16) {
		register unsigned short *dst;
		for (y = 0; y < SIZEY*2; y++) 	{
			dst = (unsigned short *)((unsigned char *)SurfDesc.lpSurface + y*Pitch);
			dst +=which *SIZEX*2;
			for (x = 0; x < SIZEX*2; x++) *dst++ = Palette16[*src++];
			src +=341*2 -SIZEX*2;
		}
	} else {
		register unsigned short *dst;
		for (y = 0; y < SIZEY*2; y++) {
			dst = (unsigned short *)((unsigned char *)SurfDesc.lpSurface + y*Pitch);
			dst +=which *SIZEX*2;
			for (x = 0; x < SIZEX*2; x++) *dst++ = Palette15[*src++];
			src +=341*2 -SIZEX*2;
		}
	}*/
}

void	Update (void) {
	if (RI.ROMType ==ROM_NSF) return;
	// ensure secondary surface exists
	if (!SecondarySurf) return;
	// if it got lost, try to restore it
	if (SecondarySurf->IsLost() == DDERR_SURFACELOST) {
		EI.DbgOut(L"Secondary surface lost!");
		SecondarySurf->Restore();
	}
	if (InError) return;
	Try(SecondarySurf->Lock(NULL, &SurfDesc, DDLOCK_WAIT | DDLOCK_NOSYSLOCK | DDLOCK_WRITEONLY | DDLOCK_SURFACEMEMORYPTR, NULL), _T("Failed to lock secondary surface"));

	#ifdef SHOW_COLOR_NUM
	if (PPU::PPU[0]) {
		POINT mousepos;
		GFX::GetCursorPos(&mousepos);
		if (mousepos.x >=0 && mousepos.x <SIZEX && mousepos.y >=0 && mousepos.y <SIZEY) {
			int color = PPU::PPU[0]->DrawArray[mousepos.y*341 +mousepos.x +16];
			if (color >=PALETTE_VT369) color -=PALETTE_VT369; else
			if (color >=PALETTE_VT32) color -=PALETTE_VT32; else
			if (color >=PALETTE_VT03)  color -=PALETTE_VT03;  else
			if (color >=PALETTE_VT01)  color -=PALETTE_VT01;
			EI.DbgOut(L"Color: %03X", color);
		}
	}
	#endif
	if (Fullscreen && CurrentCursor ==&CursorCross && PPU::PPU[0]) {
		// Draw cross
		POINT pos;
		GFX::GetCursorPos(&pos);
		for (int x =pos.x -4; x <=pos.x +4; x++) {
			if (x <0 || x >255) continue;
			PPU::PPU[0]->DrawArray[pos.y*341 +x +16] =0x30;
		}
		for (int y =pos.y -4; y <=pos.y +4; y++) {
			if (y <0 || y >239) continue;
			PPU::PPU[0]->DrawArray[y*341 +pos.x +16] =0x30;
		}
	}
	if (GFX::g_bSan2) GFX::CHINA_ER_SAN2_ShowFont();

	if (RI.ConsoleType ==CONSOLE_VS && RI.INES2_VSFlags ==VS_DUAL) {
		if (Fullscreen && Settings::VSDualScreens ==2) {
			Draw1H2V(0);
			Draw1H2V(1);
		} else {
			if (Settings::VSDualScreens !=1) {
				//if (Fullscreen || Settings::Scanlines)
					Draw2x(0);
				/*else
					Draw1x(0);*/
			}
			if (Settings::VSDualScreens !=0) {
				//if (Fullscreen || Settings::Scanlines)
					Draw2x(1);
				/*else
					Draw1x(1);*/
			}
		}
	} else {
		if (RI.ConsoleType ==CONSOLE_VT369 && reg2000[0x1C] &0x04)
			DrawHiRes(0);
		else
		if (verticalMode) {
			if (verticalFlip)
				Draw2xVerticalFlip(0);
			else
				Draw2xVertical(0);
		} else
			Draw2x(0);
	}

	Try(SecondarySurf->Unlock(NULL), _T("Failed to unlock secondary surface"));
	Repaint();
}


void	Repaint (void) {
	RepaintInProgress =TRUE;
	if (ScreenshotRequested) { // Only set if Running==TRUE
		ScreenshotRequested =FALSE;
		GFX::SaveScreenshot();
	}

	// ensure primary surface exists
	if (!PrimarySurf) {
		RepaintInProgress =FALSE;
		return;
	}
	// if it got lost, try to restore it
	if (PrimarySurf->IsLost() == DDERR_SURFACELOST) {
		EI.DbgOut(L"Primary surface lost!");
		PrimarySurf->Restore();
	}
	// just to be safe, make sure the secondary surface exists too - this is only called by Repaint
	if (!SecondarySurf) {
		RepaintInProgress =FALSE;
		return;
	}
	if (InError) {
		RepaintInProgress =FALSE;
		return;
	}

	if (Fullscreen) {
		HRESULT	result;
		TCHAR strResult [256];

		// can't use Try() here, because a failure will make it retry in Windowed mode, where Flip() isn't allowed
		result = PrimarySurf->Flip(NULL, DDFLIP_WAIT);
		if (SUCCEEDED(result)) {
			RepaintInProgress =FALSE;
			return;
		}
		_stprintf_s(strResult, 256, _T("Failed to flip to primary surface! %s %08X"), DirectDrawErrorCode(result), (int) PrimarySurf),
		RepaintInProgress =FALSE;
		Stop();
		MessageBox(hMainWnd, strResult, _T("Nintendulator"), MB_OK | MB_ICONWARNING);
		Fullscreen =FALSE;
		Start();
	}
	// translate window position appropriately
	RECT rect;
	POINT pt = {0, 0};
	GetClientRect(hMainWnd, &rect);
	if (rect.right ==0 || rect.bottom ==0) {
		RepaintInProgress =FALSE;
		return;
	}
	ClientToScreen(hMainWnd, &pt);
	OffsetRect(&rect, pt.x, pt.y);

	if (Settings::VSync && (!Controllers::capsLock || Controllers::scrollLock) && WantFPS ==60 && RI.ROMType !=ROM_NSF) DirectDraw->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, NULL);

	if (PrimarySurf) Try(PrimarySurf->Blt(&rect, SecondarySurf, NULL, DDBLT_WAIT, NULL), _T("Failed to blit to primary surface"));
	RepaintInProgress =FALSE;
}

void	GetCursorPos (POINT *pos) {
	::GetCursorPos(pos);
	if (Fullscreen)
	{
		pos->x -= FullscreenBorderH;
		pos->x /= 2;
		pos->y /= 2;
	}
	else
	{
		RECT rect;
		ScreenToClient(hMainWnd, pos);
		GetClientRect(hMainWnd, &rect);
		if (rect.left == rect.right)
			pos->x = 0;
		else	pos->x = pos->x * SIZEX / (rect.right - rect.left);
		if (rect.top == rect.bottom)
			pos->y = 0;
		else	pos->y = pos->y * SIZEY / (rect.bottom - rect.top);
	}

}

void	SetCursorPos (int x, int y) {
	POINT pos;
	pos.x = x;
	pos.y = y;
	if (Fullscreen)
	{
		pos.x *= 2;
		pos.y *= 2;
		pos.x += FullscreenBorderH;
	}
	else
	{
		RECT rect;
		GetClientRect(hMainWnd, &rect);
		pos.x = pos.x * (rect.right - rect.left) / SIZEX;
		pos.y = pos.y * (rect.bottom - rect.top) / SIZEY;
		ClientToScreen(hMainWnd, &pos);
	}
	::SetCursorPos(pos.x, pos.y);
}

int	ZapperHit (int color) {
	int val = 0;
	val += (int)(RawPalette[(color >> 6) & 0x7][color & 0x3F][0] * 0.299);
	val += (int)(RawPalette[(color >> 6) & 0x7][color & 0x3F][1] * 0.587);
	val += (int)(RawPalette[(color >> 6) & 0x7][color & 0x3F][2] * 0.114);
	return val;
}

int	getPhase (double *wave)
{
	double max = -999, min = 999;
	double amp, offset;
	double angle = 180, base;
	int i, j, k;
	for (i = 0; i < 12; i++)
	{
		if (wave[i] > max)
			max = wave[i];
		if (wave[i] < min)
			min = wave[i];
	}
	amp = (max - min) / 2;
	offset = (max + min) / 2;

	for (k = 0; k < 4; k++)
	{
		double error[12], curerror = 0;
		double segsize = 180;
		for (i = 0; i <= k; i++)
			segsize /= 6.0;

		for (j = 0; j < 12; j++)
		{
			error[j] = 0;
			for (i = 0; i < 12; i++)
				error[j] += fabs((amp * sin((i * 30 + (j - 6) * segsize + angle) * M_PI / 180.0) + offset) - wave[i]);
			curerror += error[j];
		}
		base = 0;
		for (j = 0; j < 12; j++)
		{
			if (error[j] < curerror)
			{
				base = (j - 6) * segsize;
				curerror = error[j];
			}
		}
		angle += base;
	}

	return (int)floor(angle + 0.5) -45;
}

void	getNESColor (uint8_t luma, uint8_t phase, uint8_t emphasis, double &Y, double &angle, double &amplitude) {
	const double levels[2][4] = {
		{ 0.391,0.673,1.000,1.000},
		{-0.109,0.000,0.309,0.727}
	};
	const char colorPhases[12][12] = {
		{1,1,1,1,1,1,0,0,0,0,0,0},
		{1,1,1,1,1,0,0,0,0,0,0,1},
		{1,1,1,1,0,0,0,0,0,0,1,1},
		{1,1,1,0,0,0,0,0,0,1,1,1},
		{1,1,0,0,0,0,0,0,1,1,1,1},
		{1,0,0,0,0,0,0,1,1,1,1,1},
		{0,0,0,0,0,0,1,1,1,1,1,1},
		{0,0,0,0,0,1,1,1,1,1,1,0},
		{0,0,0,0,1,1,1,1,1,1,0,0},
		{0,0,0,1,1,1,1,1,1,0,0,0},
		{0,0,1,1,1,1,1,1,0,0,0,0},
		{0,1,1,1,1,1,1,0,0,0,0,0},
	};
	const char emphasisPhases[8][12] = {
		{0,0,0,0,0,0,0,0,0,0,0,0},
		{0,1,1,1,1,1,1,0,0,0,0,0},
		{1,1,1,0,0,0,0,0,0,1,1,1},
		{1,1,1,1,1,1,1,0,0,1,1,1},
		{0,0,0,0,0,1,1,1,1,1,1,0},
		{0,1,1,1,1,1,1,1,1,1,1,0},
		{1,1,1,0,0,1,1,1,1,1,1,1},
		{1,1,1,1,1,1,1,1,1,1,1,1}
	};
	double wave[12];
	for (int i =0; i <12; i++) {
		switch (phase) {
			case 0:
				wave[i] =levels[0][luma];
				break;
			default:
				wave[i] =colorPhases[phase -1][i]? levels[0][luma] : levels[1][luma];
				break;
			case 13:
				wave[i] =levels[1][luma];
				break;
			case 14: case 15:
				wave[i] =0.0;
				break;
		}
		if (emphasisPhases[emphasis][i] && phase <14) wave[i] *=0.81;
	}

	Y =0.0;
	amplitude =0.0;
	for (int i =0; i <12; i++) Y +=wave[i] /12.0;
	for (int i =0; i <12; i++) amplitude +=(wave[i] -Y)*(wave[i] -Y);
	amplitude =sqrt(amplitude /12.0);
	angle =getPhase(wave);
}

void	GenerateYUV (float hue, float saturation, float differential, BOOL setup, BOOL tweakedAngles, double (*matrix)[3][3]) {
	for (unsigned char emphasisLevel =0; emphasisLevel <8; emphasisLevel++) {
		for (unsigned char lumaValue =0; lumaValue <4; lumaValue++) {
			for (unsigned char chromaValue =0; chromaValue <16; chromaValue++) {
				double Y =0.0;
				double C =0.0;
				double angle =0.0;

				if (RI.PRGROMCRC32 ==0x9BA777E1 && emphasisLevel ==7) {
					// "The Immortal" uses emphasis and color $0D as black to use color $0E as a darker gray than $2D.
					getNESColor(lumaValue, chromaValue, 0, Y, angle, C);
					if (chromaValue >=0xE) Y +=0.155f;
				} else
					getNESColor(lumaValue, chromaValue, emphasisLevel, Y, angle, C);

				if (chromaValue >=1 && chromaValue <=12) {
					// Analogue effects, in particular, differential phase distortion
					angle +=differential *Y;
					if (chromaValue ==0x6) angle -=differential /3.0;
					if (chromaValue ==0x7) angle -=differential /6.0;
					if (chromaValue ==0x8)    Y  *=1.0 + differential /-333.3;
					if (chromaValue ==0xB) angle -=differential /6.0;
					if (chromaValue ==0xC) angle -=differential *Y /3.0;
				}

				if (setup) Y =(Y -0.075) /0.925;
				if (tweakedAngles) {
					switch (chromaValue) {
						case 2:
							angle -=lumaValue ==2? 15: 13;
							break;
						case 4:
						case 11:
							break;
						case 5:
							if (lumaValue >=2) angle -=10.0;
							break;
						case 6:
							if (lumaValue <2) angle -=16.5;
							break;
						case 9:
						case 10:
							if (lumaValue >=2) angle -=13;
							break;
						case 12:
							angle -=7.5;
							break;
						default:
							angle -=13;
					}
				} else
					angle +=hue;

				C *=saturation;
				double V =C *sin(angle *M_PI /180.0);
				double U =C *cos(angle *M_PI /180.0);

				double r =Y +V*1.14;
				double b =Y +U*1.14*1.78;
				double g =(Y -0.30*r -0.11*b) /0.59;

				if (r <0.0) r =0.0;
				if (g <0.0) g =0.0;
				if (b <0.0) b =0.0;
				if (0) {
					double R = (r>=0.0812)? pow((r+0.099)/1.099, 1.0/0.45): r/4.500;
					double G = (g>=0.0812)? pow((g+0.099)/1.099, 1.0/0.45): g/4.500;
					double B = (b>=0.0812)? pow((b+0.099)/1.099, 1.0/0.45): b/4.500;
					double newR =R /pow((1.131700+0.099)/1.099, 1.0/0.45);
					double newG =G /pow((1.131700+0.099)/1.099, 1.0/0.45);
					double newB =B /pow((1.131700+0.099)/1.099, 1.0/0.45);
					if (newR<0.0) newR=0.0; if (newR>1.0) newR=1.0;
					if (newG<0.0) newG=0.0; if (newG>1.0) newG=1.0;
					if (newB<0.0) newB=0.0; if (newB>1.0) newB=1.0;
					r = (newR>=0.018)? 1.099*pow(newR, 0.45)-0.099: 4.5*newR;
					g = (newG>=0.018)? 1.099*pow(newG, 0.45)-0.099: 4.5*newG;
					b = (newB>=0.018)? 1.099*pow(newB, 0.45)-0.099: 4.5*newB;
				}
				if (r >1.0) r =1.0;
				if (g >1.0) g =1.0;
				if (b >1.0) b =1.0;

				if (matrix) {
					double R = (r>=0.0812)? pow((r+0.099)/1.099, 1.0/0.45): r/4.500;
					double G = (g>=0.0812)? pow((g+0.099)/1.099, 1.0/0.45): g/4.500;
					double B = (b>=0.0812)? pow((b+0.099)/1.099, 1.0/0.45): b/4.500;
					double newR =(*matrix)[0][0]*R +(*matrix)[0][1]*G +(*matrix)[0][2]*B;
					double newG =(*matrix)[1][0]*R +(*matrix)[1][1]*G +(*matrix)[1][2]*G;
					double newB =(*matrix)[2][0]*R +(*matrix)[2][1]*G +(*matrix)[2][2]*B;
					if (newR<0.0) newR=0.0; if (newR>1.0) newR=1.0;
					if (newG<0.0) newG=0.0; if (newG>1.0) newG=1.0;
					if (newB<0.0) newB=0.0; if (newB>1.0) newB=1.0;
					r = (newR>=0.018)? 1.099*pow(newR, 0.45)-0.099: 4.5*newR;
					g = (newG>=0.018)? 1.099*pow(newG, 0.45)-0.099: 4.5*newG;
					b = (newB>=0.018)? 1.099*pow(newB, 0.45)-0.099: 4.5*newB;
				}
				unsigned char colorIndex =lumaValue <<4 | chromaValue;
				RawPalette[emphasisLevel][colorIndex][0] =r *255.0;
				RawPalette[emphasisLevel][colorIndex][1] =g *255.0;
				RawPalette[emphasisLevel][colorIndex][2] =b *255.0;
			}
		}
	}
}

double NTSC1953_Rec709 [3][3] = {
	{ 1.4607, -0.3845, -0.0761 },
	{-0.0266,  0.9654,  0.0612 },
	{-0.0264, -0.0414,  1.0678 }
};

double EBU_Rec709 [3][3] = {
	{ 1.0440, -0.0440,  0.0000 },
	{ 0.0000,  1.0000,  0.0000 },
	{ 0.0000,  0.0118,  0.9882 }
};

double deMarsh1970sReceiverD93_Rec709 [3][3] = {
	{ 0.7621, 0.1309, 0.0000 },
	{ 0.0161, 0.9782, 0.0000 },
	{-0.0144,-0.0180, 1.4040 }
};

void	GenerateNTSC (void) {
	GenerateYUV(Settings::NTSCHue, Settings::NTSCSaturation /50.0, Settings::NTSCAxis, Settings::NTSCSetup, false, Settings::NTSCto709? &NTSC1953_Rec709: NULL);
}

void	GeneratePAL (void) {
	GenerateYUV(-15, Settings::PALSaturation /50.0, 0, false, Settings::PALTweakedAngles, NULL);
}

#include "GFX_RGBPAL.h"
void	LoadPaletteROM (const unsigned short PaletteROM[64], const unsigned char PaletteLUT[64]) {
	for (int emphasisLevel =0; emphasisLevel <8; emphasisLevel++) {
		for (int colorIndex =0; colorIndex <64; colorIndex++) {
			unsigned short entry =PaletteLUT? PaletteROM[PaletteLUT[colorIndex]]: PaletteROM[colorIndex];
			double r =(emphasisLevel &1? 7: (entry >>6 &7))/7.0;
			double g =(emphasisLevel &2? 7: (entry >>3 &7))/7.0;
			double b =(emphasisLevel &4? 7: (entry     &7))/7.0;
			if (Settings::RGBsRGB) {
				double R = (r>=0.0812)? pow((r+0.099)/1.099, 2.75): r/4.500;
				double G = (g>=0.0812)? pow((g+0.099)/1.099, 2.75): g/4.500;
				double B = (b>=0.0812)? pow((b+0.099)/1.099, 2.75): b/4.500;
				double newR, newG, newB;
				int realColorIndex =PaletteLUT? PaletteLUT[colorIndex]: colorIndex;
				if (r ==g && g ==b || realColorIndex ==0x02 || realColorIndex ==0x12) {
					newR =R;
					newG =G;
					newB =B;
				} else {
					newR =deMarsh1970sReceiverD93_Rec709[0][0]*R +deMarsh1970sReceiverD93_Rec709[0][1]*G +deMarsh1970sReceiverD93_Rec709[0][2]*B;
					newG =deMarsh1970sReceiverD93_Rec709[1][0]*R +deMarsh1970sReceiverD93_Rec709[1][1]*G +deMarsh1970sReceiverD93_Rec709[1][2]*G;
					newB =deMarsh1970sReceiverD93_Rec709[2][0]*R +deMarsh1970sReceiverD93_Rec709[2][1]*G +deMarsh1970sReceiverD93_Rec709[2][2]*B;
				}
				if (newR<0.0) newR=0.0; if (newR>1.0) newR=1.0;
				if (newG<0.0) newG=0.0; if (newG>1.0) newG=1.0;
				if (newB<0.0) newB=0.0; if (newB>1.0) newB=1.0;
				r = (newR>=0.018)? 1.099*pow(newR, 0.45)-0.099: 4.5*newR;
				g = (newG>=0.018)? 1.099*pow(newG, 0.45)-0.099: 4.5*newG;
				b = (newB>=0.018)? 1.099*pow(newB, 0.45)-0.099: 4.5*newB;
			}
			RawPalette[emphasisLevel][colorIndex][0] =r *255.0;
			RawPalette[emphasisLevel][colorIndex][1] =g *255.0;
			RawPalette[emphasisLevel][colorIndex][2] =b *255.0;
		}
	}
}

void	LoadStaticPalette (const unsigned char input[][64][3]) {
	for (int i = 0; i < 8; i++) memcpy(RawPalette[i], *input[i], sizeof(RawPalette[i]));
}

void	GenerateRGB () {
	switch(NES::WhichRGBPalette) {
		case 1:	LoadPaletteROM(PaletteROM_2C04, PaletteLUT_2C04_0001); break;
		case 2:	LoadPaletteROM(PaletteROM_2C04, PaletteLUT_2C04_0002); break;
		case 3:	LoadPaletteROM(PaletteROM_2C04, PaletteLUT_2C04_0003); break;
		case 4:	LoadPaletteROM(PaletteROM_2C04, PaletteLUT_2C04_0004); break;
		default:LoadPaletteROM(PaletteROM_2C03, NULL); break;
	}

}

BOOL	ImportPalette (const TCHAR *filename, BOOL load) {
	int i, j;
	FILE *pal;
	// If no filename specified, use configured custom palette for current region
	if (filename == NULL)
		filename = Settings::CustPalette[PaletteRegion];
	if (!_tcslen(filename))
		return FALSE;
	pal = _tfopen(filename, _T("rb"));
	if (!pal) return FALSE;
	fseek(pal, 0, SEEK_END);
	if (ftell(pal) < 0xC0) {	// too small
		fclose(pal);
		return FALSE;
	}
	if (!load) {
		fclose(pal);
		return TRUE;
	}
	if (ftell(pal) >= 0x600) {
		fseek(pal, 0, SEEK_SET);
		for (j = 0; j < 8; j++) {
			for (i = 0; i < 64; i++) {
				fread(&RawPalette[j][i][0], 1, 1, pal);
				fread(&RawPalette[j][i][1], 1, 1, pal);
				fread(&RawPalette[j][i][2], 1, 1, pal);
			}
		}
	} else {
		fseek(pal, 0, SEEK_SET);
		for (i = 0; i < 64; i++) {
			fread(&RawPalette[0][i][0], 1, 1, pal);
			fread(&RawPalette[0][i][1], 1, 1, pal);
			fread(&RawPalette[0][i][2], 1, 1, pal);
			for (j = 1; j < 8; j++) {
				int R =RawPalette[0][i][0];
				int G =RawPalette[0][i][1];
				int B =RawPalette[0][i][2];
				if (j ==7) {
					R = pow(R /255.0, 1.5) *255.0;
					G = pow(G /255.0, 1.5) *255.0;
					B = pow(B /255.0, 1.5) *255.0;
				} else {
					if (j &1) {
						G -=31;
						B -=31;
						if (G <0) G =0;
						if (B <0) B =0;
					}
					if (j &2) {
						R -=31;
						B -=31;
						if (R <0) R =0;
						if (B <0) B =0;
					}
					if (j &4) {
						R -=31;
						G -=31;
						if (R <0) R =0;
						if (G <0) G =0;
					}
				}
				RawPalette[j][i][0] =R;
				RawPalette[j][i][1] =G;
				RawPalette[j][i][2] =B;
			}
		}
	}
	fclose(pal);
	return TRUE;
}

void	LoadPalette (Settings::PALETTE PalNum) {
	unsigned int RV, GV, BV;
	int i;
	// If no palette number specified, use configured palette for current region
	if (PalNum ==Settings::PALETTE_MAX) {
		if (CurrentPalNum ==Settings::PALETTE_MAX)
			PalNum =Settings::Palette[PaletteRegion];
		else
			PalNum =CurrentPalNum;
	}
	CurrentPalNum =PalNum;

	if (PalNum == Settings::PALETTE_NTSC)
		GenerateNTSC();
	else if (PalNum == Settings::PALETTE_PAL)
		GeneratePAL();
	else if (PalNum == Settings::PALETTE_EXT) {
		if (!ImportPalette(NULL, TRUE)) {
			MessageBox(hMainWnd, _T("Unable to load the specified palette! Reverting to default!"), _T("Nintendulator"), MB_OK | MB_ICONERROR);
			LoadPalette(Settings::DefaultPalette[PaletteRegion]);
			return;
		}
	} else
		GenerateRGB();

	// Create 2C02 Palette
	for (i = 0; i < 0x200; i++) {
		RV = RawPalette[i >> 6][i & 0x3F][0];
		GV = RawPalette[i >> 6][i & 0x3F][1];
		BV = RawPalette[i >> 6][i & 0x3F][2];

		Palette15[i] = (unsigned short)(((RV << 7) & 0x7C00) | ((GV << 2) & 0x03E0) | (BV >> 3));
		Palette16[i] = (unsigned short)(((RV << 8) & 0xF800) | ((GV << 3) & 0x07E0) | (BV >> 3));
		Palette32[i] = (RV << 16) | (GV << 8) | BV;
	}

	// Create VT03 palette
	for (i =0; i <4096; i++) {
		int nPhase  = (i >>0) &0xF;
		int nLuma   = (i >>4) &0xF;
		int nChroma = (i >>8) &0xF;
		bool inverted = nLuma < (nChroma+1) >>1 || nLuma > 15 -(nChroma >>1);
		if (inverted) {
			// Strange color number wrap-around. Is this for protection reasons, or a bug of the original hardware?
			// The VT03 data sheet advises programmers that 4 <= nLuma*2 +nChroma <= 0x1F, which does not correspond exactly to this condition.
			//                                            0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15
			static const unsigned char altPhases[16] = { 13,  7,  8,  9, 10, 11, 12,  1,  2,  3,  4,  5,  6,  0, 14, 15};
			nPhase = altPhases[nPhase];
			nChroma = 16 -nChroma;
			nLuma = (nLuma -8) &0xF;
		}
		float fLuma   = nLuma /15.0;
		float fChroma = nChroma /30.0;
		float phaseOffset;

		if (Settings::VT03Palette ==2) {
			phaseOffset =-13.0;
			// Hybrid palette consisting of some NTSC and some PAL colors, with NTSC brightness.
			// Basically PAL hues except where NTSC looks better.
			if (nPhase ==0x0C
			|| i ==0x261 // 2D Escape: dark blue
			|| i ==0x3B6 // Boxing Wrestle: orange
			|| i ==0x3D6 // Plumber: orange
			|| i ==0x466 // Samuri: orange
			|| i ==0x486 // Curly Monkey: orange
			|| i ==0x4A6 // Boxing Wrestle: orange
			|| i ==0x4D6 && RI.INES2_SubMapper ==3 // Samuri: orange
			|| i ==0x522 // Curly Monkey 2: green
			|| i ==0x586 // Samuri: orange
			|| i ==0x588 // Champion Boat: dark green
			|| i ==0x5C6 // Samuri: orange
			|| i ==0x665 // Curly Monkey 2: red
			|| i ==0x6B6 // Samuri: orange
			|| i ==0x71C && RI.INES2_SubMapper ==3 // Z-Dog: Hummer face shadow
			|| i ==0x732 && RI.INES2_SubMapper ==3 // Samuri: sometimes green, sometimes yellow
			|| i ==0x73C // Tennis Ball: orange
			|| i ==0x82C // Curly Monkey: orange
			|| i ==0x846 // Samuri: orange
			|| i ==0x885 // Curly Monkey 2: red
			|| i ==0x8A6 // Samuri: orange
			|| i ==0x92C // Samuri: orange
			|| i ==0x94C // Curly Monkey 2: orange
			|| i ==0xA1C // Samuri: orange
			|| i ==0xA2C // Curly Monkey 2: orange
			|| i ==0xA65 // Curly Monkey 2: red
			|| i ==0xAA8 && RI.INES2_SubMapper ==3 // Samuri: green/yellow
			|| i ==0xB2C // Commando: orange
			|| i ==0xB3C // Penalty Kick: orange
			|| i ==0xB88 // Curly Monkey 2: green
			|| i ==0xB98 // Heroes Mice: green
			|| i ==0xC1C // Samuri: orange
			|| i ==0xC25 // Curly Monkey: blue-green
			|| i ==0xD0C // Samuri: orange
			|| i ==0xDEB // 121-in-1 menu: dark orange
			|| i ==0xE13 // Heroes Mice: green
			|| i ==0xEEC // Ghost ship: dark yellow
			) phaseOffset +=13.0;
			if (i ==0x5D7 // Thunder Man: bright skin
			) phaseOffset -=10.0;
		} else
			phaseOffset =Settings::VT03Palette ==0? 0.0: -10.0;

		float fPhase =(((nPhase -2) *30.0) +phaseOffset) *M_PI /180.0;
		float Y = fLuma;
		float C = fChroma;
		if (nPhase == 0 || nPhase >12) C =0.0;// Phases 0 and 13-15 are grays
		if (nPhase == 0) Y += fChroma;        // Phase 0 is the upper bound of the waveform
		if (nPhase ==13) Y -= fChroma;        // Phase 13 is the lower bound of the waveform
		if (nPhase >=14) Y = 0.0;             // Phases 14 and 15 always black

		float V = sin(fPhase) *C;
		float U = cos(fPhase) *C;
		float R = Y + 1.1400*V + 0.0000*U;
		float G = Y - 0.5807*V - 0.3940*U;
		float B = Y - 0.0000*V + 2.0290*U;
		R = (R -4.0/15.0) /(1.0 -4.0/15.0) *15.0/13.0;
		G = (G -4.0/15.0) /(1.0 -4.0/15.0) *15.0/13.0;
		B = (B -4.0/15.0) /(1.0 -4.0/15.0) *15.0/13.0;

		if (i ==0x0D0) R =G =B =1.0;
		if (Settings::VT03Palette !=1) {
			 R = (R -0.075) /0.925;
			 G = (G -0.075) /0.925;
			 B = (B -0.075) /0.925;
		}
		if (R <0.0) R =0.0; if (R >1.0) R =1.0;
		if (G <0.0) G =0.0; if (G >1.0) G =1.0;
		if (B <0.0) B =0.0; if (B >1.0) B =1.0;
		RV = R *255.0;
		GV = G *255.0;
		BV = B *255.0;
		Palette15[i +PALETTE_VT03] = (unsigned short)(((RV << 7) & 0x7C00) | ((GV << 2) & 0x03E0) | (BV >> 3));
		Palette16[i +PALETTE_VT03] = (unsigned short)(((RV << 8) & 0xF800) | ((GV << 3) & 0x07E0) | (BV >> 3));
		Palette32[i +PALETTE_VT03] = (RV << 16) | (GV << 8) | BV;
	}

	// Create VT01 LCD palette
	for (i =0; i <512; i++) {
		RV = ((i >>4) &3) *255 /3;
		GV = ((i >>2) &3) *255 /3;
		BV = ((i >>2) &3) *255 /3;
		//Assume Gamma of 1.5 instead of 2.22
		RV = pow(RV /255.0, 0.675) *255.0;
		GV = pow(GV /255.0, 0.675) *255.0;
		BV = pow(BV /255.0, 0.675) *255.0;
		Palette15[i +PALETTE_VT01] = (unsigned short)(((RV << 7) & 0x7C00) | ((GV << 2) & 0x03E0) | (BV >> 3));
		Palette16[i +PALETTE_VT01] = (unsigned short)(((RV << 8) & 0xF800) | ((GV << 3) & 0x07E0) | (BV >> 3));
		Palette32[i +PALETTE_VT01] = (RV << 16) | (GV << 8) | BV;
	}

	// Create VT32 (4-4-4) palette
	for (i =0; i <4096; i++) {
		RV = (i >> 0) &0x0F;
		if (Settings::VT32RemoveBlueishCast) { RV = RV *0xF/0xD; if (RV >0xF) RV =0xF; }
		GV = (i >> 4) &0x0F;
		BV = (i >> 8) &0x0F;
		RV = RV *0xFF /0x0F;
		GV = GV *0xFF /0x0F;
		BV = BV *0xFF /0x0F;

		Palette15[i +PALETTE_VT32] = (unsigned short)(((RV << 7) & 0x7C00) | ((GV << 2) & 0x03E0) | (BV >> 3));
		Palette16[i +PALETTE_VT32] = (unsigned short)(((RV << 8) & 0xF800) | ((GV << 3) & 0x07E0) | (BV >> 3));
		Palette32[i +PALETTE_VT32] = (RV << 16) | (GV << 8) | BV;
	}
	// Create VT369 (5-5-5) palette
	for (i =0; i <32768; i++) {
		RV = (i >>10) &0x1F;
		GV = (i >> 5) &0x1F;
		BV = (i >> 0) &0x1F;
		RV = RV *0xFF /0x1F;
		GV = GV *0xFF /0x1F;
		BV = BV *0xFF /0x1F;
		Palette15[i +PALETTE_VT369] = (unsigned short)(((RV << 7) & 0x7C00) | ((GV << 2) & 0x03E0) | (BV >> 3));
		Palette16[i +PALETTE_VT369] = (unsigned short)(((RV << 8) & 0xF800) | ((GV << 3) & 0x07E0) | (BV >> 3));
		Palette32[i +PALETTE_VT369] = (RV << 16) | (GV << 8) | BV;
	}
	// Create 5-6-5 palette
	for (i =0; i <65536; i++) {
		RV = (i >>11) &0x1F;
		GV = (i >> 5) &0x3F;
		BV = (i >> 0) &0x1F;
		RV = RV *0xFF /0x1F;
		GV = GV *0xFF /0x3F;
		BV = BV *0xFF /0x1F;
		Palette15[i +PALETTE_VT565] = (unsigned short)(((RV << 7) & 0x7C00) | ((GV << 2) & 0x03E0) | (BV >> 3));
		Palette16[i +PALETTE_VT565] = (unsigned short)(((RV << 8) & 0xF800) | ((GV << 3) & 0x07E0) | (BV >> 3));
		Palette32[i +PALETTE_VT565] = (RV << 16) | (GV << 8) | BV;
	}
	// redraw the screen with the new palette, but only if emulation isn't active
	if (!NES::Running) Update();
}
#undef CLIP

BOOL	inUpdate =FALSE;
BOOL	*sRGB;
int	*saturation;

void	UpdatePalette (HWND hDlg) {
	if (Settings::Palette[PaletteRegion] ==Settings::PALETTE_NTSC) {
		EnableWindow(GetDlgItem(hDlg, IDC_PAL_HUESLIDER), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_PAL_HUE), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_PAL_AXISSLIDER), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_PAL_AXIS), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_PAL_SATSLIDER), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_PAL_SAT), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_PAL_CONVERT_COLORSPACE), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_PAL_SETUP), TRUE);
		SetWindowText(GetDlgItem(hDlg, IDC_PAL_CONVERT_COLORSPACE), L"1953 NTSC to Rec.709 primaries");

		SendDlgItemMessage(hDlg, IDC_PAL_HUESLIDER, TBM_SETPOS, TRUE, Settings::NTSCHue);
		SendDlgItemMessage(hDlg, IDC_PAL_AXISSLIDER, TBM_SETPOS, TRUE, Settings::NTSCAxis);
		SendDlgItemMessage(hDlg, IDC_PAL_SATSLIDER, TBM_SETPOS, TRUE, Settings::NTSCSaturation);
		SetDlgItemInt(hDlg, IDC_PAL_HUE, Settings::NTSCHue, TRUE);
		SetDlgItemInt(hDlg, IDC_PAL_AXIS, Settings::NTSCAxis, TRUE);
		SetDlgItemInt(hDlg, IDC_PAL_SAT, Settings::NTSCSaturation, FALSE);
		CheckDlgButton(hDlg, IDC_PAL_CONVERT_COLORSPACE, Settings::NTSCto709? BST_CHECKED: BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_PAL_SETUP, Settings::NTSCSetup? BST_CHECKED: BST_UNCHECKED);
		GenerateNTSC();
	} else
	if (Settings::Palette[PaletteRegion] ==Settings::PALETTE_PAL) {
		EnableWindow(GetDlgItem(hDlg, IDC_PAL_HUESLIDER), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_PAL_HUE), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_PAL_AXISSLIDER), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_PAL_AXIS), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_PAL_SATSLIDER), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_PAL_SAT), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_PAL_CONVERT_COLORSPACE), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_PAL_SETUP), FALSE);
		SetWindowText(GetDlgItem(hDlg, IDC_PAL_CONVERT_COLORSPACE), L"Tweaked Angles");

		SendDlgItemMessage(hDlg, IDC_PAL_HUESLIDER, TBM_SETPOS, TRUE, -15);
		SendDlgItemMessage(hDlg, IDC_PAL_AXISSLIDER, TBM_SETPOS, TRUE, 0);
		SendDlgItemMessage(hDlg, IDC_PAL_SATSLIDER, TBM_SETPOS, TRUE, Settings::PALSaturation);
		SetDlgItemInt(hDlg, IDC_PAL_HUE, (UINT) -15, TRUE);
		SetDlgItemInt(hDlg, IDC_PAL_AXIS, (UINT) 0, TRUE);
		SetDlgItemInt(hDlg, IDC_PAL_SAT, Settings::PALSaturation, FALSE);
		CheckDlgButton(hDlg, IDC_PAL_CONVERT_COLORSPACE, Settings::PALTweakedAngles? BST_CHECKED: BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_PAL_SETUP, BST_UNCHECKED);
		GeneratePAL();
	} else {
		EnableWindow(GetDlgItem(hDlg, IDC_PAL_HUESLIDER), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_PAL_HUE), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_PAL_AXISSLIDER), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_PAL_AXIS), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_PAL_SATSLIDER), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_PAL_SAT), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_PAL_SETUP), FALSE);
		CheckDlgButton(hDlg, IDC_PAL_SETUP, BST_UNCHECKED);
		if (Settings::Palette[PaletteRegion] ==Settings::PALETTE_RGB) {
			EnableWindow(GetDlgItem(hDlg, IDC_PAL_CONVERT_COLORSPACE), TRUE);
			SetWindowText(GetDlgItem(hDlg, IDC_PAL_CONVERT_COLORSPACE), L"1980s' TV phosphors");
			CheckDlgButton(hDlg, IDC_PAL_CONVERT_COLORSPACE, Settings::RGBsRGB? BST_CHECKED: BST_UNCHECKED);
		} else {
			EnableWindow(GetDlgItem(hDlg, IDC_PAL_CONVERT_COLORSPACE), TRUE);
			SetWindowText(GetDlgItem(hDlg, IDC_PAL_CONVERT_COLORSPACE), L"Tweaked Angles");
			CheckDlgButton(hDlg, IDC_PAL_CONVERT_COLORSPACE, Settings::PALTweakedAngles? BST_CHECKED: BST_UNCHECKED);
		}

		SendDlgItemMessage(hDlg, IDC_PAL_HUESLIDER, TBM_SETPOS, TRUE, 0);
		SetDlgItemText(hDlg, IDC_PAL_HUE, _T("N/A"));
		SendDlgItemMessage(hDlg, IDC_PAL_AXISSLIDER, TBM_SETPOS, TRUE, 0);
		SetDlgItemText(hDlg, IDC_PAL_AXIS, _T("N/A"));
		SendDlgItemMessage(hDlg, IDC_PAL_SATSLIDER, TBM_SETPOS, TRUE, 50);
		SetDlgItemText(hDlg, IDC_PAL_SAT, _T("N/A"));

		GenerateRGB();
	}

	if (PaletteRegion !=Settings::PALREGION_VS && ImportPalette(Settings::CustPalette[PaletteRegion], FALSE)) {
		EnableWindow(GetDlgItem(hDlg, IDC_PAL_EXT), TRUE);
		if (Settings::Palette[PaletteRegion] ==Settings::PALETTE_EXT) ImportPalette(Settings::CustPalette[PaletteRegion], TRUE);
	} else
		EnableWindow(GetDlgItem(hDlg, IDC_PAL_EXT), FALSE);

	RedrawWindow(hDlg, NULL, NULL, RDW_INVALIDATE);
}

INT_PTR	CALLBACK	PaletteConfigProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	const int paltable[Settings::PALETTE_MAX] = {IDC_PAL_NTSC,IDC_PAL_PAL,IDC_PAL_RGB,IDC_PAL_EXT};
	const int PalEntries[64] = {
		IDC_PAL_00,IDC_PAL_01,IDC_PAL_02,IDC_PAL_03,IDC_PAL_04,IDC_PAL_05,IDC_PAL_06,IDC_PAL_07,IDC_PAL_08,IDC_PAL_09,IDC_PAL_0A,IDC_PAL_0B,IDC_PAL_0C,IDC_PAL_0D,IDC_PAL_0E,IDC_PAL_0F,
		IDC_PAL_10,IDC_PAL_11,IDC_PAL_12,IDC_PAL_13,IDC_PAL_14,IDC_PAL_15,IDC_PAL_16,IDC_PAL_17,IDC_PAL_18,IDC_PAL_19,IDC_PAL_1A,IDC_PAL_1B,IDC_PAL_1C,IDC_PAL_1D,IDC_PAL_1E,IDC_PAL_1F,
		IDC_PAL_20,IDC_PAL_21,IDC_PAL_22,IDC_PAL_23,IDC_PAL_24,IDC_PAL_25,IDC_PAL_26,IDC_PAL_27,IDC_PAL_28,IDC_PAL_29,IDC_PAL_2A,IDC_PAL_2B,IDC_PAL_2C,IDC_PAL_2D,IDC_PAL_2E,IDC_PAL_2F,
		IDC_PAL_30,IDC_PAL_31,IDC_PAL_32,IDC_PAL_33,IDC_PAL_34,IDC_PAL_35,IDC_PAL_36,IDC_PAL_37,IDC_PAL_38,IDC_PAL_39,IDC_PAL_3A,IDC_PAL_3B,IDC_PAL_3C,IDC_PAL_3D,IDC_PAL_3E,IDC_PAL_3F
	};

	int wmId, wmEvent;
	OPENFILENAME ofn;
	PAINTSTRUCT ps;
	HDC hdc;
	int i;

	switch (uMsg) {
	case WM_INITDIALOG:
		if (PaletteRegion ==Settings::PALREGION_VS) {
			// Force Vs. games to RGB palette and disable buttons selecting otherwise
			Settings::Palette[PaletteRegion] =Settings::PALETTE_RGB;
			EnableWindow(GetDlgItem(hDlg, IDC_PAL_NTSC), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_PAL_PAL), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_PAL_EXT), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_PAL_EXTFILE), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_PAL_BROWSE), FALSE);
		}
		// Save previous palette settings, so they all can be restored if they user clicks Cancel
		static BOOL  prevNTSCto709       =Settings::NTSCto709;
		static BOOL  prevPALTweakedAngles    =Settings::PALTweakedAngles;
		static int   prevNTSCHue        =Settings::NTSCHue;
		static int   prevNTSCSaturation =Settings::NTSCSaturation;
		static int   prevPALSaturation  =Settings::PALSaturation;
		static BOOL  prevRGBsRGB        =Settings::RGBsRGB;
		static BOOL  prevNTSCSetup      =Settings::NTSCSetup;
		static TCHAR prevCustPalette[Settings::PALREGION_MAX][MAX_PATH];
		static Settings::PALETTE prevPalette[Settings::PALREGION_MAX];
		for (int r =0; r <Settings::PALREGION_MAX; r++) {
			_tcscpy(prevCustPalette[r], Settings::CustPalette[r]);
			prevPalette[r] =Settings::Palette[r];
		}

		inUpdate =TRUE;
		switch(Settings::Palette[PaletteRegion]) {
			case Settings::PALETTE_NTSC:
				sRGB =&Settings::NTSCto709;
				saturation =&Settings::NTSCSaturation;
				break;
			case Settings::PALETTE_PAL:
				sRGB =&Settings::PALTweakedAngles;
				saturation =&Settings::PALSaturation;
				break;
			case Settings::PALETTE_EXT:
				sRGB =NULL;
				saturation =NULL;
				if (!ImportPalette(Settings::CustPalette[PaletteRegion], TRUE)) {
					MessageBox(hMainWnd, _T("Unable to load the specified palette! Reverting to default!"), _T("Nintendulator"), MB_OK | MB_ICONERROR);
					Settings::Palette[PaletteRegion] =Settings::DefaultPalette[PaletteRegion];
				}
				break;
			default:
				sRGB =&Settings::RGBsRGB;
				saturation =NULL;
				break;
		}
		SendDlgItemMessage(hDlg, IDC_PAL_HUESLIDER, TBM_SETRANGE, FALSE, MAKELONG(-30, 30));
		SendDlgItemMessage(hDlg, IDC_PAL_HUESLIDER, TBM_SETTICFREQ, 5, 0);
		SendDlgItemMessage(hDlg, IDC_PAL_AXISSLIDER, TBM_SETRANGE, FALSE, MAKELONG(-30, 30));
		SendDlgItemMessage(hDlg, IDC_PAL_AXISSLIDER, TBM_SETTICFREQ, 5, 0);
		SendDlgItemMessage(hDlg, IDC_PAL_SATSLIDER, TBM_SETRANGE, FALSE, MAKELONG(0, 100));
		SendDlgItemMessage(hDlg, IDC_PAL_SATSLIDER, TBM_SETTICFREQ, 5, 0);
		SetDlgItemText(hDlg, IDC_PAL_EXTFILE, Settings::CustPalette[PaletteRegion]);
		CheckRadioButton(hDlg, paltable[0], paltable[Settings::PALETTE_MAX-1], paltable[Settings::Palette[PaletteRegion]]);
		CheckDlgButton(hDlg, IDC_PAL_CONVERT_COLORSPACE, (sRGB && *sRGB)? BST_CHECKED: BST_UNCHECKED);
		UpdatePalette(hDlg);
		switch(PaletteRegion) {
			case Settings::PALREGION_NTSC : SetWindowText(hDlg, _T("Palette Configuration for NTSC region"       )); break;
			case Settings::PALREGION_PAL  : SetWindowText(hDlg, _T("Palette Configuration for PAL region"        )); break;
			case Settings::PALREGION_DENDY: SetWindowText(hDlg, _T("Palette Configuration for Dendy region"      )); break;
			case Settings::PALREGION_VS   : SetWindowText(hDlg, _T("Palette Configuration for Vs. System"        )); break;
			case Settings::PALREGION_PC10 : SetWindowText(hDlg, _T("Palette Configuration for Playchoice 10"     )); break;
			default                       : SetWindowText(hDlg, _T("Palette Configuration for unspecified region")); break;
		}
		inUpdate = FALSE;
		return TRUE;
	case WM_COMMAND:
		if (inUpdate) break;
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		switch (wmId) {
		case IDC_PAL_CONVERT_COLORSPACE:
			if (sRGB) {
				*sRGB =IsDlgButtonChecked(hDlg, IDC_PAL_CONVERT_COLORSPACE) ==BST_CHECKED;
				UpdatePalette(hDlg);
			}
			return TRUE;
		case IDC_PAL_SETUP:
			Settings::NTSCSetup =IsDlgButtonChecked(hDlg, IDC_PAL_SETUP) ==BST_CHECKED;
			UpdatePalette(hDlg);
			return TRUE;
		case IDC_PAL_NTSC:
			sRGB =&Settings::NTSCto709;
			saturation =&Settings::NTSCSaturation;
			Settings::Palette[PaletteRegion] =Settings::PALETTE_NTSC;
			UpdatePalette(hDlg);
			return TRUE;
		case IDC_PAL_PAL:
			sRGB =&Settings::PALTweakedAngles;
			saturation =&Settings::PALSaturation;
			Settings::Palette[PaletteRegion] =Settings::PALETTE_PAL;
			UpdatePalette(hDlg);
			return TRUE;
		case IDC_PAL_RGB:
			sRGB =&Settings::RGBsRGB;
			saturation =NULL;
			Settings::Palette[PaletteRegion] =Settings::PALETTE_RGB;
			UpdatePalette(hDlg);
			return TRUE;
		case IDC_PAL_EXTFILE:
			GetDlgItemText(hDlg, IDC_PAL_EXTFILE, Settings::CustPalette[PaletteRegion], MAX_PATH);
			UpdatePalette(hDlg);
			return TRUE;
		case IDC_PAL_EXT:
			if (ImportPalette(Settings::CustPalette[PaletteRegion], TRUE)) {
				sRGB =NULL;
				saturation =NULL;
				Settings::Palette[PaletteRegion] =Settings::PALETTE_EXT;
				UpdatePalette(hDlg);
				return TRUE;
			}
			break;
		case IDC_PAL_BROWSE:
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hDlg;
			ofn.hInstance = hInst;
			ofn.lpstrFilter = _T("Palette file (*.PAL)\0") _T("*.PAL\0") _T("\0");
			ofn.lpstrCustomFilter = NULL;
			ofn.nFilterIndex = 1;
			ofn.lpstrFile = Settings::CustPalette[PaletteRegion];
			ofn.nMaxFile = MAX_PATH;
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;
			ofn.lpstrInitialDir = Settings::Path_PAL;
			ofn.Flags = OFN_FILEMUSTEXIST;
			ofn.lpstrDefExt = NULL;
			ofn.lCustData = 0;
			ofn.lpfnHook = NULL;
			ofn.lpTemplateName = NULL;
			if (GetOpenFileName(&ofn)) {
				_tcscpy(Settings::Path_PAL, Settings::CustPalette[PaletteRegion]);
				Settings::Path_PAL[ofn.nFileOffset-1] = 0;
				if (ImportPalette(Settings::CustPalette[PaletteRegion], TRUE)) {
					Settings::Palette[PaletteRegion] = Settings::PALETTE_EXT;
					CheckRadioButton(hDlg, paltable[0], paltable[Settings::PALETTE_MAX-1], paltable[Settings::Palette[PaletteRegion]]);
					SetDlgItemText(hDlg, IDC_PAL_EXTFILE, Settings::CustPalette[PaletteRegion]);
					UpdatePalette(hDlg);
				} else
					MessageBox(hDlg, _T("Selected file is not a valid palette!"), _T("Nintendulator"), MB_OK | MB_ICONERROR);
			}
			return TRUE;
		case IDC_PAL_ER:
		case IDC_PAL_EG:
		case IDC_PAL_EB:
			UpdatePalette(hDlg);
			return TRUE;
		case IDC_PAL_EXPORT: {
			TCHAR FileName[MAX_PATH];
			FileName[0] ='\0';
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hDlg;
			ofn.hInstance = hInst;
			ofn.lpstrFilter = _T("Palette file (*.PAL)\0") _T("*.PAL\0") _T("\0");
			ofn.lpstrCustomFilter = NULL;
			ofn.nFilterIndex = 1;
			ofn.lpstrFile = FileName;
			ofn.nMaxFile = MAX_PATH;
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;
			ofn.lpstrInitialDir = Settings::Path_PAL;
			ofn.Flags = OFN_OVERWRITEPROMPT;
			ofn.lpstrDefExt = NULL;
			ofn.lCustData = 0;
			ofn.lpfnHook = NULL;
			ofn.lpTemplateName = NULL;
			if (GetSaveFileName(&ofn)) {
				FILE *handle =_tfopen(FileName, _T("wb"));
				if (handle) {
					fwrite(&RawPalette[0], 3, 64, handle);
					fclose(handle);
				}
			}
			return TRUE;
		}
		case IDOK:
			LoadPalette(Settings::Palette[PaletteRegion]);
			EndDialog(hDlg, 0);
			return TRUE;
		case IDCANCEL:
			// Restore previous palette settings
			Settings::NTSCto709       =prevNTSCto709;
			Settings::PALTweakedAngles    =prevPALTweakedAngles;
			Settings::NTSCHue        =prevNTSCHue;
			Settings::NTSCSaturation =prevNTSCSaturation;
			Settings::PALSaturation  =prevPALSaturation;
			Settings::RGBsRGB        =prevRGBsRGB;
			Settings::NTSCSetup      =prevNTSCSetup;
			for (int r =0; r <Settings::PALREGION_MAX; r++) {
				_tcscpy(Settings::CustPalette[r], prevCustPalette[r]);
				Settings::Palette[r] =prevPalette[r];
			}
			LoadPalette(Settings::PALETTE_MAX);
			EndDialog(hDlg, 0);
			return TRUE;
		}
		break;
	case WM_HSCROLL:
		if (lParam == (LPARAM)GetDlgItem(hDlg, IDC_PAL_HUESLIDER)) {
			Settings::NTSCHue =SendDlgItemMessage(hDlg, IDC_PAL_HUESLIDER, TBM_GETPOS, 0, 0);
			SetDlgItemInt(hDlg, IDC_PAL_HUE, Settings::NTSCHue, TRUE);
			UpdatePalette(hDlg);
			return TRUE;
		}
		if (lParam == (LPARAM)GetDlgItem(hDlg, IDC_PAL_AXISSLIDER)) {
			Settings::NTSCAxis =SendDlgItemMessage(hDlg, IDC_PAL_AXISSLIDER, TBM_GETPOS, 0, 0);
			SetDlgItemInt(hDlg, IDC_PAL_AXIS, Settings::NTSCAxis, TRUE);
			UpdatePalette(hDlg);
			return TRUE;
		}
		if (lParam == (LPARAM)GetDlgItem(hDlg, IDC_PAL_SATSLIDER) && saturation) {
			*saturation =SendDlgItemMessage(hDlg, IDC_PAL_SATSLIDER, TBM_GETPOS, 0, 0);
			SetDlgItemInt(hDlg, IDC_PAL_SAT, *saturation, FALSE);
			UpdatePalette(hDlg);
			return TRUE;
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hDlg, &ps);
		{
			HDC compdc = CreateCompatibleDC(hdc);
			HBITMAP bmp;
			POINT wcl = {0, 0};
			RECT wrect, rect;
			ClientToScreen(hDlg, &wcl);
			GetWindowRect(GetDlgItem(hDlg, PalEntries[0]), &rect);
			wrect.top = rect.top - wcl.y;
			wrect.left = rect.left - wcl.x;
			GetWindowRect(GetDlgItem(hDlg, PalEntries[63]), &rect);
			wrect.bottom = rect.bottom - wcl.y;
			wrect.right = rect.right - wcl.x;
			bmp = CreateCompatibleBitmap(hdc, wrect.right - wrect.left, wrect.bottom - wrect.top);
			SelectObject(compdc, bmp);
			for (i = 0; i < 64; i++) {
				HWND dlgitem = GetDlgItem(hDlg, PalEntries[i]);
				HBRUSH brush;
				unsigned char emp =
					((IsDlgButtonChecked(hDlg, IDC_PAL_ER) == BST_CHECKED) ? 0x1 : 0x0) |
					((IsDlgButtonChecked(hDlg, IDC_PAL_EG) == BST_CHECKED) ? 0x2 : 0x0) |
					((IsDlgButtonChecked(hDlg, IDC_PAL_EB) == BST_CHECKED) ? 0x4 : 0x0);
				unsigned int R = RawPalette[emp][i][0], G = RawPalette[emp][i][1], B = RawPalette[emp][i][2];
				brush = CreateSolidBrush(RGB(R, G, B));
				GetWindowRect(dlgitem, &rect);
				rect.top -= wcl.y + wrect.top;
				rect.bottom -= wcl.y + wrect.top;
				rect.left -= wcl.x + wrect.left;
				rect.right -= wcl.x + wrect.left;
				FillRect(compdc, &rect, brush);
				DeleteObject(brush);
			}
			BitBlt(hdc, wrect.left, wrect.top, wrect.right - wrect.left, wrect.bottom - wrect.top, compdc, 0, 0, SRCCOPY);
			DeleteDC(compdc);
			DeleteObject(bmp);
		}
		EndPaint(hDlg, &ps);
		return TRUE;
	}
	return FALSE;
}

void	PaletteConfig (void) {
	DialogBox(hInst, MAKEINTRESOURCE(IDD_PALETTE), hMainWnd, PaletteConfigProc);
}

INT_PTR	CALLBACK apertureDialogProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	TCHAR apertureTop[16];
	TCHAR apertureLeft[16];
	TCHAR apertureRight[16];
	TCHAR apertureBottom[16];
	TCHAR* endptr;
	int newYstart;
	int newXstart;
	int newXend;
	int newYend;

	switch (uMsg) {
	case WM_INITDIALOG:
		_stprintf_s(apertureTop,    16, _T("%d"), Settings::Ystart);
		_stprintf_s(apertureLeft,   16, _T("%d"), Settings::Xstart);
		_stprintf_s(apertureRight,  16, _T("%d"), Settings::Xend);
		_stprintf_s(apertureBottom, 16, _T("%d"), Settings::Yend);
		SendDlgItemMessage(hDlg, IDC_APERTURE_TOP,    WM_SETTEXT, 0, (LPARAM) apertureTop   );
		SendDlgItemMessage(hDlg, IDC_APERTURE_LEFT,   WM_SETTEXT, 0, (LPARAM) apertureLeft  );
		SendDlgItemMessage(hDlg, IDC_APERTURE_RIGHT,  WM_SETTEXT, 0, (LPARAM) apertureRight );
		SendDlgItemMessage(hDlg, IDC_APERTURE_BOTTOM, WM_SETTEXT, 0, (LPARAM) apertureBottom);
		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDOK:
			SendDlgItemMessage(hDlg, IDC_APERTURE_TOP,    WM_GETTEXT, 16, (LPARAM) apertureTop   );
			SendDlgItemMessage(hDlg, IDC_APERTURE_LEFT,   WM_GETTEXT, 16, (LPARAM) apertureLeft  );
			SendDlgItemMessage(hDlg, IDC_APERTURE_RIGHT,  WM_GETTEXT, 16, (LPARAM) apertureRight );
			SendDlgItemMessage(hDlg, IDC_APERTURE_BOTTOM, WM_GETTEXT, 16, (LPARAM) apertureBottom);
			newYstart =_tcstol(apertureTop,    &endptr, 0); if (*endptr !='\0') { MessageBox(hMainWnd, _T("Invalid character in top value field"),    _T("Aperture"), MB_OK | MB_ICONERROR); return TRUE; }
			newXstart =_tcstol(apertureLeft,   &endptr, 0); if (*endptr !='\0') { MessageBox(hMainWnd, _T("Invalid character in left value field"),   _T("Aperture"), MB_OK | MB_ICONERROR); return TRUE; }
			newXend   =_tcstol(apertureRight,  &endptr, 0); if (*endptr !='\0') { MessageBox(hMainWnd, _T("Invalid character in right value field"),  _T("Aperture"), MB_OK | MB_ICONERROR); return TRUE; }
			newYend   =_tcstol(apertureBottom, &endptr, 0); if (*endptr !='\0') { MessageBox(hMainWnd, _T("Invalid character in bottom value field"), _T("Aperture"), MB_OK | MB_ICONERROR); return TRUE; }
			if (Settings::Ystart !=newYstart ||
			    Settings::Xstart !=newXstart ||
			    Settings::Yend   !=newYend   ||
			    Settings::Xend   !=newXend) {
				Settings::Ystart =newYstart;
				Settings::Xstart =newXstart;
				Settings::Xend   =newXend;
				Settings::Yend   =newYend;
				if (!Fullscreen) {
					BOOL running = NES::Running;
					NES::Stop();
					GFX::Stop();
					GFX::Start();
					UpdateInterface(); // Must come after GFX::Start()
					if (running) NES::Start(FALSE);
				}
			}
			EndDialog(hDlg, 0);
			return TRUE;
		case IDCANCEL:
			EndDialog(hDlg, 0);
			return TRUE;
		}
	}
	return FALSE;
}

void	apertureDialog (void) {
	DialogBox(hInst, MAKEINTRESOURCE(IDD_APERTURE), hMainWnd, apertureDialogProc);
}

#include "CHINA_ER_SAN2.h"
} // namespace GFX

