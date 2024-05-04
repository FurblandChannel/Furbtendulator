#include	"..\..\DLL\d_iNES.h"

namespace {
uint8_t	PRG[4], CHR[4], Mirroring;

void	Sync (void) {
	for (int i =0; i <4; i++) {
		EMU->SetPRG_ROM8(8 +i*2, PRG[i]);
		EMU->SetCHR_ROM2(0 +i*2, CHR[i]);
	}
	if (Mirroring)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

void	MAPINT	WriteCHR (int Bank, int Addr, int Val) {
	CHR[Addr >>10] =Addr &0x1F;
	Sync();
}

void	MAPINT	WritePRG (int Bank, int Addr, int Val) {
	if (Addr &ROM->dipValue) {
		PRG[Addr >>10] =Addr &0x0F;
		Sync();
	}
}

void	MAPINT	WriteMirroring (int Bank, int Addr, int Val) {
	Mirroring =Addr &0x1;
	Sync();
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	EMU->SetCPUWriteHandler(0x8, WriteCHR);
	EMU->SetCPUWriteHandler(0x9, WriteCHR);
	EMU->SetCPUWriteHandler(0xA, WritePRG);
	EMU->SetCPUWriteHandler(0xB, WritePRG);
	EMU->SetCPUWriteHandler(0xC, WriteMirroring);
	if (ResetType ==RESET_HARD) for (int i =0; i <4; i++) {
		PRG[i] =0xC |i;
		CHR[i] =0;
		Mirroring =0;
		if (!ROM->dipValueSet) ROM->dipValue =0x010;
	}
	Sync();
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	for (int i = 0; i < 4; i++) SAVELOAD_BYTE(mode, offset, data, PRG[i]);
	for (int i = 0; i < 4; i++) SAVELOAD_BYTE(mode, offset, data, CHR[i]);
	SAVELOAD_BYTE(mode, offset, data, Mirroring);
	if (mode == STATE_LOAD) Sync();
	return offset;
}

uint16_t MapperNum =286;
} // namespace

MapperInfo MapperInfo_286 = {
	&MapperNum,
	_T("BS-5"),
	COMPAT_FULL,
	NULL,
	Reset,
	NULL,
	NULL,
	NULL,
	SaveLoad,
	NULL,
	NULL
};
