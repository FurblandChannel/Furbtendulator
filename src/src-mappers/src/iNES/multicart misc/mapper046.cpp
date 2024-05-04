#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

#define	prg (outerBank &0x0F)
#define chr (outerBank &0xF0)
namespace {
uint8_t		outerBank;

void	sync (void) {
	EMU->SetPRG_ROM32(0x8, prg <<1     | Latch::data     &1);
	EMU->SetCHR_ROM8 (0x0, chr >>1 &~7 | Latch::data >>4 &7);
	iNES_SetMirroring();
}

void	MAPINT	writeOuterBank (int bank, int addr, int val) {
	outerBank =val;
	sync();
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, true);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	outerBank =0;
	Latch::reset(RESET_HARD);	
	for (int bank =0x6; bank<=0x7; bank++) EMU->SetCPUWriteHandler(bank, writeOuterBank);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =Latch::saveLoad_D(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, outerBank);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =46;
} // namespace

MapperInfo MapperInfo_046 ={
	&mapperNum,
	_T("GameStation/RumbleStation"),
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
