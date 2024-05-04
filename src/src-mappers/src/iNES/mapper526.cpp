#include	"..\DLL\d_iNES.h"

namespace {
uint8_t	PRG[4], CHR[8];
uint16_t IRQCounter;

BOOL	MAPINT	Load (void) {
	iNES_SetSRAM();
	return TRUE;
}

void	Sync (void) {
	EMU->SetPRG_RAM8(0x6, 0);
	for (int i =0; i <4; i++) EMU->SetPRG_ROM8(0x8 +i*2, PRG[i]);
	for (int i =0; i <8; i++) EMU->SetCHR_ROM1(     i  , CHR[i]);
	EMU->Mirror_V();
}

void	MAPINT	Write (int Bank, int Addr, int Val) {
	switch (Addr &0xF) {
		case 0x0: case 0x1: case 0x2: case 0x3: case 0x4: case 0x5: case 0x6: case 0x7:
			CHR[Addr &7] =Val; break;
		case 0x8: case 0x9: case 0xA: case 0xB:
			PRG[Addr &3] =Val; break;
		case 0xD:
			IRQCounter =0; break; // Don't know which one (0xD/0xF) does what because they are always written to one after the other
		case 0xF:
			EMU->SetIRQ(1); break;
		
	}
	Sync();
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	if (ResetType ==RESET_HARD) {
		for (int i =0; i <4; i++) PRG[i] =0xFC +i;
		for (int i =0; i <8; i++) CHR[i] =i;
		IRQCounter =0;
	}
	EMU->SetCPUWriteHandler(0x8, Write);
	Sync();
}

void	MAPINT	CPUCycle (void) {
	if (++IRQCounter &4096) EMU->SetIRQ(0);
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	for (int i =0; i <4; i++) SAVELOAD_BYTE(mode, offset, data, PRG[i]);
	for (int i =0; i <8; i++) SAVELOAD_BYTE(mode, offset, data, CHR[i]);
	SAVELOAD_WORD(mode, offset, data, IRQCounter);
	if (mode == STATE_LOAD)	Sync();
	return offset;
}

uint16_t MapperNum =526;
} // namespace

MapperInfo MapperInfo_526 ={
	&MapperNum,
	_T("BJ-56"),
	COMPAT_FULL,
	Load,
	Reset,
	NULL,
	CPUCycle,
	NULL,
	SaveLoad,
	NULL,
	NULL
};