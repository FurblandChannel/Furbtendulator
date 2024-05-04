#pragma once
#include	"..\interface.h"
#include	"h_OneBus_GPIO.hpp"

namespace OneBus {
int		getPRGBank (int);
void		syncPRG (int, int);
void		syncPRG16 (int, int, int, int);
void		syncCHR (int, int);
void		syncMirror (void);

extern	uint32_t CHRROMSize;
extern	uint8_t* chrData;
extern	uint8_t* chrLow;
extern	uint8_t* chrHigh;
extern	uint8_t* chrLow16;
extern	uint8_t* chrHigh16;
extern	uint8_t  reg2000[0x100];
extern	uint8_t  reg4100[0x100];
extern	GPIO     gpio[4];

int	MAPINT	readAPU (int, int );
int	MAPINT	readAPUDebug (int, int);
void	MAPINT	writeAPU (int, int, int);
void	MAPINT	writePPU (int, int, int);
void	MAPINT	writeMMC3 (int, int, int);
void	MAPINT	load (FSync);
void	MAPINT	reset (RESET_TYPE);
void	MAPINT	unload (void);
int	MAPINT	saveLoad (STATE_TYPE, int, unsigned char *);
void	MAPINT	cpuCycle (void);
void	MAPINT	ppuCycle (int, int, int, int);
int	MAPINT	mapperSound (int);
} // namespace OneBus
