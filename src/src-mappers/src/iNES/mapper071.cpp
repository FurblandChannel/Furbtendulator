#include	"..\DLL\d_iNES.h"

namespace {
uint8_t 	prg;
uint8_t 	mirroring;

void	sync (void) {
	EMU->SetPRG_ROM16(0x8, prg);
	EMU->SetPRG_ROM16(0xC, 0xFF);
	
	EMU->SetCHR_RAM8(0, 0);
	
	if (ROM->INES2_SubMapper !=1) 
		iNES_SetMirroring();
	else
	if (mirroring &0x10)
		EMU->Mirror_S1();
	else
		EMU->Mirror_S0();
}

void	MAPINT	writeMirroring (int bank, int addr, int val) {
	mirroring =val;
	sync();
}

void	MAPINT	writePRG (int bank, int addr, int val) {
	prg =val;
	sync();
}

BOOL	MAPINT	load (void) {
	if (ROM->INES_Version >=2) MapperInfo_071.Description =ROM->INES2_SubMapper ==1? L"BIC BF9097": L"BIC BF9093";
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType == RESET_HARD) {
		prg =0;
		mirroring =0;
	}
	sync();
	
	if (ROM->INES2_SubMapper ==1) EMU->SetCPUWriteHandler(0x9, writeMirroring);
	for (int bank =0xC; bank <=0xF; bank++) EMU->SetCPUWriteHandler(bank, writePRG);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, prg);
	if (ROM->INES2_SubMapper ==1) SAVELOAD_BYTE(stateMode, offset, data, mirroring);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =71;
} // namespace

MapperInfo MapperInfo_071 ={
	&mapperNum,
	_T("BIC BF9093/BF9097"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	NULL,
	NULL,
	saveLoad,
	NULL,
	NULL
};