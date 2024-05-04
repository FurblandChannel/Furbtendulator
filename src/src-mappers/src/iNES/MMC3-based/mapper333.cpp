#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
uint8_t	Reg;

void	Sync (void) {
	uint8_t OuterBank =(Reg >>2) &3;	
	if (Reg &0x10)
		MMC3::syncPRG(0xF, OuterBank <<4);
	else
		EMU->SetPRG_ROM32(0x8, Reg);
	MMC3::syncCHR_ROM(0x7F, OuterBank <<7);
	MMC3::syncMirror();
	MMC3::syncWRAM();
}

void	MAPINT	WriteReg (int Bank, int Addr, int Val) {
	Reg =Val;
	Sync();
}

BOOL	MAPINT	Load (void) {
	MMC3::load(Sync);
	return TRUE;
}
void	MAPINT	Reset (RESET_TYPE ResetType) {
	if (ResetType == RESET_HARD) {
		/* Initialize palette to zero, otherwise selection arrow will not be seen */
		(EMU->GetCPUReadHandler(2))(2, 2);
		(EMU->GetCPUWriteHandler(2))(2, 6, 0x3F);
		(EMU->GetCPUWriteHandler(2))(2, 6, 0x00);
		for (int i =0; i <32; i++) (EMU->GetCPUWriteHandler(2))(2, 7, 0x00);
		Reg =0;
	}

	MMC3::reset(ResetType);
	for (int i =0x9; i<=0xF; i+=2) EMU->SetCPUWriteHandler(i, WriteReg);
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	offset = MMC3::saveLoad(mode, offset, data);
	SAVELOAD_BYTE(mode, offset, data, Reg);
	if (mode == STATE_LOAD) Sync();
	return offset;
}

uint16_t MapperNum =333;
} // namespace

MapperInfo MapperInfo_333 = {
	&MapperNum,
	_T("GRM070"),
	COMPAT_FULL,
	Load,
	Reset,
	NULL,
	MMC3::cpuCycle,
	MMC3::ppuCycle,
	SaveLoad,
	NULL,
	NULL
};