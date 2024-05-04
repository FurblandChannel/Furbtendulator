#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\Sound\s_FDS.h"

namespace {
uint8_t index, Reg[8];

void	Sync() {
	EMU->SetPRG_ROM8(0x6, 0xFE);
	EMU->SetPRG_ROM8(0x8, Reg[6]);
	EMU->SetPRG_ROM8(0xA, Reg[7]);
	EMU->SetPRG_RAM8(0xC, 0);
	EMU->SetPRG_ROM8(0xE, 0xFF);
	EMU->SetCHR_RAM8(0, 0);
}

void	MAPINT	Write (int Bank, int Addr, int Val) {
	if (~Addr &1)
		index =Val &7;
	else {
		Reg[index] =Val;
		Sync();
	}
		
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	iNES_SetMirroring();
	if (ResetType ==RESET_HARD) {
		index =0;
		for (int i =0; i <8; i++) Reg[i] =0;
	}
	for (int i =0x8; i<=0x9; i++) EMU->SetCPUWriteHandler(i, Write);
	FDSsound::ResetBootleg(ResetType);
	Sync();
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(mode, offset, data, index);
	for (int i =0; i <8; i++) SAVELOAD_BYTE(mode, offset, data, Reg[i]);
	offset = FDSsound::SaveLoad(mode, offset, data);
	return offset;
}

uint16_t MapperNum =522;
} // namespace

MapperInfo MapperInfo_522 = {
	&MapperNum,
	_T("Whirlwind Manu LH10"),
	COMPAT_FULL,
	NULL,
	Reset,
	NULL,
	NULL,
	NULL,
	SaveLoad,
	FDSsound::GetBootleg,
	NULL
};