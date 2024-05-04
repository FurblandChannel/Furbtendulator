#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_Latch.h"

namespace {
FPPURead _ReadCHR;
FCPURead _Read2;
int PPUReadCount;

void	Sync (void) {
	EMU->SetPRG_ROM32(0x8, 0);
	EMU->SetCHR_ROM8(0, 0);
}

BOOL	MAPINT	Load (void) {
	Latch::load(Sync, FALSE);
	return TRUE;
}

int	MAPINT Read2(int Bank, int Addr) {
	if ((Addr &7) ==7 && PPUReadCount <2) PPUReadCount++;
	return _Read2(Bank, Addr);
}

int	MAPINT ReadCHR(int Bank, int Addr) {
	if ((ROM->INES2_SubMapper &0xC) ==4)
		return ((Latch::data &3) ==(ROM->INES2_SubMapper &3))? _ReadCHR(Bank, Addr): 0xFF;
	else
		return (PPUReadCount >=2) ?_ReadCHR(Bank, Addr): 0xFF;
}


void	MAPINT	Reset (RESET_TYPE ResetType) {
	iNES_SetMirroring();
	Latch::reset(ResetType);

	if ((ROM->INES2_SubMapper &0xC) !=4) { // If no valid submapper has been specified, use heuristic
		_Read2 =EMU->GetCPUReadHandler(2);
		EMU->SetCPUReadHandler(2, Read2);
		PPUReadCount =0;
	}

	_ReadCHR =EMU->GetPPUReadHandler(0);
	for (int i =0; i <8; i++) {
		EMU->SetPPUReadHandler(i, ReadCHR);
		EMU->SetPPUReadHandlerDebug(i, ReadCHR);
	}
}

uint16_t MapperNum =185;
} // namespace

MapperInfo MapperInfo_185 = {
	&MapperNum,
	_T("Nintendo CNROM+Security"),
	COMPAT_FULL,
	Load,
	Reset,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_D,
	NULL,
	NULL
};
