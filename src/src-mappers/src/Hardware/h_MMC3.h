#pragma once
#include	"..\interface.h"

enum struct MMC3Type {
	NEC,
	Sharp,
	Acclaim
};

namespace MMC3 {
extern	uint8_t		pointer;
extern	uint8_t		reg[8];
extern	uint8_t		mirroring;
extern	uint8_t		wram;

extern	uint8_t		reloadValue;
extern	uint8_t		counter;
extern	uint8_t		prescaler;
extern	bool		reload;
extern	bool		enableIRQ;
extern	uint8_t		pa12Filter;
extern	uint16_t	prevAddr;

extern	FSync		sync;
extern	FCPURead	wramRead, wramReadCallback;
extern	FCPUWrite	wramWrite, wramWriteCallback;
void	MAPINT	load			(FSync);
void	MAPINT	load			(FSync, MMC3Type);
void	MAPINT	reset			(RESET_TYPE);
void		setWRAMCallback		(FCPURead, FCPUWrite);
void		syncMirror		(void);
void		syncMirrorA17		(void);
int		getPRGBank		(int);
void		syncPRG			(int,int);
void		syncPRG_NROM		(int,int,int,int);
void		syncPRG_GNROM_66	(int,int,int);
void		syncPRG_GNROM_67	(int,int,int);
void		syncPRG			(int,int);
void		syncPRG_2 		(int,int,int,int);
void		syncPRG_4		(int,int,int,int,int,int,int,int);
void		syncWRAM		(void);
int		getCHRBank		(int);
void		syncCHR			(int,int);
void		syncCHR_ROM		(int,int);
void		syncCHR_ROM		(int,int,int);
void		syncCHR_ROM		(int,int,int,int,int);
void		syncCHR_RAM		(int,int);
void	MAPINT	write			(int,int,int);
void	MAPINT	writeReg		(int,int,int);
void	MAPINT	writeMirroringWRAM	(int,int,int);
void	MAPINT	writeIRQConfig		(int,int,int);
void	MAPINT	writeIRQEnable		(int,int,int);
void	MAPINT	cpuCycle		(void);
void	MAPINT	ppuCycle		(int,int,int,int);
void	MAPINT	ppuCycle_NEC		(int,int,int,int);
void	MAPINT	ppuCycle_Sharp		(int,int,int,int);
void	MAPINT	ppuCycle_Acclaim	(int,int,int,int);
void	MAPINT	ppuCycle_HBlank		(int,int,int,int);
int	MAPINT	saveLoad		(STATE_TYPE,int,unsigned char *);
} // namespace MMC3
