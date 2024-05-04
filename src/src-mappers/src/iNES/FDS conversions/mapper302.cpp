#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_VRC24.h"

namespace {
void	Sync (void) {
	EMU->SetPRG_ROM8 (0xA, 0x0D);
	EMU->SetPRG_ROM16(0xC, 0x07);
	EMU->SetCHR_RAM8(0, 0);
	VRC24::syncMirror();
}

int	MAPINT	Read (int Bank, int Addr) {
	int index= ((Bank -6) <<1) | ((Addr &0x800)? 1: 0);
	index ^=4;
	return ROM->PRGROMData[(VRC24::chr[index] <<11) | (Addr &0x7FF)];
	
}

BOOL	MAPINT	Load (void) {
	VRC24::load(Sync, false, 0x01, 0x02, NULL, true, 0);
	return TRUE;
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	VRC24::reset(ResetType);

	for (int i =0x6; i<=0x9; i++) EMU->SetCPUReadHandler(i, Read);
	for (int i =0x6; i<=0x9; i++) EMU->SetCPUReadHandlerDebug(i, Read);
}

uint16_t MapperNum =302;
} // namespace

MapperInfo MapperInfo_302 = {
	&MapperNum,
	_T("Kaiser KS-7057"),
	COMPAT_FULL,
	Load,
	Reset,
	NULL,
	VRC24::cpuCycle,
	NULL,
	VRC24::saveLoad,
	NULL,
	NULL
};