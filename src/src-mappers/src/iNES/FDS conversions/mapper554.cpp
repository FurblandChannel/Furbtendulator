#include	"..\..\DLL\d_iNES.h"

namespace {
uint8_t		reg;
FCPURead	readCart;

void	sync (void) {
	EMU->SetPRG_ROM8(0x6, reg);
	EMU->SetPRG_ROM8(0x8, 0xA);
	EMU->SetPRG_ROM8(0xA, 0xB);
	EMU->SetPRG_ROM8(0xC, 0x6);
	EMU->SetPRG_ROM8(0xE, 0x7);
	iNES_SetCHR_Auto8(0x0, reg);
	iNES_SetMirroring();
}

int	MAPINT	trapCartReadC (int bank, int addr) {
	if (addr >=0xAB6 && addr <=0xAD7) {
		reg =addr >>2 &0xF;
		sync();
	}
	return readCart(bank, addr);
}

int	MAPINT	trapCartReadE (int bank, int addr) {
	if ((addr &~1) ==0xBE2 || (addr &~1) ==0xE32) {
		reg =addr >>2 &0xF;
		sync();
	}
	return readCart(bank, addr);
}

int	MAPINT	trapCartReadF (int bank, int addr) {
	if ((addr &~1) ==0xFFC) {
		reg =addr >>2 &0xF;
		sync();
	}
	return readCart(bank, addr);
}

void	MAPINT	reset (RESET_TYPE resetType) {
	reg =0;
	sync();
	readCart =EMU->GetCPUReadHandler(0xC);	
	EMU->SetCPUReadHandler(0xC, trapCartReadC);
	EMU->SetCPUReadHandler(0xE, trapCartReadE);
	EMU->SetCPUReadHandler(0xF, trapCartReadF);
	EMU->SetCPUReadHandlerDebug(0xC, readCart);
	EMU->SetCPUReadHandlerDebug(0xE, readCart);
	EMU->SetCPUReadHandlerDebug(0xF, readCart);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t MapperNum =554;
} // namespace

MapperInfo MapperInfo_554 ={
	&MapperNum,
	_T("Kaiser KS-7010"),
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