#include	"..\..\DLL\d_iNES.h"

namespace {
uint8_t		prgHi;
uint8_t		chrHi;
uint8_t		innerBank;

void	sync (void) {
	if (chrHi &2) {
		EMU->SetPRG_ROM16(0x8, prgHi >>2 | innerBank >>2 &3);
		EMU->SetPRG_ROM16(0xC, prgHi >>2 |                3);
	} else
		EMU->SetPRG_ROM32(0x8, prgHi >>3);
	EMU->SetCHR_ROM8(0x0, chrHi >>1 &0xFC | innerBank &0x03);
	if (prgHi &1)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

void	MAPINT	writePRGHi (int bank, int addr, int val) {
	prgHi =addr &0xFF;
	sync();
}

void	MAPINT	writeCHRHi (int bank, int addr, int val) {
	chrHi =addr &0xFF;
	sync();
}

void	MAPINT	writeinnerBank (int bank, int addr, int val) {
	innerBank =addr &0xFF;
	sync();
}

void	MAPINT	reset (RESET_TYPE resetType) {
	prgHi =0;
	chrHi =0;
	innerBank =0;
	sync();

	EMU->SetCPUWriteHandler(0x8, writePRGHi);
	EMU->SetCPUWriteHandler(0x9, writeCHRHi);
	for (int bank =0xA; bank<=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeinnerBank);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, prgHi);
	SAVELOAD_BYTE(stateMode, offset, data, chrHi);
	SAVELOAD_BYTE(stateMode, offset, data, innerBank);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =389;
} // namespace

MapperInfo MapperInfo_389 ={
	&mapperNum,
	_T("Caltron 9-in-1"),
	COMPAT_FULL,
	NULL,
	reset,
	NULL,
	NULL,
	NULL,
	saveLoad,
	NULL,
	NULL
};