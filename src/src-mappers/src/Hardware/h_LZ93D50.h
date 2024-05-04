#pragma once

#include	"..\interface.h"
#include	"h_EEPROM.h"

namespace LZ93D50 {
extern	uint8_t		prg;
extern	uint8_t		chr[8];
extern	uint8_t		mirroring;
extern	uint16_t	counter;
extern	uint16_t	latch;
extern	bool		irqEnabled;
extern	bool		wramEnabled;

void	syncPRG (int, int);
void	syncCHR (int, int);
void	syncMirror (void);
void	MAPINT	writeASIC (int, int, int);
BOOL	MAPINT	load (FSync, EEPROM_I2C*, bool);
void	MAPINT	reset (RESET_TYPE);
void	MAPINT	cpuCycle (void);
int	MAPINT	saveLoad (STATE_TYPE, int, unsigned char *);
}