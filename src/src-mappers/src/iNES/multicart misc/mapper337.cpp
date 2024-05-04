#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
uint16_t Reg;

void	Sync (void) {
	EMU->SetPRG_ROM8(0x6, 1);
	if (Latch::addr &0x4000)
		Reg =(Reg &~7) | (Latch::data & 7);
	else
		Reg =(Reg & 7) | (Latch::data &~7);

	switch (Reg >>6) {
		case 0:	EMU->SetPRG_ROM16(0x8, Reg);
			EMU->SetPRG_ROM16(0xC, Reg);
			break;
		case 1:	EMU->SetPRG_ROM32(0x8, Reg >>1);
			break;
		default:EMU->SetPRG_ROM16(0x8, Reg);
			EMU->SetPRG_ROM16(0xC, Reg |7);
			break;
	}
	EMU->SetCHR_RAM8(0, 0);
	if (~Reg &0x80) for (int i =0; i <8; i++) EMU->SetCHR_Ptr1(i, EMU->GetCHR_Ptr1(i), FALSE);
	if (Reg &0x20)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

BOOL	MAPINT	Load (void) {
	Latch::load(Sync, FALSE);
	return TRUE;
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	if (ResetType ==RESET_HARD) Reg =0;
	Latch::reset(ResetType);
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	offset = Latch::saveLoad_D(mode, offset, data);
	SAVELOAD_WORD(mode, offset, data, Reg);
	if (mode == STATE_LOAD) Sync();
	return offset;
}

uint16_t MapperNum =337;
} // namespace

MapperInfo MapperInfo_337 = {
	&MapperNum,
	_T("CTC-12IN1"),
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