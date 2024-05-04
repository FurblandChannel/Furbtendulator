#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

#define prg16k    (extraVal >>2 &0x07 | extraAddr <<1 &0x08 | extraVal >>2 &0x10)
#define prg8k     (prg16k <<1)
#define prgAND    (extraAddr &1? 0x1F: 0x0F)
#define chrAND    (extraAddr &2? 0xFF: 0x7F)
#define nrom    !!(extraVal &0x01)
#define nrom256 !!(extraVal &0x02)

namespace {
uint8_t		extraVal;
uint8_t		extraAddr;
FCPUWrite	writeAPU;

void	sync (void) {
	if (nrom) {
		EMU->SetPRG_ROM16(0x8, prg16k &~nrom256);
		EMU->SetPRG_ROM16(0xC, prg16k | nrom256);
	} else
		MMC3::syncPRG(prgAND, prg8k &~prgAND);
	MMC3::syncCHR_ROM(chrAND, prg8k <<3 &~chrAND);
	MMC3::syncMirror();
}

void	MAPINT	interceptAPUWrite (int bank, int addr, int val) {
	if (bank ==4) writeAPU(bank, addr, val);
	if (addr &0x100) {
		extraVal =val;
		extraAddr =addr &0xFF;
		sync();
	}
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	extraVal =1;
	extraAddr =0;
	MMC3::reset(resetType);	
	writeAPU =EMU->GetCPUWriteHandler(0x4);
	EMU->SetCPUWriteHandler(0x4, interceptAPUWrite);
	EMU->SetCPUWriteHandler(0x5, interceptAPUWrite);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, extraVal);
	SAVELOAD_BYTE(stateMode, offset, data, extraAddr);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =455;
} // namespace

MapperInfo MapperInfo_455 ={
	&mapperNum,
	_T("N625836"),
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