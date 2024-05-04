#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_VRC24.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
uint8_t		reg;
FCPUWrite	writeWRAM;

void	sync (void) {
	if (reg &0x08) {
		EMU->SetPRG_ROM32(0x8, Latch::data &0x07 | reg <<2 &~0x07);
		if (Latch::data &0x10)
			EMU->Mirror_S1();
		else	
			EMU->Mirror_S0();
	} else {
		if (reg &0x04) {
			EMU->SetPRG_ROM16(0x8, VRC24::prg[0] &0x0F | reg <<3 &~0x0F);
			EMU->SetPRG_ROM16(0xC,                0x0F | reg <<3 &~0x0F);
		} else {
			EMU->SetPRG_ROM16(0x8, VRC24::prg[0] &0x07 | reg <<3);
			EMU->SetPRG_ROM16(0xC,                0x07 | reg <<3);
		}
		VRC24::syncMirror();
	}
	EMU->SetCHR_RAM8(0x0, 0);
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	if (VRC24::wramEnable) reg =addr &0xFF;
	writeWRAM(bank, addr, val);
	sync();
}

void	MAPINT	writeASIC (int bank, int addr, int val) {
	Latch::write(bank, addr, val);
	VRC24::write(bank, addr, val);
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	Latch::load(sync, false);
	VRC24::load(sync, true, 0x04, 0x08, NULL, true, 0);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	reg =0;
	Latch::reset(RESET_HARD);
	VRC24::reset(resetType);
	writeWRAM =EMU->GetCPUWriteHandler(0x6);
	for (int bank =0x6; bank <=0x7; bank++) EMU->SetCPUWriteHandler(bank, writeReg);
	for (int bank =0x8; bank <=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeASIC);
}

uint16_t MapperNum =448;
} // namespace

MapperInfo MapperInfo_448 = {
	&MapperNum,
	_T("830768C"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	VRC24::cpuCycle,
	NULL,
	VRC24::saveLoad,
	NULL,
	NULL
};