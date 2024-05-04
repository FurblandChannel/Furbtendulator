#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
uint8_t		outerBank;

void	sync (void) {
	EMU->SetPRG_ROM16(0x8, Latch::data &7 | outerBank);
	EMU->SetPRG_ROM16(0xC,            0x7 | outerBank);
	EMU->SetCHR_RAM8(0, Latch::data >>5);
	if (outerBank ==0x80)
		iNES_SetMirroring();
	else
	if (outerBank &0x20)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

void	MAPINT	writeLED (int bank, int addr, int val) {
	ROM->dipValue =val;
}

void	MAPINT	writeOuterBank (int bank, int addr, int val) {
	if (addr &0x800) {
		outerBank =val &~7;
		sync();
	}
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, TRUE);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	outerBank =0x80;
	Latch::reset(RESET_HARD);
	EMU->SetCPUWriteHandler(0x7, writeOuterBank);
	for (int bank =0x8; bank<=0xB; bank++) EMU->SetCPUWriteHandler(bank, writeLED);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =Latch::saveLoad_D(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, outerBank);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =400;
} // namespace

MapperInfo MapperInfo_400 ={
	&mapperNum,
	_T("8BIT-XMAS"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	NULL,
	NULL,
	saveLoad,
	NULL,
	NULL
};