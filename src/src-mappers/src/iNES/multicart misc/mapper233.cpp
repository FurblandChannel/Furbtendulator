#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
uint8_t		prgA19;

void	sync (void) {
	if (Latch::data &0x20) {
		EMU->SetPRG_ROM16(0x8, Latch::data &0x1F | prgA19);
		EMU->SetPRG_ROM16(0xC, Latch::data &0x1F | prgA19);
	} else			// NROM-256
		EMU->SetPRG_ROM32(0x8,(Latch::data &0x1F | prgA19) >>1);
	EMU->SetCHR_RAM8(0x0, 0x0);

	switch (Latch::data >>6) {
		case 0:	EMU->Mirror_S0(); break;
		case 1:	EMU->Mirror_V (); break;
		case 2:	EMU->Mirror_H (); break;
		case 3:	EMU->Mirror_S1(); break;
	}
}
	
BOOL	MAPINT	load (void) {
	Latch::load(sync, FALSE);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD)
		prgA19 =0;
	else
		prgA19 ^=0x20;
	Latch::reset(resetType);
}


int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset = Latch::saveLoad_D(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, prgA19);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t MapperNum =233;
} // namespace

MapperInfo MapperInfo_233 ={
	&MapperNum,
	_T("Reset-based Tsang Hai 4+4 Mib"),
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

