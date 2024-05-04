#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
FCPURead readCart;

void	Sync (void) {
	EMU->SetPRG_ROM16(0x8, Latch::addr);
	EMU->SetPRG_ROM16(0xC, Latch::addr);
	iNES_SetCHR_Auto8(0, Latch::addr);
	if (Latch::addr &(ROM->INES2_SubMapper ==1? 0x04: 0x08))
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
	Latch::reset(ResetType);
	readCart =EMU->GetCPUReadHandler(8);
	for (int i =0x8; i <=0xF; i++) EMU->SetCPUReadHandler(i, interceptCartRead);
}

uint16_t MapperNum =200;	// variant with 16-/32-KiB switch in Mapper 204
} // namespace

MapperInfo MapperInfo_200 ={
	&MapperNum,
	_T("36-in-1"),
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