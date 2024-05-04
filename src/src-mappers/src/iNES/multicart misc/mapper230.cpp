#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
uint8_t	Mode;
FCPURead _Read;

void	Sync (void) {
	if (Mode ==0) {		// Contra mode
		EMU->SetPRG_ROM16(0x8, Latch::data &7);
		EMU->SetPRG_ROM16(0xC, 7);
		EMU->Mirror_V();
	} else {		// 22 games mode
		if (Latch::data &0x20) {	// NROM-128
			EMU->SetPRG_ROM16(0x8, (Latch::data &0x1F) +8);
			EMU->SetPRG_ROM16(0xC, (Latch::data &0x1F) +8);
		} else			// NROM-256
			EMU->SetPRG_ROM32(0x8,((Latch::data &0x1F) +8) >>1);

		// Mirroring
		if (Latch::data &0x40)
			EMU->Mirror_V();
		else
			EMU->Mirror_H();
	}
	EMU->SetCHR_RAM8(0, 0);
}

BOOL	MAPINT	Load (void) {
	Latch::load(Sync, FALSE);
	return TRUE;
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	if (ResetType ==RESET_HARD)
		Mode =0;
	else
		Mode ^=1;
	Latch::reset(RESET_HARD);
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(mode, offset, data, Mode);
	offset = Latch::saveLoad_D(mode, offset, data);
	if (mode == STATE_LOAD) Sync();
	return offset;
}

uint16_t MapperNum230 =230;
} // namespace

MapperInfo MapperInfo_230 ={
	&MapperNum230,
	_T("CTC-43A"),
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