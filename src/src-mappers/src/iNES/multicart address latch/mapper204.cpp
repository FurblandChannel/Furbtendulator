#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
FCPURead readCart;
void	Sync (void) {
	if (Latch::addr &0x20) {
		EMU->SetPRG_ROM32(0x8, Latch::addr >>1);
		EMU->SetCHR_ROM8(0, Latch::addr &0xE);
	} else {
		EMU->SetPRG_ROM16(0x8, Latch::addr);
		EMU->SetPRG_ROM16(0xC, Latch::addr);
		EMU->SetCHR_ROM8(0, Latch::addr &0xF);
	}	
	if (Latch::addr &0x10)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
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
	iNES_SetMirroring();
	Latch::reset(ResetType);
	readCart =EMU->GetCPUReadHandler(8);
	for (int i =0x8; i <=0xF; i++) EMU->SetCPUReadHandler(i, interceptCartRead);
}

uint16_t MapperNum =204;	// Variant with just 16 KiB in Mapper 200
} // namespace

MapperInfo MapperInfo_204 =
{
	&MapperNum,
	_T("204"),
	COMPAT_FULL,
	Load,
	Reset,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_A,
	NULL,
	NULL
};
