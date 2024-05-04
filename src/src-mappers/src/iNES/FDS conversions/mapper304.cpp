#include	"..\..\DLL\d_iNES.h"

namespace {
uint8_t		prg;
FCPURead	readAPU;
FCPUWrite	writeAPU;

void	sync (void) {
	EMU->SetPRG_ROM8(0x6, prg |4);
	EMU->SetPRG_ROM32(0x8, 0);
	iNES_SetCHR_Auto8(0, 0);
}

int	MAPINT	readReg (int bank, int addr) {
	if (addr <=0x20)
		return readAPU(bank, addr);
	else
		return 0xFF;
}

void	MAPINT	writePRG (int bank, int addr, int val) {
	writeAPU(bank, addr, val);
	if (addr ==0x27) {
		prg =val &1;
		sync();
	} else
	if (addr ==0x68) EMU->SetIRQ(~val &1); // LE10 apparently drives the IRQ line directly?
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) prg =0;
	iNES_SetMirroring();
	sync();

	readAPU =EMU->GetCPUReadHandler(0x4);
	writeAPU =EMU->GetCPUWriteHandler(0x4);
	EMU->SetCPUReadHandler(0x4, readReg);
	EMU->SetCPUWriteHandler(0x4, writePRG);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, prg);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t MapperNum =304;
} // namespace

MapperInfo MapperInfo_304 ={
	&MapperNum,
	_T("09-034A"),
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