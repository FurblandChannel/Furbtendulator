#include	"..\..\DLL\d_iNES.h"

namespace {
uint8_t	outerBank, innerBank;

void	sync (void) {	
	EMU->SetPRG_ROM16(0x8, outerBank | innerBank);
	EMU->SetPRG_ROM16(0xC, outerBank | 0x3);
	EMU->SetCHR_RAM8(0, 0);
}

void	MAPINT	writeOuterBank (int bank, int addr, int val) {
	outerBank = (val >>1) &0xC;
	if (ROM->INES2_SubMapper ==1) outerBank = ((outerBank &0x8) >>1) | ((outerBank &0x4) <<1); // Submapper #1: Swapped outer bank bits 
	sync();
}
void	MAPINT	writeInnerBank (int bank, int addr, int val) {
	innerBank = val &0x3;
	sync();
}

void	MAPINT	reset (RESET_TYPE resetType) {
	outerBank =innerBank =0;
	for (int i =0x8; i <=0xB; i++) EMU->SetCPUWriteHandler(i, writeOuterBank);
	for (int i =0xC; i <=0xF; i++) EMU->SetCPUWriteHandler(i, writeInnerBank);
	iNES_SetMirroring();
	sync();
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, outerBank);
	SAVELOAD_BYTE(stateMode, offset, data, innerBank);
	if (stateMode == STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =232;
} // namespace

MapperInfo MapperInfo_232 ={
	&mapperNum,
	_T("BIC BF9096"),
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