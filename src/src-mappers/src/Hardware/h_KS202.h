#pragma once
#include	"..\interface.h"

namespace KS202 {
extern	uint8_t		index;
extern	uint8_t		prg[4];
extern	uint8_t		irqControl;
extern	uint16_t	irqCounter;
extern	uint16_t	irqLatch;
extern	FSync		sync;

void	syncPRG (int, int);
void	syncPRG (int, int, int, int, int, int);
void	MAPINT	writeIRQcounter (int, int, int);
void	MAPINT	writeIRQenable (int, int, int);
void	MAPINT	writeIRQacknowledge (int, int, int);
void	MAPINT	writePRGindex (int, int, int);
void	MAPINT	writePRGdata (int, int, int);
void	MAPINT	load (FSync);
void	MAPINT	reset (RESET_TYPE);
void	MAPINT	cpuCycle (void);
int	MAPINT	saveLoad (STATE_TYPE, int, unsigned char *);
}