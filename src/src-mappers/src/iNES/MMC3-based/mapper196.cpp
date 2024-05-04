#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
uint8_t		prg;
bool		nrom;
FCPUWrite	writeAPU;

void	sync (void) {
	if (nrom)
		EMU->SetPRG_ROM32(0x8, prg);
	else
		MMC3::syncPRG(0x3F, 0);
	MMC3::syncCHR_ROM(0xFF, 0);
	MMC3::syncMirror();
}

int	MAPINT	readDIP (int bank, int addr) {
	return ROM->dipValue;
}

void	MAPINT	writeNROM (int bank, int addr, int val) {
	nrom =true;
	prg =val | (val >>4);
	sync();
}

void	MAPINT	writeMMC3 (int bank, int addr, int val) {
	addr =!!(addr &0xE) ^(addr &1);
	MMC3::write(bank, addr, val);
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		prg =0;
		nrom =false;
	}
	MMC3::reset(resetType);
	EMU->SetCPUReadHandler(0x5, readDIP);
	for (int i =0x6; i<=0x7; i++) EMU->SetCPUWriteHandler(i, writeNROM);
	for (int i =0x8; i<=0xF; i++) EMU->SetCPUWriteHandler(i, writeMMC3);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, prg);
	SAVELOAD_BOOL(stateMode, offset, data, nrom); 
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =196;
} // namespace

MapperInfo MapperInfo_196 ={
	&mapperNum,
	_T("MRCM UT1374"),
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