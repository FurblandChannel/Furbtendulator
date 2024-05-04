#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"
#include	"..\..\Hardware\h_JY.h"

namespace {
uint8_t		reg[4];
bool		gotHandlers =false;
FCPUWrite	writeMMC3[8];
FCPUWrite	writeJY[8];

void	sync (void) {
	int prgAND =reg[3] &0x10? 0x1F: 0x0F;
	int chrAND =reg[3] &0x80? 0xFF: 0x7F;
	int prgOR  =reg[3] <<1 &0x10 | reg[1] <<5 &0x20;
	int chrOR  =reg[3] <<1 &0x080 | reg[1] <<8 &0x100;
	if (reg[1] &0x10) {
		if (gotHandlers) for (int bank =0x8; bank<=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeJY[bank &7]);
		JY::syncPRG(0x1F, prgOR);
		JY::syncCHR(0xFF, chrOR);
		JY::syncNT (0xFF, chrOR);
	} else {
		if (gotHandlers) for (int bank =0x8; bank<=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeMMC3[bank &7]);
		if (reg[1] &0x08)
			MMC3::syncPRG(prgAND, prgOR);
		else {
			prgOR |= reg[3] <<1 &0x0F;
			EMU->SetPRG_ROM32(0x8, prgOR >>2);
		}
		MMC3::syncCHR(chrAND, chrOR);
		MMC3::syncWRAM();
		MMC3::syncMirror();
	}
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync);
	JY::load(sync, true);
	return TRUE;
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	reg[addr &3] =val;
	sync();
}

void	MAPINT	reset (RESET_TYPE resetType) {	
	if (resetType ==RESET_HARD) {
		reg[1] =0x0F;
		reg[3] =0x10;
	}
	gotHandlers =false;
	MMC3::reset(resetType);
	for (int bank =0x8; bank<=0xF; bank++) writeMMC3[bank &7] =EMU->GetCPUWriteHandler(bank);
	JY::reset(resetType);
	for (int bank =0x8; bank<=0xF; bank++) writeJY  [bank &7] =EMU->GetCPUWriteHandler(bank);
	
	EMU->SetCPUWriteHandler(0x5, writeReg);
	gotHandlers =true;
	sync();
}

void	MAPINT	cpuCycle () {
	if (reg[1] &0x10)
		JY::cpuCycle();
	else
		MMC3::cpuCycle();
}

void	MAPINT	ppuCycle (int addr, int scanline, int cycle, int isRendering) {
	if (reg[1] &0x10)
		JY::ppuCycle(addr, scanline, cycle, isRendering);
	else
		MMC3::ppuCycle(addr, scanline, cycle, isRendering);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	offset =JY::saveLoad(stateMode, offset, data);
	for (auto& r: reg) SAVELOAD_BYTE(stateMode, offset, data, r);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =394;
} // namespace

MapperInfo MapperInfo_394 ={
	&mapperNum,
	_T("HSK007"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	cpuCycle,
	ppuCycle,
	saveLoad,
	NULL,
	NULL
};