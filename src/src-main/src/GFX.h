#pragma once

#define PALETTE_NES 0
#define PALETTE_VT01 1024
#define PALETTE_VT03 1536
#define PALETTE_VT32 5632
#define PALETTE_VT369 9728
#define PALETTE_VT565 42496
#define PALETTE_END  108032
#define	DIRECTDRAW_VERSION 0x0700
#include <ddraw.h>

namespace GFX {
extern bool apertureChanged;
extern bool smallLCDMode;
extern bool verticalMode;
extern bool verticalFlip;
extern int SIZEX;
extern int SIZEY;
extern uint32_t frameCount;
extern unsigned char RawPalette[8][64][3];
extern unsigned long Palette32[PALETTE_END];
extern BOOL Fullscreen;
extern BOOL ScreenshotRequested;

extern int FPSnum, FPSCnt;

extern int WantFPS;

extern BOOL SlowDown, RepaintInProgress;
extern int SlowRate;

extern LPDIRECTDRAW7 DirectDraw;

void	Init (void);
void	Destroy (void);
void	SetRegion (void);
void	Start (void);
void	Stop (void);
void	SaveSettings (HKEY);
void	LoadSettings (HKEY);
void	DrawScreen (void);
void	SaveScreenshot (void);
void	Draw1x (int);
void	Draw2x (int);
void	Update (void);
void	Repaint (void);
void	LoadPalette (Settings::PALETTE);
void	SetFrameskip (int);
void	PaletteConfig (void);
void	GetCursorPos (POINT *);
void	SetCursorPos (int, int);
int	ZapperHit (int);
void	apertureDialog (void);

extern BOOL g_bSan2;
extern unsigned char pSan2Font[64*1024];//定义64K的字库空间 Defining a 64K font space
extern unsigned char CPU_BACKUP[256];
void CHINA_ER_SAN2_ShowFont(void);
} // namespace GFX
