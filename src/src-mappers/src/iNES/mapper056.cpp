#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_KS202.h"

namespace {
uint8_t		prg[4];
uint8_t		chr[8];
uint8_t		mirroring;

void	sync (void) {
	KS202::syncPRG(0x0F, 0x00, prg[0], prg[1], prg[2], 0x10);
	for (int bank =0; bank <8; bank++) EMU->SetCHR_ROM1(bank, chr[bank]);
	if (mirroring &1)
		EMU->Mirror_V();
	else
		EMU->Mirror_H();
}

void	MAPINT	writePAL (int bank, int addr, int val) {
	KS202::writePRGdata(bank, addr, val);
	switch (addr >>10) {
		case 0: prg[addr &3] =val &0x10;
			break;
		case 2: mirroring =val;
			break;
		case 3:	chr[addr &7] =val;
			break;
	}
	sync();
}

BOOL	MAPINT	load (void) {
	KS202::load(sync);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	for (auto& c: prg) c =0x10;
	for (auto& c: chr) c =0x00;
	mirroring =0;
	KS202::reset(resetType);
	EMU->SetCPUWriteHandler(0xF, writePAL);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =KS202::saveLoad(stateMode, offset, data);
	for (auto& c: prg) SAVELOAD_BYTE(stateMode, offset, data, c);
	for (auto& c: chr) SAVELOAD_BYTE(stateMode, offset, data, c);
	SAVELOAD_BYTE(stateMode, offset, data, mirroring);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =56;
} // namespace

MapperInfo MapperInfo_056 ={
	&mapperNum,
	_T("Kaiser SMB3"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	KS202::cpuCycle,
	NULL,
	saveLoad,
	NULL,
	NULL
};