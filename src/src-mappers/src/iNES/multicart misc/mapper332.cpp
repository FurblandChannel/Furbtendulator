#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

#define	prg       (reg[0] &0x07 | reg[0] >>3 &0x08)
#define chr       (reg[1] &0x07 | reg[0] >>3 &0x08)
#define dip       (reg[1] &0xC0)
#define nrom256  !(reg[0] &0x08)
#define chrMask   (reg[1] &0x10? 0: reg[1] &0x20? 1: 3)
#define mirrorH !!(reg[0] &0x10)
#define lock    !!(reg[0] &0x20)

namespace {
uint8_t		reg[2];

void	sync (void) {
	EMU->SetPRG_ROM16(0x8, prg &~nrom256);
	EMU->SetPRG_ROM16(0xC, prg | nrom256);
	EMU->SetCHR_ROM8(0x0, chr &~chrMask | Latch::data &chrMask);
	if (mirrorH)
		EMU->Mirror_H();
	else	
		EMU->Mirror_V();
	
	if (dip &ROM->dipValue) for (int bank =0x8; bank<=0xF; bank++) EMU->SetPRG_OB4(bank);
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	if (!lock) {
		reg[addr &1] =val;
		sync();
	}
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, false);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	for (auto& c: reg) c =0x00;
	Latch::reset(RESET_HARD);
	for (int bank =0x6; bank <=0x7; bank++) EMU->SetCPUWriteHandler(bank, writeReg);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset = Latch::saveLoad_D(stateMode, offset, data);
	for (auto& c: reg) SAVELOAD_BYTE(stateMode, offset, data, c);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =332;
} // namespace

MapperInfo MapperInfo_332 ={
	&mapperNum,
	_T("WS-1001"),
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