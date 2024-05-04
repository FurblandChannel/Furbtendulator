#include	"..\DLL\d_iNES.h"

namespace {
uint8_t		prg;
uint16_t	chr[8];
uint8_t		mirroring;

void	sync (void) {
	EMU->SetPRG_RAM8(0x6, 0);
	EMU->SetPRG_ROM16(0x8, prg);
	EMU->SetPRG_ROM16(0xC, 0xFF);
	for (int bank =0; bank <8; bank++) EMU->SetCHR_ROM1(bank, chr[bank]);
	switch (mirroring) {
		case 0:  EMU->Mirror_V();  break;
		case 1:  EMU->Mirror_H();  break;
		default: EMU->Mirror_S0(); break;
	}
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	return TRUE;
}

void	MAPINT	write (int bank, int addr, int val) {
	switch (addr >>2) {
		case 0: chr[0 |addr &3] =(chr[0 |addr &3] &0xFF00) | val;     break;
		case 1: chr[0 |addr &3] =(chr[0 |addr &3] &0x00FF) | val <<8; break;
		case 2: chr[4 |addr &3] =(chr[4 |addr &3] &0xFF00) | val;     break;
		case 3: chr[4 |addr &3] =(chr[4 |addr &3] &0x00FF) | val <<8; break;
		case 4: prg =val; break;
		case 5: mirroring =val &1; break;
	}
	sync();
}

void	MAPINT	reset (RESET_TYPE resetType) {
	for (int bank =0xC; bank<=0xF; bank++) EMU->SetCPUWriteHandler(bank, write);
	if (resetType ==RESET_HARD) {
		prg =0;
		for (auto& c: chr) c =0;
		mirroring =2; // Use one-screen mirroring until mirroring register is written to
	}
	sync();
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, prg);
	for (auto& c: chr) SAVELOAD_BYTE(stateMode, offset, data, c);
	SAVELOAD_BYTE(stateMode, offset, data, mirroring);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =156;
} // namespace

MapperInfo MapperInfo_156 ={
	&mapperNum,
	_T("다우 ROM Controller DIS23C01 다우 245"),
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