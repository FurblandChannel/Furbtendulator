#pragma once

#include	"..\interface.h"
#include	"Sound\s_SUN5.h"

namespace FME7 {
extern	uint8_t		chr[8];
extern	uint8_t		prg[4];

void	MAPINT	load		(FSync);
void	MAPINT	reset		(RESET_TYPE);
void	MAPINT	unload		(void);
void		syncMirror	(void);
void		syncPRG		(int,int);
void		syncCHR		(int,int);
int	MAPINT	saveLoad	(STATE_TYPE,int,unsigned char *);
void	MAPINT	writeIndex	(int,int,int);
void	MAPINT	writeData	(int,int,int);
void	MAPINT	writeSound	(int,int,int);
void	MAPINT	cpuCycle	(void);
} // namespace FME7
