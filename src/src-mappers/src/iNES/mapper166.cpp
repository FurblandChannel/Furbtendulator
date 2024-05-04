#include	"..\DLL\d_iNES.h"

namespace {
uint8_t Reg[4];

void	Sync (void) {
	EMU->SetPRG_RAM8(0x6, 0);
	EMU->SetCHR_RAM8(0, 0);
	uint8_t Base =((Reg[0] ^Reg[1]) &0x10) <<1;
	uint8_t Bank = (Reg[2] ^Reg[3]) &0x1F;
	if (Reg[1] &8) {
		EMU->SetPRG_ROM16(0x8, (Base +Bank) &0xFE);
		EMU->SetPRG_ROM16(0xC, (Base +Bank) |0x01);
	} else if (Reg[1] &4) {
		EMU->SetPRG_ROM16(0x8, 0x1F);
		EMU->SetPRG_ROM16(0xC, Base +Bank);
	} else {
		EMU->SetPRG_ROM16(0x8, Base +Bank);
		EMU->SetPRG_ROM16(0xC, 0x07);
	}
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	for (int i =0; i <4; i++) SAVELOAD_BYTE(mode, offset, data, Reg[i]);
	if (mode == STATE_LOAD) Sync();
	return offset;
}

void	MAPINT	Write (int Bank, int Addr, int Val) {
	Reg[(Bank >>1) &0x3] =Val;
	Sync();
}

BOOL	MAPINT	Load (void) {
	return TRUE;
}
void	MAPINT	Reset (RESET_TYPE ResetType) {
	iNES_SetMirroring();

	for (int i =8; i<16; i++) EMU->SetCPUWriteHandler(i, Write);
	if (ResetType ==RESET_HARD) for (int i =0; i <4; i++) Reg[i] =0;
	Sync();
}

uint16_t MapperNum = 166;
} // namespace

MapperInfo MapperInfo_166 = {
	&MapperNum,
	_T("Subor Type A"),
	COMPAT_FULL,
	Load,
	Reset,
	NULL,
	NULL,
	NULL,
	SaveLoad,
	NULL,
	NULL
};