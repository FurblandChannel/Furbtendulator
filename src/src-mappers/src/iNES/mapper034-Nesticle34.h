#include	"..\DLL\d_iNES.h"

namespace Nesticle34 {
uint8_t		reg[3];
FCPUWrite	writeWRAM;

void	sync (void) {
	EMU->SetPRG_RAM8(0x6, 0);
	EMU->SetPRG_ROM32(0x8, reg[0]);
	EMU->SetCHR_ROM4(0, reg[1]);
	EMU->SetCHR_ROM4(4, reg[2]);
	iNES_SetMirroring();
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	if (bank <=0x7) writeWRAM(bank, addr, val);
	if (bank ==0x7 && addr >=0xFFD) {
		reg[addr -0xFFD] =val;
		sync();
	} else
	if (bank >=0x8) {
		reg[0] =val;
		sync();
	}
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) for (auto& c: reg) c =0;
	sync();
	writeWRAM =EMU->GetCPUWriteHandler(0x6);
	for (int bank =0x6; bank <=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeReg);
	
	if (ROM->MiscROMSize && ROM->MiscROMData) for (size_t i =0; i <ROM->MiscROMSize; i++) (EMU->GetCPUWriteHandler(0x7))(0x7, i, ROM->MiscROMData[i]);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	for (auto& c: reg) SAVELOAD_BYTE(stateMode, offset, data, c);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =34;

MapperInfo MapperInfo_Nesticle34 ={
	&mapperNum,
	_T("Nesticle mapper 34"),
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
} // namespace
