#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"
#include	"..\..\Hardware\Sound\s_FDS.h"

namespace {
void	sync (void) {
	EMU->SetPRG_ROM8(0x6,                                        Latch::data | 1);
	EMU->SetPRG_ROM8(0x8, Latch::data &1 && ~Latch::data &8? 10: Latch::data &~1);
	EMU->SetPRG_ROM8(0xA, 0xD);
	EMU->SetPRG_ROM8(0xC, 0xE);
	EMU->SetPRG_ROM8(0xE, 0xF);
	EMU->SetCHR_RAM8(0x0, 0);
	iNES_SetMirroring();
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, false);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	FCPUWrite writeCart =EMU->GetCPUWriteHandler(0x8);
	
	Latch::reset(resetType);
	/* Latch only responds from $C000-$DFFF*/
	EMU->SetCPUWriteHandler(0x8, writeCart);
	EMU->SetCPUWriteHandler(0x9, writeCart);
	EMU->SetCPUWriteHandler(0xA, writeCart);
	EMU->SetCPUWriteHandler(0xB, writeCart);
	EMU->SetCPUWriteHandler(0xE, writeCart);
	EMU->SetCPUWriteHandler(0xF, writeCart);	
	
	FDSsound::ResetBootleg(resetType);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =Latch::saveLoad_D(stateMode, offset, data);
	offset =FDSsound::SaveLoad(stateMode, offset, data);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =538;
} // namespace

MapperInfo MapperInfo_538 ={
	&mapperNum,
	_T("60-1064-16L"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	NULL,
	NULL,
	saveLoad,
	FDSsound::GetBootleg,
	NULL
};