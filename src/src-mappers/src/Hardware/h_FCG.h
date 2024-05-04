#pragma once

#include	"..\interface.h"

namespace FCG {
extern	uint8_t		prg;
extern	uint8_t		chr[8];
extern	uint8_t		mirroring;
extern	uint16_t	counter;
extern	bool		irqEnabled;

void	syncPRG (int, int);
void	syncCHR (int, int);
void	syncMirror (void);
void	MAPINT	writeASIC (int, int, int);
BOOL	MAPINT	load (FSync);
void	MAPINT	reset (RESET_TYPE);
void	MAPINT	cpuCycle (void);
int	MAPINT	saveLoad (STATE_TYPE, int, unsigned char *);
}