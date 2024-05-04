#include	"..\..\DLL\d_iNES.h"

namespace {
bool		extraChip;
uint8_t		reg8;
uint8_t		regA;

void	sync (void) {
	if (extraChip)
		EMU->SetPRG_ROM16(0x8,       0x80  | reg8 &((ROM->INES_PRGSize -1) &0xF));
	else
		EMU->SetPRG_ROM16(0x8, regA &0x70  | reg8 &0x0F);
	EMU->SetPRG_ROM16(0xC, regA &0x7F);
	EMU->SetCHR_RAM8(0, 0);
	if (reg8 &0x10) {
		EMU->Mirror_H();
	} else {
		EMU->Mirror_V();
	}
}

void	MAPINT	write8 (int bank, int addr, int val) {
	reg8 =val;
	sync();
}

void	MAPINT	writeA (int bank, int addr, int val) {
	regA =val;
	extraChip =!(bank &4);
	sync();
}

void	MAPINT	reset (RESET_TYPE resetType) {
	reg8 =0;
	regA =0;
	extraChip =true;
	sync();
	for (int bank =0x8; bank <=0x9; bank++) EMU->SetCPUWriteHandler(bank, write8);
	for (int bank =0xA; bank <=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeA);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BOOL(stateMode, offset, data, extraChip);
	SAVELOAD_BYTE(stateMode, offset, data, reg8);
	SAVELOAD_BYTE(stateMode, offset, data, regA);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =274;
} // namespace

MapperInfo MapperInfo_274 = {
	&mapperNum,
	_T("80013-B"),
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
