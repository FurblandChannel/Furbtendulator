/* Nintendulator Mapper DLLs
 * Copyright (C) 2002-2011 QMT Productions
 *
 * $URL: svn+ssh://quietust@svn.code.sf.net/p/nintendulator/code/mappers/trunk/src/Hardware/h_N118.h $
 * $Id: h_N118.h 1258 2012-02-14 04:17:32Z quietust $
 */

#pragma once

#include	"..\interface.h"

namespace N118 {
extern	uint8_t		pointer;
extern	uint8_t		reg[8];
	
void		syncPRG			(int,int);
void		syncCHR			(int,int);
void	MAPINT	writeReg		(int,int,int);
void	MAPINT	load			(FSync);
void	MAPINT	reset			(RESET_TYPE);
int	MAPINT	saveLoad		(STATE_TYPE,int,unsigned char *);
} // namespace N118
