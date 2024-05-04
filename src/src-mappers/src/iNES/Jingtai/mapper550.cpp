#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC1.h"

namespace {
uint8_t		outerBank;
uint8_t		latch;
FCPUWrite	writeWRAM;
	
void	sync (void) {
	if ((outerBank &6) ==6) {
		MMC1::syncPRG(0x07, outerBank <<2);
		MMC1::syncCHR(0x07, outerBank <<2 &0x18);
	} else {
		EMU->SetPRG_ROM32(0x8, latch >>4 | outerBank <<1);
		EMU->SetCHR_ROM8(0x0, latch &3 | outerBank <<1 &0xC);
	}
	MMC1::syncWRAM(0);
	MMC1::syncMirror();
}

void	MAPINT	writeOuterBank (int bank, int addr, int val) {
	if (~outerBank &8) {
		outerBank =addr &15;
		sync();
	}
	writeWRAM(bank, addr, val);
}

void	MAPINT	writeLatch (int bank, int addr, int val) {
	latch =val;
	if ((outerBank &6) ==6) MMC1::write(bank, addr, val);
	sync();
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	MMC1::load(sync, MMC1Type::MMC1A);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	latch =0;
	outerBank =0;
	MMC1::reset(RESET_HARD);
	writeWRAM =EMU->GetCPUWriteHandler(0x7);
	EMU->SetCPUWriteHandler(0x7, writeOuterBank);
	for (int bank =0x8; bank <=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeLatch);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC1::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, outerBank);
	SAVELOAD_BYTE(stateMode, offset, data, latch);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =550;
} // namespace

MapperInfo MapperInfo_550 ={
	&mapperNum,
	_T("晶太 JY820845C"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	MMC1::cpuCycle,
	NULL,
	saveLoad,
	NULL,
	NULL
};
