/* Nintendulator Mapper DLLs
 * Copyright (C) 2002-2011 QMT Productions
 *
 * $URL: svn+ssh://quietust@svn.code.sf.net/p/nintendulator/code/mappers/trunk/src/Vs/mapper000.cpp $
 * $Id: mapper000.cpp 1311 2015-03-01 03:56:04Z quietust $
 */

#include	"..\DLL\d_VS.h"
#include	"..\Hardware\h_VS.h"

namespace {
BOOL	MAPINT	Load (void) {
	VS::Load();
	return TRUE;
}
void	MAPINT	Reset (RESET_TYPE ResetType) {
	VS::Reset(ResetType);
	iNES_SetMirroring();

	EMU->SetPRG_ROM32(0x8, 0);
	EMU->SetCHR_ROM8(0, 0);
}

void	MAPINT	Unload (void) {
	VS::Unload();
}

uint16_t MapperNum = 0;
} // namespace

MapperInfo MapperInfo_000 =
{
	&MapperNum,
	_T("NROM"),
	COMPAT_FULL,
	Load,
	Reset,
	Unload,
	VS::CPUCycle,
	NULL,
	VS::SaveLoad,
	NULL,
	NULL
};