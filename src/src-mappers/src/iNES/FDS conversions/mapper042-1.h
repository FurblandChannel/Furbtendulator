#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\Sound\s_FDS.h"

namespace Submapper1 {
uint8_t		prg;
uint8_t		chr;

void	sync (void) {
	EMU->SetPRG_ROM32(0x8, 0xFF);
	EMU->SetPRG_ROM8(0x6, prg);
	EMU->SetCHR_ROM8(0x0, chr);
	iNES_SetMirroring();
}

void	MAPINT	writeCHR (int bank, int addr, int val) {
	chr =val;
	sync();
}

void	MAPINT	writePRG (int bank, int addr, int val) {
	prg =val;
	sync();
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {		
		prg =0xFF;
		chr =0;
	}
	sync();

	EMU->SetCPUWriteHandler(0x8, writeCHR);
	for (int i =0xE; i<=0xF; i++) EMU->SetCPUWriteHandler(i, writePRG);
	FDSsound::ResetBootleg(resetType);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, prg);
	SAVELOAD_BYTE(stateMode, offset, data, chr);
	offset =FDSsound::SaveLoad(stateMode, offset, data);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =42;
MapperInfo MapperInfo_042 ={
	&mapperNum,
	_T("Kaiser KS-7050"),
	COMPAT_FULL,
	NULL,
	reset,
	::unload,
	NULL,
	NULL,
	saveLoad,
	FDSsound::GetBootleg,
	NULL,
};
} // namespace

