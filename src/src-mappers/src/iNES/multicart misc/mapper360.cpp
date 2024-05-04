#include	"..\..\DLL\d_iNES.h"

namespace {
uint8_t		reg;
	
void	sync (void) {
	if (ROM->INES2_SubMapper ==0) reg =ROM->dipValue |0x20;
	if (~reg &0x20) {
		EMU->SetPRG_ROM8(0x8, 0x40);
		EMU->SetPRG_ROM8(0xA, 0x40);
		EMU->SetPRG_ROM8(0xC, 0x40);
		EMU->SetPRG_ROM8(0xE, 0x40);
	} else
	if ((reg &0x1F) <2)
		EMU->SetPRG_ROM32(0x8, reg >>1 &0x0F);
	else {
		EMU->SetPRG_ROM16(0x8, reg &0x1F);
		EMU->SetPRG_ROM16(0xC, reg &0x1F);
	}
	EMU->SetCHR_ROM8(0x0, reg);
	if (reg &0x10)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	EMU->WriteAPU(bank, addr, val);
	if (addr &0x100) {
		reg =val;
		sync();
	}
}

void	MAPINT	reset (RESET_TYPE resetType) {
	reg =0;
	sync();
	if (ROM->INES2_SubMapper ==1) EMU->SetCPUWriteHandler(0x4, writeReg);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =360;
} // namespace

MapperInfo MapperInfo_360 ={
	&mapperNum,
	_T("普澤 P3150"),
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
