#include	"..\DLL\d_iNES.h"

namespace {
BOOL	MAPINT	Load (void) {
	return TRUE;
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	EMU->SetPRG_ROM32(0x8, 0);

	int Shift;
	if (~ROM->INES_Flags &8)
		Shift =(ROM->INES_Flags &1)? 0: 1;
	else
		Shift =(ROM->INES_Flags &1)? 3: 2;
		
	for (int i =0; i <16; i++) EMU->SetCHR_NT1(i, (i >>Shift) &1);
}

uint16_t MapperNum =218;
} // namespace

MapperInfo MapperInfo_218 ={
	&MapperNum,
	_T("Magic Floor"),
	COMPAT_FULL,
	Load,
	Reset,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};