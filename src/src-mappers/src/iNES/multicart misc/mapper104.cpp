#include	"..\..\DLL\d_iNES.h"

#define	locked (outer &8)

namespace {
uint8_t		inner;
uint8_t		outer;
int		cycles;

void	sync (void) {
	EMU->SetPRG_ROM16(0x8, outer <<4 |inner &0xF);
	EMU->SetPRG_ROM16(0xC, outer <<4 |0xF);
	EMU->SetCHR_RAM8(0x0, 0);
	iNES_SetMirroring();
}

void	MAPINT	writeOuter (int bank, int addr, int val) {
	if (!locked && cycles >=120'000) {
		outer =val;
		sync();
	}
}

void	MAPINT	writeInner (int bank, int addr, int val) {
	inner =val;
	sync();
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		outer =0;
		inner =0;
		cycles =0;
	}
	sync();
	for (int bank =0x8; bank <=0xB; bank++) EMU->SetCPUWriteHandler(bank, writeOuter);
	for (int bank =0xC; bank <=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeInner);
}

void	MAPINT	cpuCycle (void) {
	if (cycles <120'000) cycles++;
}

int	MAPINT	saveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(mode, offset, data, outer);
	SAVELOAD_BYTE(mode, offset, data, inner);
	SAVELOAD_LONG(mode, offset, data, cycles);
	if (mode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =104;
} // namespace

MapperInfo MapperInfo_104 = {
	&mapperNum,
	_T("Pegasus 5-in-1"),
	COMPAT_FULL,
	NULL,
	reset,
	NULL,
	cpuCycle,
	NULL,
	saveLoad,
	NULL,
	NULL
};