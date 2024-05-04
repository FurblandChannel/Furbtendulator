#include	"..\..\DLL\d_iNES.h"

namespace {
FCPUWrite	writeAPU;
uint8_t		reg[4];

void	sync (void) {
	if (reg[2] &0x01) {
		EMU->SetPRG_ROM16(0x8, reg[0] >>1);
		EMU->SetPRG_ROM16(0xC, reg[0] >>1);
	} else
		EMU->SetPRG_ROM32(0x8, reg[0] >>2); // The reg[2] bits probably can select other modes, but we don't know anything about them.
	EMU->SetCHR_RAM8(0, reg[1]);
	if (reg[2] &0x10) {
		EMU->Mirror_H();
		for (int bank =0; bank <8; bank++) EMU->SetCHR_Ptr1(bank, EMU->GetCHR_Ptr1(bank), FALSE); // Prevent graphics corruption in Mighty Bomb Jack by write-protecting CHR-RAM. Need to check whether real cartridge does this as well.
	} else
		EMU->Mirror_V();
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	if (bank ==0x4) writeAPU(bank, addr, val);
	if (addr &0x100) {
		reg[addr &3] =val;
		sync();
	}
}

void	MAPINT	writeCHR (int bank, int addr, int val) {
	if (reg[2] &0x04) {
		reg[1] =val;
		sync();
	}
}

void	MAPINT	reset (RESET_TYPE resetType) {
	for (auto& c: reg) c =0;
	sync();

	writeAPU =EMU->GetCPUWriteHandler(0x4);
	for (int bank =0x4; bank <=0x5; bank++) EMU->SetCPUWriteHandler(bank, writeReg);
	for (int bank =0x8; bank <=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeCHR);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	for (auto& c: reg) SAVELOAD_BYTE(stateMode, offset, data, c);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =403;
} // namespace

MapperInfo MapperInfo_403 ={
	&mapperNum,
	_T("89433"),
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
