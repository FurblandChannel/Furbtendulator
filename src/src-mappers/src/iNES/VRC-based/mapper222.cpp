#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_VRC24.h"

/*
	222.1: Dragon Ninja: CTC-31: Normal VRC2, with strangely-wired 76'161 creating an IRQ counter
	222.2: Super Bros. 8: 810343-C/920318-M. Normal VRC, plus latch that nibblizes an 8-bit CHR bank number.
*/
namespace {
bool		clockMode;
bool		pending;
uint8_t		counter1, counter2;
uint8_t		prescaler;

void	sync (void) {
	VRC24::syncPRG(0x1F, 0x00);
	VRC24::syncCHR_ROM(0xFFF, 0x000);
	VRC24::syncMirror();
}

void	MAPINT	trapCHRWrite (int bank, int addr, int val) {
	if (~addr &1) {
		VRC24::writeCHR(bank, addr,    val);
		VRC24::writeCHR(bank, addr |1, val >>4);
	}
}

void	MAPINT	writeIRQ (int bank, int addr, int val) {
	switch (addr &3) {
		case 0: clockMode =false;
		        break;
		case 1: if (!clockMode) {
				counter1 =val &0xF;
				counter2 =val >>4;
			}
		        pending =false;
			break;
		case 2: clockMode =true;
			break;
	}
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	VRC24::load(sync, false, 0x01, 0x02, NULL, true, 0);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	clockMode =false;
	counter1 =0;
	counter2 =0;
	prescaler =0;
	VRC24::reset(resetType);
	for (int bank =0xB; bank<=0xE; bank++) EMU->SetCPUWriteHandler(bank, trapCHRWrite);
	EMU->SetCPUWriteHandler(0xF, writeIRQ);
}

void	MAPINT	cpuCycle (void) {
	auto previousPrescaler =prescaler;
	if (pending)
		prescaler =0;
	else
		prescaler++;
	
	if (clockMode && ~previousPrescaler &0x40 && prescaler &0x40) {
		if (++counter1 ==0xF && ++counter2 ==0xF) pending =true;
		counter1 &=0xF;
		counter2 &=0xF;
	}
	EMU->SetIRQ(pending? 0: 1);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =VRC24::saveLoad(stateMode, offset, data);
	SAVELOAD_BOOL(stateMode, offset, data, clockMode);
	SAVELOAD_BYTE(stateMode, offset, data, counter1);
	SAVELOAD_BYTE(stateMode, offset, data, counter2);
	SAVELOAD_BYTE(stateMode, offset, data, prescaler);
	return offset;
}

uint16_t mapperNum =222;
} // namespace

MapperInfo MapperInfo_222 = {
	&mapperNum,
	_T("810343-C"),
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