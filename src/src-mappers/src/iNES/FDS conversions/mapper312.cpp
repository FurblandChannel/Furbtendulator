#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
uint8_t	PRG;
void	Sync (void) {
	EMU->SetPRG_ROM16(0x8, PRG);
	EMU->SetPRG_ROM16(0xC, 0xFF);
	EMU->SetCHR_RAM8(0, 0);
	if (Latch::data &1)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

void	MAPINT	WritePRG (int Bank, int Addr, int Val) {
	PRG =Val;
	Sync();
}

BOOL	MAPINT	Load (void) {
	Latch::load(Sync, FALSE);
	return TRUE;
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	Latch::reset(ResetType);
	EMU->SetCPUWriteHandler(6, WritePRG);
	EMU->SetCPUWriteHandler(7, WritePRG);
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(mode, offset, data, PRG);
	offset = Latch::saveLoad_D(mode, offset, data);
	if (mode == STATE_LOAD) Sync();
	return offset;
}

uint16_t MapperNum =312;
} // namespace

MapperInfo MapperInfo_312 = {
	&MapperNum,
	_T("Kaiser KS-7013B"),
	COMPAT_FULL,
	Load,
	Reset,
	NULL,
	NULL,
	NULL,
	SaveLoad,
	NULL,
	NULL
};