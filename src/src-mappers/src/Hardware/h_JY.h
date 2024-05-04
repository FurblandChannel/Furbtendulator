#pragma once
#include	"..\interface.h"
#include	"..\DLL\d_iNES.h"

namespace JY {
extern	uint8_t		mode;
extern	uint8_t		ciramConfig;
extern	uint8_t		vramConfig;
extern	uint8_t		outerBank;
extern	uint8_t		irqControl;

void		syncPRG (int, int );
void		syncCHR (int, int );
void		syncNT (int, int );
BOOL	MAPINT	load (FSync, bool );
void	MAPINT	reset (RESET_TYPE );
void	MAPINT	cpuCycle (void);
void	MAPINT	ppuCycle (int, int, int, int );
int	MAPINT	saveLoad (STATE_TYPE, int, unsigned char *);

} // namespace