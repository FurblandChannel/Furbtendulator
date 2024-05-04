#pragma once

namespace Settings {
enum Region { REGION_NONE, REGION_NTSC, REGION_PAL, REGION_DENDY, REGION_MAX };
enum PaletteRegion { PALREGION_NONE, PALREGION_NTSC, PALREGION_PAL, PALREGION_DENDY, PALREGION_VS, PALREGION_PC10, PALREGION_MAX };
enum PALETTE { PALETTE_NTSC, PALETTE_PAL, PALETTE_RGB, PALETTE_EXT, PALETTE_MAX };
enum CommitMode { COMMIT_DISCARD, COMMIT_APPDATA, COMMIT_DIRECTLY}; 

// General
extern	int	plugThruDevice;
extern	BOOL	AutoCorrect;
extern	BOOL	AutoRun;
extern	BOOL	dbgVisible;
extern	Region	DefaultRegion;
extern	BOOL	GameGenie;
extern	int	SizeMult;
extern	BOOL	FixAspect;
extern	BOOL	FastLoad;
extern	CommitMode CommitMode35;
extern	BOOL	HardGameSaver;
extern	BOOL	SkipSRAMSave;
// CPU
extern	BOOL	LogBadOps;
extern	int	RAMInitialization;
// APU
extern	BOOL	BootWithDisabledFrameIRQ;
extern	int	ExpansionAudioVolume;
extern	int	LowPassFilterAPU;
extern	BOOL	LowPassFilterOneBus;
extern	BOOL	NonlinearMixing;
extern	BOOL	SoundEnabled;
extern	BOOL	SwapDutyCycles;
extern	BOOL	BootlegExpansionAudio;
extern	BOOL	CleanN163;
extern	BOOL	PreventWaveformClash;
extern	BOOL	RemoveDPCMBuzz;
extern	BOOL	RemoveDPCMPops;
extern	BOOL	ReverseDPCM;
extern	BOOL	DMCJoypadCorruption;
extern	int	Xstart;
extern	int	Xend;
extern	int	Ystart;
extern	int	Yend;
extern	BOOL	DisablePeriodicNoise;
extern	BOOL	VT369SoundHLE;
// PPU
extern	BOOL	DisableEmphasis;
extern	BOOL	IgnoreRaceCondition;
extern	int	VSDualScreens;
extern	BOOL	VSync;
extern	int	VT03Palette;
extern  BOOL    VT32RemoveBlueishCast;
extern	BOOL	PPUSoftReset;
extern	BOOL	PPUNeverClip;
extern	BOOL	Dendy60Hz;
extern	BOOL	DendyNTSCAspect;
extern	BOOL	DendySwapEmphasis;
extern	BOOL	DisableOAMData;
extern	BOOL	NoSpriteLimit;
extern	BOOL	ScrollGlitch;
extern	BOOL	upByOne;
// GFX
extern	BOOL	aFSkip;
extern	int	FSkip;
extern	BOOL	NTSCto709;
extern	int	NTSCHue;
extern	int	NTSCAxis;
extern	int	NTSCSaturation;
extern	int	PALSaturation;
extern	BOOL	PALTweakedAngles;
extern	PALETTE Palette[PALREGION_MAX];
extern	BOOL	RGBsRGB;
extern	BOOL	Scanlines;
extern	BOOL	NTSCSetup;
// Paths and filenames
extern	TCHAR	CustPalette[PALREGION_MAX][MAX_PATH];
extern	TCHAR	Path_ROM[MAX_PATH];
extern	TCHAR	Path_NMV[MAX_PATH];
extern	TCHAR	Path_AVI[MAX_PATH];
extern	TCHAR	Path_PAL[MAX_PATH];
extern	TCHAR	Path_NST[MAX_PATH];
extern	TCHAR	Path_BMP[MAX_PATH];
extern	TCHAR	Path_CopyNES[MAX_PATH];
extern	PALETTE DefaultPalette[PALREGION_MAX];
void	SaveSettings (void);
void	LoadSettings (void);
void	ApplySettingsToMenu (void);
void	GetWindowPosition (void);
void	SetWindowPosition (void);
}