#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
bool		nrom;
bool		nrom256;
uint8_t		prg;

void	sync (void) {
	if (nrom) {
		EMU->SetPRG_ROM16(0x8, nrom256? prg &~1: prg);
		EMU->SetPRG_ROM16(0xC, nrom256? prg | 1: prg);
	} else
		MMC3::syncPRG(0x3F, 0x00);
	MMC3::syncCHR_ROM(0xFF, (~MMC3::pointer &0x80) <<1, (MMC3::pointer &0x80) <<1);
	MMC3::syncMirror();
}

int	MAPINT	readProtection (int bank, int addr) {
	return *EMU->OpenBus |0x80;
}

void	MAPINT	writeNROM (int bank, int addr, int val) {	
	if (~addr &1) {
		prg =(val &0x1E) >>1;
		nrom =!!(val &0x80);
		nrom256 =!!(val &0x20);
		sync();
	}
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		nrom =false;
		nrom256 =false;
		prg =0;
	}
	MMC3::reset(resetType);
	EMU->SetCPUReadHandler(0x5, readProtection);
	EMU->SetCPUWriteHandler(0x5, writeNROM);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BOOL(stateMode, offset, data, nrom);
	SAVELOAD_BOOL(stateMode, offset, data, nrom256);
	SAVELOAD_BYTE(stateMode, offset, data, prg);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =187;
} // namespace

MapperInfo MapperInfo_187 ={
	&mapperNum,
	_T("卡聖 A98402"),
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