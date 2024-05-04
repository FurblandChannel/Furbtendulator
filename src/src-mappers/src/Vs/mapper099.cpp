/* Nintendulator Mapper DLLs
 * Copyright (C) 2002-2011 QMT Productions
 *
 * $URL: svn+ssh://quietust@svn.code.sf.net/p/nintendulator/code/mappers/trunk/src/Vs/mapper099.cpp $
 * $Id: mapper099.cpp 1311 2015-03-01 03:56:04Z quietust $
 */

#include	"..\DLL\d_VS.h"
#include	"..\Hardware\h_VS.h"

namespace
{
FCPUWrite _Write, _WriteSecond;
uint8_t CHR[2];

void	Sync (void) {
	if (ROM->PRGROMSize ==24*1024) { // Vs. Tetris trimmed
		EMU->SetPRG_OB4(0x8);
		EMU->SetPRG_ROM8(0xA, 0);
		EMU->SetPRG_ROM8(0xC, 1);
		EMU->SetPRG_ROM8(0xE, 2);
	} else
		EMU->SetPRG_ROM32(0x8, 0);
	EMU->SetCHR_ROM8(0, CHR[0]);
	// Vs. Gumshoe has an extra 8KB of PRG ROM which it swaps using the same register
	if (ROM->INES_PRGSize ==3) EMU->SetPRG_ROM8(0x8, CHR[0] << 2);
	
	if (ROM->INES2_VSFlags ==VS_DUAL || ROM->INES2_VSFlags ==VS_BUNGELING) {
		if (ROM->INES_PRGSize ==3) { // 48 KiB of PRG-ROM means trimmed ROM
			if (ROM->INES2_VSFlags ==VS_DUAL) { // Vs. Mahjong
				EMU->SetPRG_OB4(0x8);
				EMU->SetPRG_ROM8(0xA, 0);
				EMU->SetPRG_ROM8(0xC, 1);
				EMU->SetPRG_ROM8(0xE, 2);
				EMU->SetPRG_OB4(0x18);
				EMU->SetPRG_ROM8(0x1A, 3);
				EMU->SetPRG_ROM8(0x1C, 4);
				EMU->SetPRG_ROM8(0x1E, 5);
			} else { // Vs. Raid on Bungeling Bay
				EMU->SetPRG_ROM32(0x8, 0);
				EMU->SetPRG_OB4(0x18);
				EMU->SetPRG_OB4(0x1A);
				EMU->SetPRG_OB4(0x1C);
				EMU->SetPRG_ROM8(0x1E, 4);
			}
		} else {
			EMU->SetPRG_ROM32(0x8, 0);
			EMU->SetPRG_ROM32(0x18, 1);
		}
		EMU->SetCHR_ROM8(0x40, CHR[1] |2);
	}
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(mode, offset, data, CHR[0]);
	SAVELOAD_BYTE(mode, offset, data, CHR[1]);
	offset = VS::SaveLoad(mode, offset, data);
	if (mode == STATE_LOAD)
		Sync();
	return offset;
}

void	MAPINT	Write (int Bank, int Addr, int Val) {
	if (Bank &0x10)
		_WriteSecond(Bank, Addr, Val);
	else
		_Write(Bank, Addr, Val);
	if (Addr ==0x016) {
		CHR[Bank >>4] = (Val & 0x04) >> 2;
		Sync();
	}
}

BOOL	MAPINT	Load (void) {
	VS::Load();
	return TRUE;
}
void	MAPINT	Reset (RESET_TYPE ResetType) {
	EMU->Mirror_4();
	VS::Reset(ResetType);
	_Write = EMU->GetCPUWriteHandler(0x4);
	EMU->SetCPUWriteHandler(0x4, Write);
	if (VSDUAL) {
		_WriteSecond = EMU->GetCPUWriteHandler(0x14);
		EMU->SetCPUWriteHandler(0x14, Write);
	}

	if (ResetType == RESET_HARD)
		CHR[0] =CHR[1] =0;
	Sync();
}
void	MAPINT	Unload (void) {
	VS::Unload();
}

uint16_t MapperNum = 99;
} // namespace

MapperInfo MapperInfo_099 =
{
	&MapperNum,
	_T("VS Unisystem"),
	COMPAT_FULL,
	Load,
	Reset,
	Unload,
	VS::CPUCycle,
	NULL,
	SaveLoad,
	NULL,
	NULL
};