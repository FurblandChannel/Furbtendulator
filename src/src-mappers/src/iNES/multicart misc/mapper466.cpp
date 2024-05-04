#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
uint8_t		reg[2];

void	sync (void) {
	uint8_t prg     =reg[1] <<5 | reg[0] <<1 &0x1E | reg[0] >>5 &1;
	bool    nrom    =!!(reg[0] &0x40);
	bool    nrom128 =!!(reg[0] &0x10);

	if (prg &0x20 && ROM->PRGROMSize <1024*1024)
		for (int bank =0x8; bank <=0xF; bank++) EMU->SetPRG_OB4(bank);
	else
	if (nrom) {
		EMU->SetPRG_ROM16(0x8, prg &~!nrom128);
		EMU->SetPRG_ROM16(0xC, prg | !nrom128);
	} else {
		EMU->SetPRG_ROM16(0x8, prg &~7 | Latch::data &7);
		EMU->SetPRG_ROM16(0xC, prg &~7 |              7);
	}
	EMU->SetPRG_RAM8(0x6, 0x00);
	EMU->SetCHR_RAM8(0x0, 0x00);
	
	if (reg[0] &0x80)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	reg[addr >>11 &1] =addr &0xFF;
	sync();
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, false);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	for (auto& c: reg) c =0;
	Latch::reset(resetType);
	EMU->SetCPUWriteHandler(0x5, writeReg);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =Latch::saveLoad_D(stateMode, offset, data);
	for (auto& c: reg) SAVELOAD_WORD(stateMode, offset, data, c);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t MapperNum =466;
} // namespace

MapperInfo MapperInfo_466 = {
	&MapperNum,
	_T("Keybyte Computer"),
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