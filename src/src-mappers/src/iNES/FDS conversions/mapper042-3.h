#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\Sound\s_FDS.h"

#define	prg                 reg[0]
#define horizontalMirroring reg[1] &8
#define irq                 reg[2] &2

namespace Submapper3 {
uint8_t		reg[4];
uint16_t	counter;

void	sync (void) {
	EMU->SetPRG_ROM8(0x6, prg);
	EMU->SetPRG_ROM32(0x8, 0xFF);
	EMU->SetCHR_RAM8(0x0, 0);
	if (horizontalMirroring)
		EMU->Mirror_H();
	else	
		EMU->Mirror_V();
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	reg[addr &3] =val;
	sync();
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		for (auto& c: reg) c =0;
		counter =0;
	}
	sync();
	
	FDSsound::ResetBootleg(resetType);
	for (int bank =0xE; bank<=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeReg);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =FDSsound::SaveLoad(stateMode, offset, data);
	for (auto& c: reg) SAVELOAD_BYTE(stateMode, offset, data, c);
	SAVELOAD_WORD(stateMode, offset, data, counter);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

void	MAPINT	cpuCycle (void) {
	if (irq)
		EMU->SetIRQ((++counter &0x6000) ==0x6000? 0: 1);
	else {
		EMU->SetIRQ(1);
		counter =0;
	}
}

uint16_t mapperNum =42;
MapperInfo MapperInfo_042 ={
	&mapperNum,
	_T("Mario Baby"),
	COMPAT_FULL,
	NULL,
	reset,
	::unload,
	cpuCycle,
	NULL,
	saveLoad,
	FDSsound::GetBootleg,
	NULL,
};

#undef	prg
#undef	horizontalMirroring
#undef	irq

} // namespace

