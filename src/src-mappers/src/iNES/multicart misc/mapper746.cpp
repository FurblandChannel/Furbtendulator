#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
void	sync (void) {
	EMU->SetCHR_RAM8(0, 0);
	if (Latch::addr &0x20) {
		protectCHRRAM();
		if (~Latch::addr &0x01)
			EMU->SetPRG_ROM32(0x8, Latch::addr >>2 &0x10 | Latch::addr >>1 &0x0F);
		else {
			EMU->SetPRG_ROM16(0x8, Latch::addr >>1 &0x20 | Latch::addr &0x1F);
			EMU->SetPRG_ROM16(0xC, Latch::addr >>1 &0x20 | Latch::addr &0x1F);
		}
	} else {
		EMU->SetPRG_ROM16(0x8, Latch::addr >>1 &0x20 | Latch::addr &0x1F | Latch::data);
		EMU->SetPRG_ROM16(0xC, Latch::addr >>1 &0x20 | Latch::addr &0x1F | 7);
	}
	if (Latch::addr &0x80)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

void	MAPINT	writeLatch (int bank, int addr, int val) {
	if (Latch::addr &0x20 && ~addr &0x20)
		Latch::write(bank, addr, val);
	else
		Latch::write(bank, Latch::addr &0xC0 | addr &0x3F, val);
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, false);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	Latch::reset(RESET_HARD);
	for (int bank =0x8; bank <=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeLatch);
}

uint16_t mapperNum =746;
} // namespace

MapperInfo MapperInfo_746 = {
	&mapperNum,
	_T("ET-82"), /* 東方巨龍 80-in-1 */
	COMPAT_FULL,
	load,
	reset,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_AD,
	NULL,
	NULL
};