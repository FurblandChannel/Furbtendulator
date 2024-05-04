#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

#define gameMode            (reg &0x10)
#define horizontalMirroring (reg &0x20)

namespace {
uint8_t		reg;

void	sync (void) {
	EMU->SetPRG_ROM8(0x6, reg <<4 |0xF);
	if (gameMode) {
		EMU->SetPRG_ROM16(0x8, reg <<3 | Latch::data &7);
		EMU->SetPRG_ROM16(0xC, reg <<3 |              7);
	} else
		for (int bank =0; bank <8; bank++) EMU->SetPRG_Ptr4(0x8 |bank, &ROM->MiscROMData[bank <<12], FALSE);
	EMU->SetCHR_RAM8(0x0, 0);
	if (horizontalMirroring)
		EMU->Mirror_H();
	else	
		EMU->Mirror_V();
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	if (!gameMode) {
		reg =val;
		sync();
	}
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, false);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	reg =0;
	Latch::reset(RESET_HARD);
	for (int bank =0x6; bank<=0x7; bank++) EMU->SetCPUWriteHandler(bank, writeReg);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =Latch::saveLoad_D(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum = 53;
} // namespace

MapperInfo MapperInfo_053 ={
	&mapperNum,
	_T("Supervision 16-in-1"),
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