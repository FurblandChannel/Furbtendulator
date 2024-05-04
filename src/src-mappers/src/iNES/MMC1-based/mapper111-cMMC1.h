#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC1.h"

namespace cMMC1 {
void	sync (void) {
	MMC1::syncPRG(0xF, 0);
	MMC1::syncCHR_ROM(0x3F, 0);
	MMC1::syncWRAM(0);
	MMC1::syncMirror();
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	MMC1::reg[bank >>1 &3] =val; 
	sync();
}

BOOL	MAPINT	load (void) {
	MMC1::load(sync, MMC1Type::MMC1B);
	iNES_SetSRAM();
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	MMC1::reset(resetType);
	for (int bank =0x8; bank<=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeReg);
}

uint16_t mapperNum =111;
MapperInfo MapperInfo_111 = {
	&mapperNum,
	_T("MMC1 with non-serialized registers"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	NULL,
	NULL,
	MMC1::saveLoad,
	NULL,
	NULL
};
} // namespace
