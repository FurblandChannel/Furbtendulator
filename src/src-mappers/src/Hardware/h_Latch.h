#pragma once
#include	"..\interface.h"

namespace Latch {
extern	uint8_t		data;
extern	uint16_t	addr;

void	MAPINT	write		(int,int,int);
void	MAPINT	load		(FSync,bool);
void	MAPINT	reset		(RESET_TYPE);
void	MAPINT	resetHard	(RESET_TYPE);
int	MAPINT	saveLoad_AD	(STATE_TYPE,int,unsigned char *);
int	MAPINT	saveLoad_AL	(STATE_TYPE,int,unsigned char *);
int	MAPINT	saveLoad_A	(STATE_TYPE,int,unsigned char *);
int	MAPINT	saveLoad_D	(STATE_TYPE,int,unsigned char *);
} // namespace Latch
