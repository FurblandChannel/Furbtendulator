#include	"..\DLL\d_iNES.h"

namespace {
uint8_t		mode;
uint8_t		prg[2];
uint8_t		chr[8];

#define if13                ROM->INES2_SubMapper ==1
#define	prgInvert          (if13? 0: mode <<1 &4)
#define horizontalMirroring mode &1
void	sync (void) {
	EMU->SetPRG_RAM8(0x6, 0);
	EMU->SetPRG_ROM8(0x8 ^prgInvert, prg[0]);
	EMU->SetPRG_ROM8(0xA,            prg[1]);
	EMU->SetPRG_ROM8(0xC ^prgInvert, 0xFE);
	EMU->SetPRG_ROM8(0xE,            0xFF);
	
	for (int bank =0; bank <8; bank++) EMU->SetCHR_ROM1(bank, chr[bank]);
	
	if (if13)
		EMU->Mirror_S1();
	else
	if (horizontalMirroring)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

void	MAPINT	writePRG (int bank, int addr, int val) {
	prg[bank >>1 &1] =val;
	sync();
}

void	MAPINT	writeMode (int bank, int addr, int val) {
	mode =val;
	sync();
}

void	MAPINT	writeCHR (int bank, int addr, int val) {
	chr[addr &7] =val;
	sync();
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		mode =0;
		for (int bank =0; bank <2; bank++) prg[bank] =bank;
		for (int bank =0; bank <8; bank++) chr[bank] =bank;
	}
	sync();
	
	EMU->SetCPUWriteHandler(0x8, writePRG);
	EMU->SetCPUWriteHandler(0x9, writeMode);
	EMU->SetCPUWriteHandler(0xA, writePRG);
	EMU->SetCPUWriteHandler(0xB, writeCHR);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, mode);
	for (auto& c: prg) SAVELOAD_BYTE(stateMode, offset, data, c);
	for (auto& c: chr) SAVELOAD_BYTE(stateMode, offset, data, c);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =32;
} // namespace

MapperInfo MapperInfo_032 ={
	&mapperNum,
	_T("Irem G-101"),
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
