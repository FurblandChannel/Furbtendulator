#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
void	sync (void) {
	if (Latch::data &0x80)
		for (int bank =0x8; bank <=0xF; bank++) EMU->SetPRG_OB4(bank);
	else {
		EMU->SetPRG_ROM16(0x8, Latch::data);
		EMU->SetPRG_ROM16(0xC, Latch::data |7 | Latch::data >>1 &8);
	}
	EMU->SetCHR_RAM8(0, 0);
}

void	MAPINT	writeLatch (int bank, int addr, int val) {
	if (bank ==0xF && (addr &0xF00) ==0xE00)
		Latch::data |=0x80;
	else
	if (bank ==0xF && (addr &0xF00) ==0xF00)
		Latch::data =Latch::data &0x0F | addr <<3 &0x18;
	else
		Latch::data =Latch::data &0x18 | val &0x0F;
	sync();
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, FALSE);
	return TRUE;
}
void	MAPINT	reset (RESET_TYPE resetType) {
	iNES_SetMirroring();
	Latch::reset(RESET_HARD);
	for (int bank =0x8; bank <=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeLatch);
}

uint16_t MapperNum =759;
} // namespace

MapperInfo MapperInfo_759 = {
	&MapperNum,
	_T("5068"), /* Super New HiK 3-in-1 - 高容量強卡三合一 */
	COMPAT_FULL,
	load,
	reset,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_D,
	NULL,
	NULL
};