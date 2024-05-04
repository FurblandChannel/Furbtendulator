#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_N118.h"

namespace Submapper2 {
uint8_t		mirroring;
FCPUWrite	writeAPU;

void	sync (void) {
	EMU->SetPRG_ROM8(0x6, N118::reg[5] >>1 &0xF);
	EMU->SetPRG_ROM32(0x8, (ROM->INES_PRGSize &0xF)? 0x4: 0x7); // Two PRG-ROM chips (128+32 KiB), allow for unpadded and padded-to-power-of-two ROM file
	EMU->SetCHR_RAM8(0x0, 0);
	if (mirroring &8)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

void	MAPINT	writeMirroring (int bank, int addr, int val) {
	writeAPU(bank, addr, val);
	if (addr ==0x025) {
		mirroring =val;
		sync();
	}
}

BOOL	MAPINT	load (void) {
	N118::load(sync);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) mirroring =0;
	N118::reset(resetType);
	writeAPU =EMU->GetCPUWriteHandler(0x4);
	EMU->SetCPUWriteHandler(0x4, writeMirroring);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =N118::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, mirroring);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t MapperNum =42;
MapperInfo MapperInfo_042 ={
	&MapperNum,
	_T("Kaiser KS-018"),
	COMPAT_FULL,
	load,
	reset,
	::unload,
	NULL,
	NULL,
	saveLoad,
	NULL,
	NULL,
};
} // namespace

