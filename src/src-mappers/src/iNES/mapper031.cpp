#include	"..\DLL\d_iNES.h"
namespace {
uint8_t		prg[8];
uint8_t		scratch[4096];
FCPURead	readAPU, readAPUDebug;
FCPUWrite	writeAPU;

void	sync (void) {
	EMU->SetPRG_RAM8(0x6, 0);
	for (int bank =0; bank <8; bank++) EMU->SetPRG_ROM4(0x8 |bank, prg[bank]);
	iNES_SetCHR_Auto8(0x0, 0);
	iNES_SetMirroring();
}

int	MAPINT	read4 (int bank, int addr) {
	return addr <0x20? readAPU(bank, addr): scratch[addr];
}
int	MAPINT	read4Debug (int bank, int addr) {
	return addr <0x20? readAPUDebug(bank, addr): scratch[addr];
}
void	MAPINT	write4 (int bank, int addr, int val) {
	if (addr <0x20)
		writeAPU(bank, addr, val);
	else
		scratch[addr] =val;
}

void	MAPINT	writePRG (int bank, int addr, int val) {
	prg[addr &7] =val;
	sync();
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {	
	if (resetType ==RESET_HARD) for (int bank =0; bank <8; bank++) prg[bank] =0xF8 |bank;
	sync();
	
	readAPU      =EMU->GetCPUReadHandler     (0x4);
	readAPUDebug =EMU->GetCPUReadHandlerDebug(0x4);
	writeAPU     =EMU->GetCPUWriteHandler    (0x4);
	EMU->SetCPUReadHandler     (0x4, read4);
	EMU->SetCPUReadHandlerDebug(0x4, read4Debug);
	EMU->SetCPUWriteHandler    (0x4, write4);
	EMU->SetCPUWriteHandler(0x5, writePRG);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	for (auto& c: prg) SAVELOAD_BYTE(stateMode, offset, data, c);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =31;
} // namespace

MapperInfo MapperInfo_031 ={
	&mapperNum,
	_T("2A03 Puritans Album"),
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