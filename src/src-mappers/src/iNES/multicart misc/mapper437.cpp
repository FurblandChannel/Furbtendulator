#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
uint8_t		outerBank;

void	sync (void) {
	EMU->SetPRG_ROM16(0x8, Latch::data &7 | outerBank <<3);
	EMU->SetPRG_ROM16(0xC,              7 | outerBank <<3);
	iNES_SetCHR_Auto8(0x0, 0);
	if (outerBank &0x08)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, TRUE);
	return TRUE;
}

void	MAPINT	writeOuterBank (int bank, int addr, int val) {
	outerBank =addr &0xF;
	sync();
}

void	MAPINT	reset (RESET_TYPE resetType) {
	outerBank =0;
	Latch::reset(resetType);
	EMU->SetCPUWriteHandler(0x5, writeOuterBank);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =Latch::saveLoad_D(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, outerBank);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =437;
} // namespace

MapperInfo MapperInfo_437 ={
	&mapperNum,
	_T("TH2348"),
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