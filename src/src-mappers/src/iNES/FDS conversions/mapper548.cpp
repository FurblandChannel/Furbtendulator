#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\Sound\s_FDS.h"

namespace {
uint8_t		latch;
uint8_t		reg;
uint16_t	counter;
bool		counting;

void	sync (void) {
	EMU->SetPRG_RAM8(0x6, 0);
	
	EMU->SetPRG_ROM16(0x8, reg);
	EMU->SetPRG_ROM16(0xC, 0x3);
	EMU->SetCHR_RAM8(0, 0);

	iNES_SetMirroring();
}

void	MAPINT	writeLatch (int bank, int addr, int val) {
	FDSsound::Write4(bank, addr, val);	
	if (addr &0x800) {
		latch =addr >>2 &0x3 | addr >>3 &4;
		if (addr &4) {
			counting =false;
			counter =0;
			EMU->SetIRQ(1);
		} else
			counting =true;
	}
}

void	MAPINT	applyLatch (int bank, int addr, int val) {
	if (~addr &0x800) {
		reg =latch ^0x5;
		sync();
	}
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		latch =0x7;
		reg =latch ^0x5;
		counter =0;
		counting =false;
	}
	sync();

	FDSsound::ResetBootleg(resetType);
	EMU->SetCPUWriteHandler(0x4, writeLatch);
	EMU->SetCPUWriteHandler(0x5, applyLatch);
}

void	MAPINT	cpuCycle (void) {
	if (counting) {
		if (counter ==23680)
			EMU->SetIRQ(0);
		else
		if (counter ==24320)
			EMU->SetIRQ(1);
		counter++;
	}
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, latch);
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	SAVELOAD_WORD(stateMode, offset, data, counter);
	SAVELOAD_BOOL(stateMode, offset, data, counting);
	offset =FDSsound::SaveLoad(stateMode, offset, data);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =548;
} // namespace

MapperInfo MapperInfo_548 = {
	&mapperNum,
	_T("CTC-15"),
	COMPAT_FULL,
	NULL,
	reset,
	NULL,
	cpuCycle,
	NULL,
	saveLoad,
	FDSsound::GetBootleg,
	NULL
};