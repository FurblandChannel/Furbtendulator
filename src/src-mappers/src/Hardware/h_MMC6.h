#pragma once
#include	"..\interface.h"

namespace MMC6 {
extern	uint8_t		pointer;
extern	uint8_t		reg[8];
extern	uint8_t		mirroring;
extern	uint8_t		wram;

extern	uint8_t		reloadValue;
extern	uint16_t	counter;
extern	bool		enableIRQ;
extern	uint8_t		pa12Filter;

extern	FSync		sync;
void	MAPINT	load			(FSync);
void	MAPINT	reset			(RESET_TYPE);
void		syncMirror		(void);
int		getPRGBank		(int);
void		syncPRG			(int,int);
void		syncPRG			(int,int);
void		syncWRAM		(void);
int		getCHRBank		(int);
void		syncCHR			(int,int);
void		syncCHR_ROM		(int,int);
void		syncCHR_RAM		(int,int);
int	MAPINT	saveLoad		(STATE_TYPE,int,unsigned char *);
void	MAPINT	write			(int,int,int);
void	MAPINT	writeReg		(int,int,int);
void	MAPINT	writeMirroringWRAM	(int,int,int);
void	MAPINT	writeIRQConfig		(int,int,int);
void	MAPINT	writeIRQEnable		(int,int,int);
void	MAPINT	cpuCycle		(void);
void	MAPINT	ppuCycle		(int,int,int,int);
} // namespace MMC6
