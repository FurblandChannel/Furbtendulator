/* Nintendulator Mapper DLLs
 * Copyright (C) 2002-2011 QMT Productions
 *
 * $URL: https://nintendulator.svn.sourceforge.net/svnroot/nintendulator/mappers/trunk/src/Hardware/Sound/s_FDS.h $
 * $Id: s_FDS.h 1258 2012-02-14 04:17:32Z quietust $
 */

#pragma once

namespace FDSsound
{
void		Reset		(RESET_TYPE);
void		ResetBootleg	(RESET_TYPE);
int		Read		(int);
void		Write		(int,int);
void	MAPINT	Write4		(int, int, int);
int	MAPINT	Get		(int);
int	MAPINT	GetBootleg	(int);
int	MAPINT	SaveLoad	(STATE_TYPE,int,unsigned char *);
} // namespace FDSsound
