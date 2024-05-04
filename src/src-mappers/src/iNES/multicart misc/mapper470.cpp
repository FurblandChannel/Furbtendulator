#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
uint8_t		outerBank;

void	sync (void) {
	EMU->SetPRG_ROM32(0x8, Latch::data &7 | outerBank <<3);
	EMU->SetCHR_RAM8(0, 0);
	if (Latch::data &0x10)
		EMU->Mirror_S1();
	else	
		EMU->Mirror_S0();
}

void	MAPINT	writeOuterBank (int bank, int addr, int val) {
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
	EMU->SetCPUWriteHandler(0x5, writeOuterBank);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =Latch::saveLoad_D(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, outerBank);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =470;
} // namespace

MapperInfo MapperInfo_470 ={
	&mapperNum,
	_T("INX_007T_V01"),
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
