#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_VRC24.h"

namespace {
uint8_t		reg;

void	sync (void) {
	int prgAND =reg &0x10? 0x1F: 0x0F;
	int chrAND =reg &0x10? 0xFF: 0x7F;
	VRC24::A0 =reg &0x10? 8: 4;
	VRC24::A1 =reg &0x10? 4: 8;
	if (~reg &0x20) {
		if (reg &0x6)
			EMU->SetPRG_ROM32(0x8, reg >>1);
		else {
			EMU->SetPRG_ROM16(0x8, reg);
			EMU->SetPRG_ROM16(0xC, reg);
		}
	} else {
		VRC24::syncPRG(prgAND, reg <<1 &~prgAND);
	}
	VRC24::syncCHR(chrAND, reg <<4 &~chrAND);
	VRC24::syncMirror();
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	VRC24::writeWRAM(bank, addr, val);
	if (VRC24::wramEnable && ~reg &1)  {
		reg =addr &0xFF;
		sync();
	}
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	VRC24::load(sync, true, 0x04, 0x08, NULL, true, 0);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	reg =0;
	VRC24::reset(RESET_HARD);
	for (int bank =0x6; bank <=0x7; bank++) EMU->SetCPUWriteHandler(bank, writeReg);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =VRC24::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t MapperNum =447;
} // namespace

MapperInfo MapperInfo_447 = {
	&MapperNum,
	_T("JC-011"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	VRC24::cpuCycle,
	NULL,
	saveLoad,
	NULL,
	NULL
};