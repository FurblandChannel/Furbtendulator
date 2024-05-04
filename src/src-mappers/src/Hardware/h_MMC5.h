#pragma once

#include	"..\interface.h"

namespace MMC5 {
void	MAPINT	load		(void);
void	MAPINT	reset		(RESET_TYPE);
int	MAPINT	saveLoad	(STATE_TYPE,int,unsigned char *);
void	MAPINT	cpuCycle	(void);
int	MAPINT	mapperSound	(int);
} // namespace MMC5
