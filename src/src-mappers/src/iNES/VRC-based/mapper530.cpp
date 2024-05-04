#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_VRC24.h"

namespace {
void	Sync (void) {
	VRC24::syncPRG(0x1F, 0x00);
	VRC24::syncCHR_ROM(0x1FF, 0x00);
	EMU->SetPRG_RAM8(0x6, 0);
	VRC24::syncMirror();
}

void	MAPINT	Write (int Bank, int Addr, int Val) {
	Bank |=(Addr &8) >>3;
	switch (Bank) {
		case 0x9: VRC24::writeMisc(Bank, Addr, Val); break;
		case 0x8:
		case 0xA: VRC24::writePRG(Bank, Addr, ((Val &0x2) <<2) | ((Val &0x8) >>2) | (Val &0x5)); break;
		case 0xB:
		case 0xC:
		case 0xD:
		case 0xE: if (Addr &1)
				VRC24::writeCHR(Bank, Addr, ((Val &0x4) >>1) | ((Val &0x2) <<1) | (Val &0x9));
			  else
				VRC24::writeCHR(Bank, Addr, Val);
			  break;
		case 0xF: VRC24::writeIRQ(Bank, Addr, Val); break;
	}
}

BOOL	MAPINT	Load (void) {
	iNES_SetSRAM();
	VRC24::load(Sync, true, 0x01, 0x02, NULL, true, 0);
	return TRUE;
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	VRC24::reset(ResetType);
	for (int i =0x8; i<=0xF; i++) EMU->SetCPUWriteHandler(i, Write);
}

uint16_t MapperNum =530;
} // namespace

MapperInfo MapperInfo_530 = {
	&MapperNum,
	_T("AX5705"),
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