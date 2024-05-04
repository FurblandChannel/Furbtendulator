/* Nintendulator Mapper DLLs
 * Copyright (C) 2002-2011 QMT Productions
 *
 * $URL: https://nintendulator.svn.sourceforge.net/svnroot/nintendulator/mappers/trunk/src/Hardware/Sound/s_VRC7.h $
 * $Id: s_VRC7.h 1258 2012-02-14 04:17:32Z quietust $
 */

#pragma once

namespace VRC7sound
{
void		Load		(bool isVRC7);
void		Reset		(RESET_TYPE);
void		Unload		(void);
void		Write		(int,int);
int	MAPINT	Get		(int);
int	MAPINT	SaveLoad	(STATE_TYPE,int,unsigned char *);
} // namespace VRC7sound
