/* 	Subor Type 2

	Mapper 514: 32K PRG Banking, Combined register at $8000.
	Mapper 518: 16K/32K PRG Banking, Two registers at $5000 and $5200.
	
	These games must be run in "Hybrid" (a.k.a. Dendy) mode;
	Subor 5.0 will crash in NTSC mode; Subor Karaoke will have a glitched title screen.
*/

#include	"..\DLL\d_iNES.h"

namespace {
uint8_t ModeBank;
FPPURead _ReadNTRAM;

#define MODE_HORIZONTAL_MIRRORING 0x40
#define MODE_BANKSWITCH_CHR 0x80

void	Sync (void) {
	EMU->SetPRG_RAM8(0x6, 0);
	if (ModeBank &MODE_HORIZONTAL_MIRRORING)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();

	EMU->SetPRG_ROM32(0x8, ModeBank &0x3F);
}

BOOL	MAPINT	Load (void) {
	iNES_SetSRAM();
	return TRUE;
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(mode, offset, data, ModeBank);
	if (mode == STATE_LOAD)	Sync();
	return offset;
}

int	MAPINT	ReadNTRAM (int Bank, int Addr) {
	if (ModeBank &MODE_BANKSWITCH_CHR && Bank &((ModeBank &MODE_HORIZONTAL_MIRRORING)? 2: 1))
		EMU->SetCHR_RAM4(0, 1);
	else
		EMU->SetCHR_RAM4(0, 0);
	return _ReadNTRAM(Bank, Addr);
}

void	MAPINT	Write8(int Bank, int Addr, int Val) {
	if (Addr ==0) {
		ModeBank =Val;
		Sync();
	}
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	_ReadNTRAM =EMU->GetPPUReadHandler(8);
	for (int i =8; i<12; i++) EMU->SetPPUReadHandler(i, ReadNTRAM);
	EMU->SetCPUWriteHandler(0x8, Write8);

	ModeBank =0;
	EMU->SetCHR_RAM8(0, 0);
	Sync();
}

uint16_t MapperNum = 514;
} // namespace

MapperInfo MapperInfo_514 = {
	&MapperNum,
	_T("Subor Type 2a (32 KiB Banking)"),
	COMPAT_FULL,
	Load,
	Reset,
	NULL,
	NULL,
	NULL,
	SaveLoad,
	NULL,
	NULL
};