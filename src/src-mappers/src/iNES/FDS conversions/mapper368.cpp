#include	"..\..\DLL\d_iNES.h"

namespace {
FCPURead readAPU;
FCPUWrite writeAPU;
bool counting;
uint16_t counter;
uint8_t prg;
uint8_t latch;

void	sync (void) {
	EMU->SetPRG_ROM8(0x6, 2);
	EMU->SetPRG_ROM8(0x8, 1);
	EMU->SetPRG_ROM8(0xA, 0);
	EMU->SetPRG_ROM8(0xC, prg &7);
	EMU->SetPRG_ROM8(0xE, 8);
	iNES_SetCHR_Auto8(0, 0);
}

int	MAPINT	readReg (int bank, int addr) {
	switch (addr &0x1FF) {
		case 0x122:
			return 0x8A | (latch &0x35);
		default:
			return readAPU(bank, addr);
	}
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	switch (addr &0x1FF) {
		case 0x022:
			prg =val &1? 3: val >>1 |4;
			sync();
			break;
		case 0x122:
			latch =val;
			counting =!!(val &1);
			if (!counting) {
				counter =0;
				EMU->SetIRQ(1);
			}			
			break;
		default:
			writeAPU(bank, addr, val);
			break;
	}
}

void	MAPINT	reset (RESET_TYPE ResetType) {
	if (ResetType ==RESET_HARD) {
		prg =0;
		counting =false;
		counter =0;
	}
	sync();
	iNES_SetMirroring();
	readAPU =EMU->GetCPUReadHandler(0x4);
	writeAPU =EMU->GetCPUWriteHandler(0x4);
	EMU->SetCPUReadHandler(0x4, readReg);
	EMU->SetCPUWriteHandler(0x4, writeReg);
}

void	MAPINT	cpuCycle (void) {
	if (counting && !(++counter &0xFFF)) EMU->SetIRQ(0);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, prg);
	SAVELOAD_BOOL(stateMode, offset, data, counting);
	SAVELOAD_WORD(stateMode, offset, data, counter);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t MapperNum =368;
} // namespace

MapperInfo MapperInfo_368 ={
	&MapperNum,
	_T("YUNG-08"),
	COMPAT_FULL,
	NULL,
	reset,
	NULL,
	cpuCycle,
	NULL,
	saveLoad,
	NULL,
	NULL
};
