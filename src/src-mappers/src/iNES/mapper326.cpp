#include	"..\DLL\d_iNES.h"

namespace {
uint8_t	PRG[4], CHR[16];

void	Sync (void) {
	for (int i =0; i <4; i++) EMU->SetPRG_ROM8(8 +i*2, PRG[i]);
	for (int i =0; i <8; i++) EMU->SetCHR_ROM1(i, CHR[i]);
	for (int i =8; i<16; i++) EMU->SetCHR_NT1 (i, CHR[i] &1);
}

void	MAPINT	Write (int Bank, int Addr, int Val) {
	Addr |=Bank <<12;
	switch(Addr &0xE010) {
		case 0x8000: PRG[0] =Val; break;
		case 0xA000: PRG[1] =Val; break;
		case 0xC000: PRG[2] =Val; break;
	}
	if ((Addr &0x8010) ==0x8010) CHR[Addr &0xF] =Val;
	Sync();
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	if (ResetType ==RESET_HARD) {
		for (int i =0; i <4;  i++) PRG[i] =0xFC |i;
		for (int i =0; i <16; i++) CHR[i] =i;
	}
	for (int i =0x8; i<=0xF; i++) EMU->SetCPUWriteHandler(i, Write);
	Sync();
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	for (int i =0; i <4; i++) SAVELOAD_BYTE(mode, offset, data, PRG[i]);
	for (int i =0; i<16; i++) SAVELOAD_BYTE(mode, offset, data, CHR[i]);
	if (mode ==STATE_LOAD) Sync();
	return offset;
}

uint16_t MapperNum =326;
} // namespace

MapperInfo MapperInfo_326 = {
	&MapperNum,
	_T("Gryzor Bootleg"),
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