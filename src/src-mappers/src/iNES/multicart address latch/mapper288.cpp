#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
FCPURead	readCart;

void	sync (void) {
	EMU->SetPRG_ROM32(8, Latch::addr >>3);
	iNES_SetCHR_Auto8(0, Latch::addr &7);
}

int	MAPINT	readDIP (int bank, int addr) {
	if (Latch::addr &0x20)
		return readCart(bank, addr | ROM->dipValue);
	else
		return readCart(bank, addr);
}

BOOL	MAPINT	load (void) {	
	Latch::load(sync, FALSE);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	iNES_SetMirroring();
	Latch::reset(resetType);
	readCart =EMU->GetCPUReadHandler(0x8);
	for (int bank =0x8; bank<=0xF; bank++) {
		EMU->SetCPUReadHandler(bank, readDIP);
		EMU->SetCPUReadHandlerDebug(bank, readDIP);
	}
}


uint16_t mapperNum =288;
} // namespace

MapperInfo MapperInfo_288 ={
	&mapperNum,
	_T("GKCXIN1"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_A,
	NULL,
	NULL
};
