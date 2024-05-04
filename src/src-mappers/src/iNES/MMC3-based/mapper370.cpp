#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
uint8_t 	prgAND;
uint8_t		prgOR;
uint8_t		chrAND;
uint16_t	chrOR;

void	sync (void) {
	MMC3::syncWRAM();
	MMC3::syncPRG(prgAND, prgOR);
	MMC3::syncCHR(chrAND, chrOR);
	if (chrOR ==0x080)
		MMC3::syncMirrorA17();
	else
		MMC3::syncMirror();
}

int	MAPINT	readDIP (int bank, int addr) {
	return (ROM->dipValue &0x80) | (*EMU->OpenBus &~0x80);
}

void	MAPINT	writeOuterBank (int bank, int addr, int val) {
	prgOR =(addr &0x38) <<1;
	chrOR =(addr &0x07) <<7;
	prgAND =addr &0x20? 0x0F: 0x1F;
	chrAND =addr &0x04? 0xFF: 0x7F;
	sync();
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync);
	iNES_SetSRAM();
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	prgAND =0x1F; prgOR =0x00;
	chrAND =0x7F; chrOR =0x00;
	MMC3::reset(RESET_HARD);
	EMU->SetCPUReadHandler(0x5, readDIP);
	EMU->SetCPUReadHandlerDebug(0x5, readDIP);
	EMU->SetCPUWriteHandler(0x5, writeOuterBank);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, prgAND);
	SAVELOAD_BYTE(stateMode, offset, data, prgOR);
	SAVELOAD_BYTE(stateMode, offset, data, chrAND);
	SAVELOAD_WORD(stateMode, offset, data, chrOR);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =370;
} // namespace

MapperInfo MapperInfo_370 ={
	&mapperNum,
	_T("F600"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	MMC3::cpuCycle,
	MMC3::ppuCycle,
	saveLoad,
	NULL,
	NULL
};
