#pragma once

#include	"..\interface.h"

namespace FDS {
extern uint8_t Mirror;

void			sync		(void);
void		MAPINT	reset		(RESET_TYPE);
int		MAPINT	saveLoad	(STATE_TYPE,int,unsigned char *);
void		MAPINT	cpuCycle	(void);
int		MAPINT	mapperSnd	(int);
} // namespace FDS

