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
	MMC3::syncCHR_ROM(0xFF, 0x00);	
	MMC3::syncMirror();
}

void	MAPINT	writeNROM (int bank, int addr, int val) {	
	if (addr &0x800) {
		prg =(val &0x05) | ((val &0x28) >>2);
		nrom =!!(val &0x40);
		nrom256 =!!(val &0x02);
	}
	sync();
}

void	MAPINT	writeReg (int bank, int addr, int val) {	
	static const uint8_t lut[8] ={ 0, 3, 1, 5, 6, 7, 2, 4 };
	if (~addr &1) val =val &0xC0 | lut[val &0x7];
	MMC3::writeReg(bank, addr, val);
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
	EMU->SetCPUWriteHandler(0x5, writeNROM);
	for (int bank =0x8; bank<=0x9; bank++) EMU->SetCPUWriteHandler(bank, writeReg);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BOOL(stateMode, offset, data, nrom);
	SAVELOAD_BOOL(stateMode, offset, data, nrom256);
	SAVELOAD_BYTE(stateMode, offset, data, prg);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =123;
} // namespace

MapperInfo MapperInfo_123 ={
	&mapperNum,
	_T("卡聖 H2288"),
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