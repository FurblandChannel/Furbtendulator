#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_Latch.h"

namespace {
uint8_t		chr;
uint8_t		prevA12A13;

void	sync (void) {
	EMU->SetPRG_ROM32(0x8, Latch::data &3);
	EMU->SetCHR_RAM4(0, Latch::data &4 |chr &3);
	EMU->SetCHR_RAM4(4, Latch::data &4 |     3);
	iNES_SetMirroring();
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, true);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		chr =0;
		prevA12A13 =0;
	}
	Latch::reset(resetType);
}

void	MAPINT	ppuCycle (int addr, int scanline, int cycle, int isRendering) {
	int A12A13 =addr >>12 &3;
	if (prevA12A13 !=2 && A12A13 ==2) {
		chr =addr >>8;
		sync();
	}
	prevA12A13 =A12A13;
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =Latch::saveLoad_D(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, chr);
	SAVELOAD_BYTE(stateMode, offset, data, prevA12A13);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =96;
} // namespace

MapperInfo MapperInfo_096 ={
	&mapperNum,
	_T("Oeka Kids"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	NULL,
	ppuCycle,
	saveLoad,
	NULL,
	NULL
};