#include	"..\DLL\d_iNES.h"

namespace {
uint8_t		index;
uint8_t		reg[8];
uint8_t		chrA18;
uint8_t		mirroring;

void	sync (void) {
	EMU->SetPRG_RAM8 (0x6, 0x00);
	EMU->SetPRG_ROM8 (0x8, reg[0]);
	EMU->SetPRG_ROM8 (0xA, reg[1]);
	EMU->SetPRG_ROM8 (0xC, 0xFE);
	EMU->SetPRG_ROM8 (0xE, 0xFF);

	EMU->SetCHR_ROM2(0x0, reg[2] >>1);
	EMU->SetCHR_ROM2(0x2, reg[3] >>1);
	EMU->SetCHR_ROM1(0x4, reg[4] | chrA18 <<4 &0x100);
	EMU->SetCHR_ROM1(0x5, reg[5] | chrA18 <<3 &0x100);
	EMU->SetCHR_ROM1(0x6, reg[6] | chrA18 <<2 &0x100);
	EMU->SetCHR_ROM1(0x7, reg[7] | chrA18 <<1 &0x100);

	if (mirroring &1)
		EMU->Mirror_H();
	else	
		EMU->Mirror_V();
}

void	MAPINT	writeIndex (int bank, int addr, int val) {
	index =val;
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	reg[index &7] =val;
	sync();
}

void	MAPINT	writeCHRA18 (int bank, int addr, int val) {
	chrA18 =val;
	sync();
}


void	MAPINT	writeMirroring (int bank, int addr, int val) {
	mirroring =val;
	sync();
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		index =0;
		reg[0] =0x00;
		reg[1] =0x01;
		reg[2] =0x00;
		reg[3] =0x02;
		reg[4] =0x04;
		reg[5] =0x05;
		reg[6] =0x06;
		reg[7] =0x07;
		chrA18 =0;
		mirroring =0;
	}
	sync();
	
	for (int i =0x8; i<=0x9; i++) EMU->SetCPUWriteHandler(i, writeIndex);
	for (int i =0xA; i<=0xB; i++) EMU->SetCPUWriteHandler(i, writeReg);
	for (int i =0xC; i<=0xD; i++) EMU->SetCPUWriteHandler(i, writeCHRA18);
	for (int i =0xE; i<=0xF; i++) EMU->SetCPUWriteHandler(i, writeMirroring);	
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, index);
	for (int i =0; i <8; i++) SAVELOAD_BYTE(stateMode, offset, data, reg[i]);
	SAVELOAD_BYTE(stateMode, offset, data, chrA18);
	SAVELOAD_BYTE(stateMode, offset, data, mirroring);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =112;
} // namespace

MapperInfo MapperInfo_112 ={
	&mapperNum,
	_T("NTDEC MMC3"),
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