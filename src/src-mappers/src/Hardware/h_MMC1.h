#pragma once
#include	"..\interface.h"

enum struct MMC1Type {
	MMC1A,
	MMC1B
};

namespace MMC1 {

extern FSync	sync;
extern MMC1Type	revision;
extern uint8_t	reg[4];
extern uint8_t	shift;
extern uint8_t	bits;
extern uint8_t	filter;

void	MAPINT	write		(int, int, int);
int		getPRGBank	(int);
int		getCHRBank	(int);
void		syncWRAM	(int);
void		syncPRG		(int, int);
void		syncCHR		(int, int);
void		syncCHR_ROM	(int, int);
void		syncCHR_RAM	(int, int);
void		syncMirror	(void);

void	MAPINT	load		(FSync, MMC1Type);
void	MAPINT	reset		(RESET_TYPE);
void	MAPINT	cpuCycle	(void);
int	MAPINT	saveLoad	(STATE_TYPE, int, unsigned char *);
} // namespace MMC1
