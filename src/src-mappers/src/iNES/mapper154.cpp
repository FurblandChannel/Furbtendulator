#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_N118.h"

namespace {
uint8_t		mirroring;
	
void	sync (void) {
	N118::syncPRG(0x0F, 0x00);
	EMU->SetCHR_ROM2(0, N118::reg[0] >>1 &0x1F);
	EMU->SetCHR_ROM2(2, N118::reg[1] >>1 &0x1F);
	EMU->SetCHR_ROM1(4, N118::reg[2] |0x40);
	EMU->SetCHR_ROM1(5, N118::reg[3] |0x40);
	EMU->SetCHR_ROM1(6, N118::reg[4] |0x40);
	EMU->SetCHR_ROM1(7, N118::reg[5] |0x40);
	if (mirroring &0x40)
		EMU->Mirror_S1();
	else	
		EMU->Mirror_S0();
}

void	MAPINT	writeMirroring (int bank, int addr, int val) {
	mirroring =val;
	if (bank <=0x9)
		N118::writeReg(bank, addr, val);
	else
		sync();
}

BOOL	MAPINT	Load (void) {
	N118::load(sync);
	return TRUE;
}

void	MAPINT	Reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) mirroring =0;
	N118::reset(resetType);
	for (int bank =0x8; bank<=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeMirroring);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =N118::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, mirroring);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t MapperNum =154;
} // namespace

MapperInfo MapperInfo_154 ={
	&MapperNum,
	_T("Namco 3453"),
	COMPAT_FULL,
	Load,
	Reset,
	NULL,
	NULL,
	NULL,
	saveLoad,
	NULL,
	NULL
};