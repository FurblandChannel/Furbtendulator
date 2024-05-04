#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
uint8_t		outerBank;

void	sync (void) {
	if (~outerBank &0x20) outerBank =Latch::addr;
	if (outerBank &0x08)
		EMU->SetPRG_ROM32(0x8, Latch::data &3 | (outerBank <<2));
	else {
		EMU->SetPRG_ROM16(0x8, Latch::data &7 | (outerBank <<3));
		
		EMU->SetPRG_ROM16(0xC,              7 | (outerBank <<3));
	}
	EMU->SetCHR_RAM8(0x0, 0x0);
	if (outerBank &0x10)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, TRUE);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	outerBank =0;
	Latch::reset(RESET_HARD);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =Latch::saveLoad_AD(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, outerBank);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =382;
} // namespace

MapperInfo MapperInfo_382 ={
	&mapperNum,
	_T("830928C"),
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