﻿/* Nintendulator Mapper DLLs
 * Copyright (C) 2002-2011 QMT Productions
 *
 * $URL: svn+ssh://quietust@svn.code.sf.net/p/nintendulator/code/mappers/trunk/src/iNES/mapper145.cpp $
 * $Id: mapper145.cpp 1311 2015-03-01 03:56:04Z quietust $
 */

#include	"..\DLL\d_iNES.h"

namespace
{
uint8_t Reg;
FCPUWrite _Write4;

void	Sync (void)
{
	EMU->SetPRG_ROM32(0x8, 0);
	EMU->SetCHR_ROM8(0, Reg >> 7);
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data)
{
	SAVELOAD_BYTE(mode, offset, data, Reg);
	if (mode == STATE_LOAD)
		Sync();
	return offset;
}

void	MAPINT	Write (int Bank, int Addr, int Val)
{
	if (Bank == 4)
		_Write4(Bank, Addr, Val);
	if (Addr & 0x100)
		Reg = Val & 0x80;
	Sync();
}

void	MAPINT	Reset (RESET_TYPE ResetType)
{
	iNES_SetMirroring();

	_Write4 = EMU->GetCPUWriteHandler(0x4);
	for (int i = 0x4; i < 0x6; i++)
		EMU->SetCPUWriteHandler(i, Write);

	if (ResetType == RESET_HARD)
		Reg = 0;
	Sync();
}

uint16_t MapperNum = 145;
} // namespace

MapperInfo MapperInfo_145 =
{
	&MapperNum,
	_T("聖謙 (SA-72007)"),
	COMPAT_FULL,
	NULL,
	Reset,
	NULL,
	NULL,
	NULL,
	SaveLoad,
	NULL,
	NULL
};