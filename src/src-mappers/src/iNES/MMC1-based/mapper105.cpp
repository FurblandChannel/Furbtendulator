#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC1.h"

#define	target       (ROM->dipValue <<25 |0x20000000)
#define mmc1Mode      MMC1::reg[1] &0x08
#define nromBank      MMC1::reg[1] >>1 &3
#define counterReset (MMC1::reg[1] &0x10)

namespace {
uint32_t	counter;

void	sync (void) {
	MMC1::syncWRAM(0x0);
	if (mmc1Mode)
		MMC1::syncPRG(0x7, 0x8);
	else
		EMU->SetPRG_ROM32(0x8, nromBank);
	
	MMC1::syncCHR(0x01, 0x00); // Always 8 KiB of CHR RAM
	MMC1::syncMirror();
}

BOOL	MAPINT	load (void) {
	MMC1::load(sync, MMC1Type::MMC1B);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	counter =0;
	MMC1::reset(RESET_HARD);
}

void	MAPINT	cpuCycle (void) {
	MMC1::cpuCycle();
	if (counterReset) {
		EMU->SetIRQ(1);
		counter =0;
	} else {
		if (++counter ==target) EMU->SetIRQ(0);
		if ((counter %1789773) ==0) {
			uint32_t seconds =(target -counter) /1789773;
			EMU->StatusOut(_T("Time left: %02i:%02i"), seconds /60, seconds %60);
		}
	}
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC1::saveLoad(stateMode, offset, data);
	SAVELOAD_LONG(stateMode, offset, data, counter);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =105;
} // namespace

MapperInfo MapperInfo_105 ={
	&mapperNum,
	_T("NES-EVENT"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	cpuCycle,
	NULL,
	saveLoad,
	NULL,
	NULL
};