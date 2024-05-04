#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_VRC24.h"

namespace {
uint8_t		reg;

int	MAPINT	interceptRead (int bank, int addr);
void	sync (void) {
	if (reg &4) {
		int A14 =~reg &2;
		EMU->SetPRG_ROM8(0x8, (VRC24::prg[0] &~A14) &0x0F | reg <<4);
		EMU->SetPRG_ROM8(0xA, (VRC24::prg[1] &~A14) &0x0F | reg <<4);
		EMU->SetPRG_ROM8(0xC, (VRC24::prg[0] | A14) &0x0F | reg <<4);
		EMU->SetPRG_ROM8(0xE, (VRC24::prg[1] | A14) &0x0F | reg <<4);
	} else
		VRC24::syncPRG(0x0F, reg <<4);
	VRC24::syncCHR(0x7F, reg <<7);
	VRC24::syncMirror();

	for (int bank =0x8; bank <=0xF; bank++) EMU->SetCPUReadHandler(bank, ROM->dipValueSet && reg &8? interceptRead: EMU->ReadPRG);
}

int	MAPINT	interceptRead (int bank, int addr) {
	return EMU->ReadPRG(bank, addr &~3 | ROM->dipValue &3);
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	VRC24::writeWRAM(bank, addr, val);
	if (VRC24::wramEnable && ~reg &1) reg =addr &0xFF;
	sync();
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	VRC24::load(sync, true, 0x04, 0x08, NULL, true, 0);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	reg =0;
	VRC24::reset(resetType);
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
	_T("KL-06"),
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