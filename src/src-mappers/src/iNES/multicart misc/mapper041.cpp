#include	"..\..\DLL\d_iNES.h"

namespace {
uint8_t		outerBank;
uint8_t		innerBank;

#define enableInnerBank	    outerBank &0x04
#define	horizontalMirroring outerBank &0x20

void	sync (void) {
	EMU->SetPRG_ROM32(0x8, outerBank     &0x07);
	EMU->SetCHR_ROM8 (0x0, outerBank >>1 &0x0C | innerBank &0x03);
	if (horizontalMirroring)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

void	MAPINT	writeOuterBank (int bank, int addr, int val) {
	if (~addr &0x800) {
		outerBank =addr;
		sync();
	}
}

void	MAPINT	writeInnerBank (int bank, int addr, int val) {
	if (enableInnerBank) {
		innerBank =val &EMU->GetCPUReadHandler(bank)(bank, addr);
		sync();
	}
}

void	MAPINT	reset (RESET_TYPE resetType) {
	outerBank =0;
	innerBank =0;
	sync();

	EMU->SetCPUWriteHandler(0x6, writeOuterBank);
	for (int bank =0x8; bank<=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeInnerBank);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, outerBank);
	SAVELOAD_BYTE(stateMode, offset, data, innerBank);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =41;
} // namespace

MapperInfo MapperInfo_041 ={
	&mapperNum,
	_T("NTDEC 2399"),
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