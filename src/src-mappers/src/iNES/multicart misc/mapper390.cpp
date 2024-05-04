#include	"..\..\DLL\d_iNES.h"

namespace {
uint8_t		prg;
uint8_t		chr;
FCPURead	readCart;

void	sync (void) {
	switch (prg >>4 &3) {
		case 0: case 1:
			EMU->SetPRG_ROM16(0x8, prg);
			EMU->SetPRG_ROM16(0xC, prg |7);
			break;
		case 2: EMU->SetPRG_ROM32(0x8, prg >>1);
			break;
		case 3:	EMU->SetPRG_ROM16(0x8, prg);
			EMU->SetPRG_ROM16(0xC, prg);
			break;
	}
	EMU->SetCHR_ROM8(0, chr);
	if (chr &0x20)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

int	MAPINT	readDIP (int bank, int addr) {
	if ((prg >>4 &3) ==1)
		return readCart(bank, addr | ROM->dipValue);
	else
		return readCart(bank, addr);
}

void	MAPINT	writeCHR (int bank, int addr, int val) {
	chr =addr &0xFF;
	sync();
}

void	MAPINT	writePRG (int bank, int addr, int val) {
	prg =addr &0xFF;
	sync();
}

void	MAPINT	reset (RESET_TYPE resetType) {
	prg =0;
	chr =0;
	sync();
	readCart =EMU->GetCPUReadHandler(0x8);
	for (int bank =0x8; bank<=0xF; bank++) {
		EMU->SetCPUReadHandler(bank, readDIP);
		EMU->SetCPUReadHandlerDebug(bank, readDIP);
	}
	for (int bank =0x8; bank<=0xB; bank++) EMU->SetCPUWriteHandler(bank, writeCHR);
	for (int bank =0xC; bank<=0xF; bank++) EMU->SetCPUWriteHandler(bank, writePRG);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, prg);
	SAVELOAD_BYTE(stateMode, offset, data, chr);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =390;
} // namespace

MapperInfo MapperInfo_390 ={
	&mapperNum,
	_T("Realtec 8031"),
	COMPAT_FULL,
	NULL,
	reset,
	NULL,
	NULL,
	NULL,
	saveLoad,
	NULL,
	NULL
};