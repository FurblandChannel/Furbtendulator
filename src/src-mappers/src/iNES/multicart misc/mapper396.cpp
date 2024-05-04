#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
uint8_t		outerBank;

void	sync (void) {
	EMU->SetPRG_ROM16(0x8, Latch::data &7 | outerBank <<3);
	EMU->SetPRG_ROM16(0xC,              7 | outerBank <<3);
	EMU->SetCHR_RAM8(0, 0);
	if (outerBank &0x60)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
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
	if (ROM->INES2_SubMapper ==1) { // Realtec 8030
		if (resetType ==RESET_HARD) outerBank =0;
		Latch::reset(RESET_HARD);
		for (int bank =0x8; bank <=0xB; bank++) EMU->SetCPUWriteHandler(bank, writeOuterBank);
	} else {
		outerBank =0;
		Latch::reset(RESET_HARD);
		for (int bank =0xA; bank <=0xB; bank++) EMU->SetCPUWriteHandler(bank, writeOuterBank);
	}
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =Latch::saveLoad_D(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, outerBank);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =396;
} // namespace

MapperInfo MapperInfo_396 = {
	&mapperNum,
	_T("晶太 YY850437C"),
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