#include	"..\..\DLL\d_iNES.h"

#define nrom256  !(reg[0] &0x04)
#define mirrorH !!(reg[0] &0x01)
#define prg        reg[1]
#define chr        reg[2]

namespace {
uint8_t		reg[4];

void	sync (void) {
	EMU->SetPRG_ROM16(0x8, prg &~nrom256);
	EMU->SetPRG_ROM16(0xC, prg | nrom256);
	EMU->SetCHR_ROM8(0x0, chr);
	if (mirrorH)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	if (addr &ROM->dipValue) {
		reg[addr &3] =val;
		sync();
	}
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (!ROM->dipValueSet) ROM->dipValue =0x010;
	for (auto& c: reg) c =0;
	sync();
	EMU->SetCPUWriteHandler(0x5, writeReg);	
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	for (auto& c: reg) SAVELOAD_BYTE(stateMode, offset, data, c);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =463;
} // namespace

MapperInfo MapperInfo_463 ={
	&mapperNum,
	_T("YH810X1"),
	COMPAT_FULL,
	NULL,
	reset,
	NULL,
	NULL,
	NULL,
	saveLoad,
	NULL,
	NULL
};