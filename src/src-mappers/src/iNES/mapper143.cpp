/* Nintendulator Mapper DLLs
 * Copyright (C) 2002-2011 QMT Productions
 *
 * $URL: svn+ssh://quietust@svn.code.sf.net/p/nintendulator/code/mappers/trunk/src/iNES/mapper143.cpp $
 * $Id: mapper143.cpp 1311 2015-03-01 03:56:04Z quietust $
 */

#include	"..\DLL\d_iNES.h"

namespace
{
FCPURead _Read4;
FCPURead _Read8;

int	MAPINT	Read4 (int Bank, int Addr)
{
	if (Addr & 0x100)
		return (~Addr & 0x3F) | ((*EMU->OpenBus) & 0xC0);
	else if (Bank == 4)
		return _Read4(Bank, Addr);
	else	return -1;
}

void	MAPINT	Reset (RESET_TYPE ResetType)
{
	iNES_SetMirroring();

	_Read4 = EMU->GetCPUReadHandler(0x4);
	EMU->SetCPUReadHandler(0x4, Read4);
	EMU->SetCPUReadHandler(0x5, Read4);
	EMU->SetPRG_ROM32(0x8, 0);
	EMU->SetCHR_ROM8(0, 0);
}

uint16_t MapperNum = 143;
} // namespace

MapperInfo MapperInfo_143 =
{
	&MapperNum,
	_T("聖謙 (TC-A001-72P/SA-014)"),
	COMPAT_FULL,
	NULL,
	Reset,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};