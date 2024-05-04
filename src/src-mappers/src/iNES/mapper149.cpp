/* Nintendulator Mapper DLLs
 * Copyright (C) 2002-2011 QMT Productions
 *
 * $URL: svn+ssh://quietust@svn.code.sf.net/p/nintendulator/code/mappers/trunk/src/iNES/mapper149.cpp $
 * $Id: mapper149.cpp 1311 2015-03-01 03:56:04Z quietust $
 */

#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_Latch.h"

namespace
{
void	Sync (void)
{
	EMU->SetPRG_ROM32(0x8, 0);
	EMU->SetCHR_ROM8(0, (Latch::data & 0x80) >> 7);
}

BOOL	MAPINT	Load (void)
{
	Latch::load(Sync, FALSE);
	return TRUE;
}
void	MAPINT	Reset (RESET_TYPE ResetType)
{
	iNES_SetMirroring();
	Latch::reset(ResetType);
}

uint16_t MapperNum = 149;
} // namespace

MapperInfo MapperInfo_149 =
{
	&MapperNum,
	_T("聖謙 (SA-0036)"),
	COMPAT_FULL,
	Load,
	Reset,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_D,
	NULL,
	NULL
};
