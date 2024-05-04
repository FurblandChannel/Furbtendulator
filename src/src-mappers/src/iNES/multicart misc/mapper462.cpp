#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
uint8_t		outerBank;

void	sync (void) {
	if (outerBank &0x40) {
		EMU->SetPRG_ROM32(0x8, Latch::data &7 | outerBank >>3 &~7);
		if (Latch::data &0x10)
			EMU->Mirror_S1();
		else
			EMU->Mirror_S0();
	} else {
		EMU->SetPRG_ROM16(0x8, Latch::data &7 | outerBank >>2 &~7);
		EMU->SetPRG_ROM16(0xC,              7 | outerBank >>2 &~7);
		EMU->SetCHR_RAM8(0, 0);
		if (outerBank &0x10)
			EMU->Mirror_V();
		else
			EMU->Mirror_H();
	}
}

void	MAPINT	writeOuterBank(int bank, int addr, int val) {
	outerBank =val;
	sync();
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, false);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	outerBank =0;
	Latch::reset(RESET_HARD);
	for (int bank =0xA; bank <=0xB; bank++) EMU->SetCPUWriteHandler(bank, writeOuterBank);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =Latch::saveLoad_D(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, outerBank);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =462;
} // namespace

MapperInfo MapperInfo_462 = {
	&mapperNum,
	_T("BMC-971107-00G"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	NULL,
	NULL,
	saveLoad,
	NULL,
	NULL
};