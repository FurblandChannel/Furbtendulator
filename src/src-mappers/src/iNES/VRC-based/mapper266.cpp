#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_VRC24.h"
#include	"..\..\Hardware\Sound\Butterworth.h"

namespace {
uint8_t		prg;
uint8_t		pcm;
Butterworth	filter(20, 1789772.727272, 2000.0); // Game uses a sampling rate of about 4 kHz

void	sync (void) {
	EMU->SetPRG_ROM32(0x8, prg);
	VRC24::syncCHR(0x1FF, 0x00);
	VRC24::syncMirror();
}

void	MAPINT	writePRG (int bank, int addr, int val) {
	if (addr &0x800)
		pcm =val &0xF;
	else {
		prg =val >>2;
		sync();
	}
}

void	MAPINT	write (int bank, int addr, int val) {
	VRC24::write (bank &~6 | bank <<1 &4 | bank >>1 &2, addr, val);
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	VRC24::load(sync, true, 0x04, 0x08, writePRG, true, 1);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	prg =0;
	pcm =7;
	VRC24::reset(resetType);
	for (int bank =0x8; bank<=0xF; bank++) EMU->SetCPUWriteHandler(bank, write);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =VRC24::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, prg);
	SAVELOAD_BYTE(stateMode, offset, data, pcm);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

int	MAPINT	mapperSound (int cycles) {
	return filter.output((pcm -7)*4096 +1e-15);
}

uint16_t mapperNum =266;
} // namespace

MapperInfo MapperInfo_266 = {
	&mapperNum,
	_T("City Fighter IV"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	VRC24::cpuCycle,
	NULL,
	saveLoad,
	mapperSound,
	NULL
};