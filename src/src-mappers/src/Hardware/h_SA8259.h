#pragma once
#include	"..\interface.h"

namespace SA8259 {
extern	uint8_t		index;
extern	uint8_t		prg;
extern	uint8_t		chrMid;
extern	uint8_t		chrHigh;
extern	uint8_t		chr[4];
extern	uint8_t		mirroring;
extern	bool		simple;
extern	FCPUWrite	writeAPU;
extern	FSync		sync;

void		syncPRG (void);
void		syncCHR (int);
void		syncMirror (void);
void	MAPINT	writeReg (int, int, int);
void	MAPINT	load (FSync);
void	MAPINT	reset (RESET_TYPE);
int	MAPINT	saveLoad (STATE_TYPE, int, unsigned char *);
}