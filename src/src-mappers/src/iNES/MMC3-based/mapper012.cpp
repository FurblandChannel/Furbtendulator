#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

extern	MapperInfo MapperInfo_012_1;	// in mapper006.cpp
extern	MapperInfo MapperInfo_SL5020B;

namespace {
uint8_t		chrA18;
FCPURead	readAPU;
FCPUWrite	writeAPU;

void	sync (void) {
	MMC3::syncPRG(0x3F, 0x00);
	MMC3::syncCHR_ROM(0xFF, chrA18 <<8 &0x100, chrA18 <<4 &0x100);
	MMC3::syncMirror();
}

int	MAPINT	readDIP (int bank, int addr) {
	return addr &0x100? ROM->dipValue: readAPU(bank, addr);
}

void	MAPINT	writeCHRA18 (int bank, int addr, int val) {
	writeAPU(bank, addr, val);
	if (addr &0x100) {
		chrA18 =val;
		sync();
	}
}

BOOL	MAPINT	load (void) {
	if (ROM->INES2_SubMapper ==1)
		memcpy(&MapperInfo_012, &MapperInfo_012_1, sizeof(MapperInfo));
	else
		memcpy(&MapperInfo_012, &MapperInfo_SL5020B, sizeof(MapperInfo));
	return MapperInfo_012.Load();
}

BOOL	MAPINT	loadSL5020B (void) {
	MMC3::load(sync, MMC3Type::NEC);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) chrA18 =0;
	MMC3::reset(resetType);
	readAPU  =EMU->GetCPUReadHandler(0x4);
	writeAPU =EMU->GetCPUWriteHandler(0x4);
	EMU->SetCPUReadHandler(0x4, readDIP);
	EMU->SetCPUWriteHandler(0x4, writeCHRA18);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, chrA18);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =12;
} // namespace

MapperInfo MapperInfo_012 ={
	&mapperNum,
	_T("哥德 SL-5020B/Front Fareast Magic Card 4M"),
	COMPAT_FULL,
	load,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};

MapperInfo MapperInfo_SL5020B ={
	&mapperNum,
	_T("哥德 SL-5020B"),
	COMPAT_FULL,
	loadSL5020B,
	reset,
	NULL,
	MMC3::cpuCycle,
	MMC3::ppuCycle,
	saveLoad,
	NULL,
	NULL
};
