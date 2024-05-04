#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
uint8_t		prg;
uint8_t		chr;

void	sync (void) {
	if (prg &0x40)
		EMU->SetPRG_ROM32(0x8, prg >>3 &3);
	else {
		EMU->SetPRG_ROM16(0x8, prg >>2 &6 | prg >>5 &1);
		EMU->SetPRG_ROM16(0xC, prg >>2 &6 | prg >>5 &1);
	}
	EMU->SetCHR_ROM8(0, chr >>4 &~(chr <<2 &4) | Latch::data <<2 &(chr <<2 &4));
	
	if (prg &0x80)
		EMU->Mirror_V();
	else	
		EMU->Mirror_H();
}

int	MAPINT	readPad (int bank, int addr) {
	return ROM->dipValue;
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	switch (addr &4) {
		case 0:	chr =val; break;
		case 4:	prg =val; break;
	}
	sync();
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, FALSE);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	prg =0;
	chr =0;	
	EMU->SetCPUReadHandler(0x5, readPad);
	for (int bank =0x6; bank<=0x7; bank++) EMU->SetCPUWriteHandler(bank, writeReg);
	Latch::reset(resetType);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, prg);
	SAVELOAD_BYTE(stateMode, offset, data, chr);
	offset =Latch::saveLoad_D(stateMode, offset, data);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t MapperNum =319;
} // namespace

MapperInfo MapperInfo_319 ={
	&MapperNum,
	_T("HP-898F"),
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