#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
uint8_t		reg[2];

void	sync (void) {
	EMU->SetPRG_RAM4(0x7, 0);
	EMU->SetPRG_ROM32(0x8, (reg[0] <<2) +(Latch::data &7)); // Battletoads is 256 KiB yet starts on a 128 KiB boundary ...
	EMU->SetCHR_RAM8(0x0, 0);
	if (Latch::data &0x10)
		EMU->Mirror_S1();
	else	
		EMU->Mirror_S0();
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	if (~reg[1] &0x80) {
		reg[addr &1] =val;
		sync();
	}
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, false);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) for (auto& c: reg) c =0;
	Latch::reset(resetType);
	EMU->SetCPUWriteHandler(0x6, writeReg);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =Latch::saveLoad_D(stateMode, offset, data);
	for (auto& c: reg) SAVELOAD_BYTE(stateMode, offset, data, c);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}


uint16_t mapperNum =501;
} // namespace

MapperInfo MapperInfo_501 ={
	&mapperNum,
	_T("Yhc AXROM"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_D,
	NULL,
	NULL
};