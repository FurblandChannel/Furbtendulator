/* Nintendulator Mapper DLLs
 * Copyright (C) 2002-2011 QMT Productions
 *
 * $URL: https://nintendulator.svn.sourceforge.net/svnroot/nintendulator/mappers/trunk/src/Hardware/Sound/s_MMC5.h $
 * $Id: s_MMC5.h 1258 2012-02-14 04:17:32Z quietust $
 */

#pragma once

namespace MMC5sound
{
void		Load		(void);
void		Reset		(RESET_TYPE);
void		Unload		(void);
void		Write		(int,int);
int		Read		(int);
int	MAPINT	Get		(int);
int	MAPINT	SaveLoad	(STATE_TYPE,int,unsigned char *);
BOOL		HaveIRQ		(void);
} // namespace MMC5sound
