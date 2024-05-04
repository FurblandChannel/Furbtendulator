#pragma once
#include	"..\interface.h"

namespace MMC4 {
extern	uint8_t		prg;
extern	uint8_t		chr[4];
extern	uint8_t		state[2];
extern	uint8_t		mirroring;
extern	FSync		sync;
extern	FPPURead	readCHR;

void		syncPRG		(int,int);
void		syncCHR		(int,int);
void		syncMirror	(void);
int	MAPINT	trapCHRRead	(int,int);
void	MAPINT	writePRG	(int,int,int);
void	MAPINT	writeCHR	(int,int,int);
void	MAPINT	writeMirroring	(int,int,int);
void	MAPINT	load		(FSync);
void	MAPINT	reset		(RESET_TYPE);
int	MAPINT	saveLoad	(STATE_TYPE,int,unsigned char *);
} // namespace MMC4
