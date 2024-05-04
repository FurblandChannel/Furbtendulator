#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
uint8_t		outerBank;
uint8_t		latch;

void	sync (void) {
	if (outerBank &0x20) {
		if (outerBank &0x10) {
			EMU->SetPRG_ROM16(0x8, latch &7 | outerBank <<3);
			EMU->SetPRG_ROM16(0xC,        7 | outerBank <<3);
		} else
			EMU->SetPRG_ROM32(0x8, MMC3::getPRGBank(0) >>2 &3 | outerBank <<2);
	} else
		MMC3::syncPRG(0x0F, outerBank <<4);
	if (outerBank &0x08)
		EMU->SetCHR_RAM8(0x0, 0);
	else
		MMC3::syncCHR(0xFF, outerBank <<8);
	MMC3::syncMirror();
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync);
	return TRUE;
}

void	MAPINT	writeOuterBank (int bank, int addr, int val) {
	outerBank =addr &0xFF;
	sync();
}

void	MAPINT	writeLatch (int bank, int addr, int val) {
	latch =val;
	MMC3::write(bank, addr, val);
	sync();
}

void	MAPINT	reset (RESET_TYPE resetType) {	
	outerBank =0;
	MMC3::reset(resetType);	
	MMC3::setWRAMCallback(NULL, writeOuterBank);
	for (int bank =0x8; bank<=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeLatch);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, outerBank);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =393;
} // namespace

MapperInfo MapperInfo_393 ={
	&mapperNum,
	_T("820720C"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	MMC3::cpuCycle,
	MMC3::ppuCycle,
	MMC3::saveLoad,
	NULL,
	NULL
};