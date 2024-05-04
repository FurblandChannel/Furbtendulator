#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

#define	target        (ROM->dipValue <<25 |0x20000000)
#define tqrom         ((reg[0] &0x06) ==0x02)
#define counterReset !(reg[0] &0x08)

namespace {
uint32_t	counter;
bool		counterExpired;
uint8_t		reg[2];

void	sync (void) {
	MMC3::syncWRAM();
	MMC3::syncPRG(reg[0] <<3 &0x18 | 0x07, reg[0] <<3 &0x20);
	if (tqrom) for (int bank =0; bank <8; bank++) {
		int val =MMC3::getCHRBank(bank);
		if (val &0x40)
			EMU->SetCHR_RAM1(bank &0x7F, val &7 | reg[0] <<5 &0x80);
		else
			EMU->SetCHR_ROM1(bank &0x7F, val    | reg[0] <<5 &0x80);
	}
	else
		MMC3::syncCHR(0x7F, reg[0] <<5 &0x80);
	MMC3::syncMirror();
}

int	MAPINT	read5 (int bank, int addr) {
	if (addr &0x800)
		return (counterExpired? 0x80: 0x00) |0x5C;
	else
		return ROM->PRGRAMData[0x2000 |addr];
}

void	MAPINT	write5 (int bank, int addr, int val) {
	if (addr &0x800) {
		reg[addr >>10 &1] =val;
		sync();
	} else
		ROM->PRGRAMData[0x2000 |addr] =val;
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	counter =0;
	for (auto& c: reg) c =0;
	MMC3::reset(RESET_HARD);
	EMU->SetCPUReadHandler     (0x5, read5);
	EMU->SetCPUReadHandlerDebug(0x5, read5);
	EMU->SetCPUWriteHandler    (0x5, write5);
}

void	MAPINT	cpuCycle (void) {
	MMC3::cpuCycle();
	if (counterReset) {
		counter =0;
		counterExpired =false;
	} else {
		if (++counter ==target) counterExpired =true;
		if ((counter %1789773) ==0) {
			uint32_t seconds =(target -counter) /1789773;
			EMU->StatusOut(_T("Time left: %02i:%02i"), seconds /60, seconds %60);
		}
	}
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_LONG(stateMode, offset, data, counter);
	for (auto& c: reg) SAVELOAD_BYTE(stateMode, offset, data, c);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =555;
} // namespace

MapperInfo MapperInfo_555 ={
	&mapperNum,
	_T("NES-EVENT2"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	cpuCycle,
	MMC3::ppuCycle,
	saveLoad,
	NULL,
	NULL
};