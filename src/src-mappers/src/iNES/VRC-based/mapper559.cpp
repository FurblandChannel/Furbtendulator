#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_VRC24.h"

namespace {
uint8_t		nt[4];
uint8_t		cpuC;

void	sync (void) {
	VRC24::syncPRG(0x01F, 0x00);
	VRC24::syncCHR(0x1FF, 0x00);
	EMU->SetPRG_ROM8(0xC, cpuC);
	
	for (int bank =0x8; bank<=0xF; bank++) EMU->SetCHR_NT1(bank, nt[bank &3]);
}

void	MAPINT	writeExtra (int bank, int addr, int val) {
	if (addr &4)
		nt[addr &3] =val;
	else
		cpuC =val;
	sync();
}

void	MAPINT	writeNibblize (int bank, int addr, int val) {
	if (addr &0x400) val >>=4;
	if (bank ==0xF)
		VRC24::writeIRQ(bank, addr, val);
	else
		VRC24::writeCHR(bank, addr, val);
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	VRC24::load(sync, true, 0x400, 0x800, writeExtra, true, 1);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		nt[0] =0xE0;
		nt[1] =0xE0;
		nt[2] =0xE1;
		nt[3] =0xE1;
		cpuC =0xFE;
	}
	VRC24::reset(resetType);
	EMU->SetCPUWriteHandler(0xB, writeNibblize);
	EMU->SetCPUWriteHandler(0xC, writeNibblize);
	EMU->SetCPUWriteHandler(0xD, writeNibblize);
	EMU->SetCPUWriteHandler(0xE, writeNibblize);
	EMU->SetCPUWriteHandler(0xF, writeNibblize);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =VRC24::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, cpuC);
	for (auto& c: nt) SAVELOAD_BYTE(stateMode, offset, data, c);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =559;
} // namespace

MapperInfo MapperInfo_559 ={
	&mapperNum,
	_T("Subor Sango II"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	VRC24::cpuCycle,
	NULL,
	saveLoad,
	NULL,
	NULL
};