#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
FCPURead readCart;

void	Sync (void) {
	EMU->SetPRG_ROM16(0x8, Latch::addr);
	EMU->SetPRG_ROM16(0xC, Latch::addr);
	EMU->SetCHR_ROM8(0, Latch::addr);
	if (Latch::addr &0x08)
		EMU->Mirror_V();
	else
		EMU->Mirror_H();
}

int	MAPINT	interceptCartRead (int Bank, int Addr) {
	switch (Latch::addr &0xFF0F) {
		case 0xF004:
			if (ROM->PRGROMSize <=65536)
				return ROM->dipValue &0x00FF;
			else
				return readCart(Bank, Addr);			
		case 0xF008:
			return (ROM->dipValue &0xFF00) >>8;
		default:
			return readCart(Bank, Addr);
	}
}

BOOL	MAPINT	Load (void) {
	Latch::load(Sync, FALSE);
	return TRUE;
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	Latch::reset(ResetType);
	readCart =EMU->GetCPUReadHandler(8);
	for (int i =0x8; i <=0xF; i++) EMU->SetCPUReadHandler(i, interceptCartRead);
}

uint16_t MapperNum =338;
} // namespace

MapperInfo MapperInfo_338 ={
	&MapperNum,
	_T("SA005-A"),
	COMPAT_FULL,
	Load,
	Reset,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_AL,
	NULL,
	NULL
};