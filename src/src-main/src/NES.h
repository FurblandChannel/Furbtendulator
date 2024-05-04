#pragma once
#include <vector>
#include <mutex>
#include "Settings.h"

namespace NES {
extern TCHAR CurrentFilename[MAX_PATH];
extern int SRAM_Size;

extern int PRGSizeROM, PRGSizeRAM, CHRSizeROM, CHRSizeRAM;
extern int PRGMaskROM, PRGMaskRAM, CHRMaskROM, CHRMaskRAM;

extern BOOL ROMLoaded;
extern volatile BOOL Running;
extern BOOL Scanline;
extern int WhichScreenToShow;
extern int WhichRGBPalette;

#define	STOPMODE_NOW	0x01
#define	STOPMODE_WAIT	0x02
#define	STOPMODE_SOFT	0x04
#define	STOPMODE_BREAK	0x08
#define	STOPMODE_QUIT	0x10

extern volatile int DoStop;
extern BOOL FrameStep, GotStep;
extern BOOL HasMenu;
extern BOOL GenericMulticart;
extern int NewPC;

extern Settings::Region CurRegion;

// Maximum supported data sizes, since it's far easier than dynamically allocating them

// 256 MiB PRG ROM
#define	MAX_PRGROM_SIZE	0x10000
#define	MAX_PRGROM_MASK	(MAX_PRGROM_SIZE - 1)

// 4 MiB PRG RAM
#define	MAX_PRGRAM_SIZE	0x400
#define	MAX_PRGRAM_MASK	(MAX_PRGRAM_SIZE - 1)

// 4096KB CHR ROM
#define	MAX_CHRROM_SIZE	0x1000
#define	MAX_CHRROM_MASK	(MAX_CHRROM_SIZE - 1)

// 256KB CHR RAM
#define	MAX_CHRRAM_SIZE	0x100
#define	MAX_CHRRAM_MASK	(MAX_CHRRAM_SIZE - 1)

extern unsigned char CPU_RAM[8192];
extern unsigned char PRG_ROM[MAX_PRGROM_SIZE][0x1000];
extern unsigned char PRG_RAM[MAX_PRGRAM_SIZE][0x1000];
extern unsigned char CHR_ROM[MAX_CHRROM_SIZE][0x400];
extern unsigned char CHR_RAM[MAX_CHRRAM_SIZE][0x400];
extern unsigned char coin1, coin2, mic;
extern uint32_t coinDelay1, coinDelay2, micDelay;
extern	char	fdsGameID [5];

extern	std::vector<std::vector<uint8_t>> disk28;
extern	TCHAR	name28[MAX_PATH];
extern	unsigned int side28;
extern	bool	modified28;
extern	bool	writeProtected28;
extern	std::mutex busy28;
extern	int fqdFusemap;
extern	int currentPlugThruDevice;

extern	std::vector<uint8_t> disk35;
extern	TCHAR	name35[MAX_PATH];
extern	std::mutex busy35;

void	Init (void);
void	Destroy (void);
void	OpenFile (TCHAR *);
void	CloseFile (void);
void	SaveSRAM (void);
void	LoadSRAM (void);
DWORD	getMask (unsigned int);
BOOL	OpenFileiNES (FILE *);
BOOL	OpenFileSTBX (FILE *);
BOOL	OpenFileUNIF (FILE *);
BOOL	OpenFileFDS (FILE *, TCHAR *);
BOOL	OpenFileNSF (FILE *);
BOOL	OpenFileMFC (FILE *);
BOOL	OpenFileSMC (FILE *);
BOOL	OpenFileTNES (FILE *);
BOOL	OpenFileBIN (FILE *);

bool	load28 (FILE *, TCHAR *);
bool	unload28 (void);
void	next28 (void);
void	previous28 (void);
void	insert28 (void);
void	eject28 (void);
int	FDSSave (FILE *out);
int	FDSLoad (FILE *in, int version_id);

bool	load35 (TCHAR *);
void	unload35 (void);

void	CreateMachine();
void	DestroyMachine();

void	SetRegion (Settings::Region);
void	ApplyRegion (void);
void	InitHandlers (void);
void	Reset (RESET_TYPE);

void	Start (BOOL);
void	Stop (void);

void	Pause (BOOL);
void	Resume (void);
void	SkipToVBlank (void);

void	SetupDataPath (void);
void	MapperConfig (void);
unsigned long int CRC32C(unsigned long int , const void *, size_t );

void	runCopyNES (TCHAR *);
void	loadMemoryDump(TCHAR *);
void	saveMemoryDump(TCHAR *);
} // namespace NES
