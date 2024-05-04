#include	"..\..\DLL\d_iNES.h"

#define	nrom    mode &0x02
#define mirrorH mode &0x10

namespace {
uint8_t		mode;
uint8_t		prg;
uint8_t		inner;

void	sync (void) {
	EMU->SetCHR_RAM8(0x0, 0);
	if (ROM->INES2_SubMapper ==1) {
		EMU->SetPRG_ROM8 (0x6, prg <<1 |0x23);
		if (nrom)
			EMU->SetPRG_ROM32(0x8, prg >>1);
		else {
			EMU->SetPRG_ROM16(0x8, prg &7 | prg >>1 &0x08 | prg >>2 &0x10);
			EMU->SetPRG_ROM16(0xC, 7      | prg >>1 &0x08 | prg >>2 &0x10);
		}
		
	} else
	if (nrom) {
		EMU->SetPRG_ROM8 (0x6, prg <<2 &0x1C |0x23);
		EMU->SetPRG_ROM32(0x8, prg);
	} else {
		EMU->SetPRG_ROM8 (0x6, prg <<2 &0x10 |0x2F);
		EMU->SetPRG_ROM16(0x8, prg <<1 | prg >>4);
		EMU->SetPRG_ROM16(0xC, prg <<1 | 7      );
	}
	if (mirrorH)
		EMU->Mirror_H();
	else	
		EMU->Mirror_V();
}

void	MAPINT	writeMode (int bank, int addr, int val) {
	mode =val;
	sync();
}
void	MAPINT	writePRG (int bank, int addr, int val) {
	prg =val;
	sync();
}

void	MAPINT	reset (RESET_TYPE ResetType) {
	mode =2;
	prg =0;
	sync();
	for (int bank =0x6; bank<=0x7; bank++) EMU->SetCPUWriteHandler(bank, writeMode);
	for (int bank =0x8; bank<=0xF; bank++) EMU->SetCPUWriteHandler(bank, writePRG);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, mode);
	SAVELOAD_BYTE(stateMode, offset, data, prg);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =51;
} // namespace

MapperInfo MapperInfo_051 = {
	&mapperNum,
	_T("820718C"),
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
