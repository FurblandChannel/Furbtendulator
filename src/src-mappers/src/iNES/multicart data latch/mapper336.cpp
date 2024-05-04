#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
void	Sync (void) {
	EMU->SetPRG_ROM16(0x8, Latch::data);
	EMU->SetPRG_ROM16(0xC, Latch::data |7);
	EMU->SetCHR_RAM8(0, 0);
}

void	MAPINT	writeLatch (int bank, int _addr, int val) {
	val |=EMU->GetCPUReadHandler(bank)(bank, _addr);
	Latch::data =val;
	Latch::addr =bank <<12 |_addr;
	Sync();
}

BOOL	MAPINT	Load (void) {
	Latch::load(Sync, FALSE);
	return TRUE;
}
void	MAPINT	Reset (RESET_TYPE ResetType) {
	iNES_SetMirroring();
	Latch::reset(RESET_HARD);
	for (int bank =0x8; bank <=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeLatch);
}

uint16_t MapperNum =336;
} // namespace

MapperInfo MapperInfo_336 = {
	&MapperNum,
	_T("K-3046"),
	COMPAT_FULL,
	Load,
	Reset,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_D,
	NULL,
	NULL
};