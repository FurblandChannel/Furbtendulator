#include	"h_FDS.h"
#include	"Sound\s_FDS.h"

namespace FDS {
FCPURead	passCPURead;
FCPURead	passCPUReadDebug;
FCPUWrite	passCPUWrite;
bool		horizontalMirroring;

void	sync (void) {
	EMU->SetPRG_RAM32(0x6, 0);
	EMU->SetCHR_RAM8(0, 0);
	EMU->SetPRG_ROM8(0xE, 0);
	if (horizontalMirroring)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

int	MAPINT	readReg (int bank, int addr) {
	if (addr >=0x40 && addr <=0x92)
		return FDSsound::Read((bank <<12) |addr);
	else
		return passCPURead(bank, addr);
}

int	MAPINT	readRegDebug (int bank, int addr) {
	if (addr >=0x40 && addr <=0x92)
		return FDSsound::Read((bank <<12) |addr);
	else
		return passCPUReadDebug(bank, addr);
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	passCPUWrite(bank, addr, val);

	if (addr ==0x25) {
		horizontalMirroring =!!(val &0x08);
		sync();
	} else
	if (addr >=0x40 && addr <=0x92)
		FDSsound::Write((bank <<12) |addr, val);
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) horizontalMirroring =false;
	sync();

	FDSsound::Reset(resetType);
	passCPURead      =EMU->GetCPUReadHandler     (0x4);
	passCPUReadDebug =EMU->GetCPUReadHandlerDebug(0x4);
	passCPUWrite     =EMU->GetCPUWriteHandler    (0x4);
	EMU->SetCPUReadHandler     (0x4, readReg);
	EMU->SetCPUReadHandlerDebug(0x4, readRegDebug);
	EMU->SetCPUWriteHandler    (0x4, writeReg);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BOOL(stateMode, offset, data, horizontalMirroring);
	offset =FDSsound::SaveLoad(stateMode, offset, data);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

int	MAPINT	mapperSnd (int Len) {
	return FDSsound::Get(Len);
}

} // namespace FDS