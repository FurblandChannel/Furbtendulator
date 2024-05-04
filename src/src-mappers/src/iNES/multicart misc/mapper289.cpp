#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
#define nrom256    !!(reg[0] &0x01)
#define unrom      !!(reg[0] &0x02)
#define protectCHR !!(reg[0] &0x04)
#define mirrorH    !!(reg[0] &0x08)
#define prgInner     (reg[1] & 0x07)
#define prgOuter     (reg[1] &~0x07)

uint8_t		reg[2];

void	sync (void) {
	EMU->SetPRG_ROM16(0x8, unrom*Latch::data&7 | unrom*nrom256*7 | !unrom*prgInner &~(!unrom*nrom256*1) |prgOuter);
	EMU->SetPRG_ROM16(0xC,                       unrom        *7 | !unrom*prgInner |  !unrom*nrom256*1  |prgOuter);
	EMU->SetCHR_RAM8(0x0, 0x00);
	if (protectCHR) protectCHRRAM();
	if (mirrorH)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

int	MAPINT	readPad (int bank, int addr) {
	return ROM->dipValue &3 | *EMU->OpenBus &~3;
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	reg[addr &1] =val;
	sync();
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, FALSE);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	for (auto& c: reg) c =0;
	Latch::reset(RESET_HARD);
	for (int bank =0x6; bank <=0x7; bank++) {
		EMU->SetCPUReadHandler(bank, readPad);
		EMU->SetCPUReadHandlerDebug(bank, readPad);
		EMU->SetCPUWriteHandler(bank, writeReg);
	}
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset = Latch::saveLoad_D(stateMode, offset, data);
	for (auto& c: reg) SAVELOAD_BYTE(stateMode, offset, data, c);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =289;
} // namespace

MapperInfo MapperInfo_289 = {
	&mapperNum,
	_T("60311C/N76A-1"),
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
