#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC1.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
uint8_t		game;
	
void	sync (void) {
	int bank128k =game >2? 3: game;
	int bank32k  =game -3;
	switch(game) {
		case 0: case 1: case 2:
			MMC1::syncPRG(0x07, bank128k <<3);
			MMC1::syncCHR_ROM(0x1F, bank128k <<5);
			for (int bank =0x8; bank <=0xF; bank++) EMU->SetCPUWriteHandler(bank, MMC1::write);
			MMC1::syncMirror();
			break;
		case 3: case 4: case 5:
			EMU->SetPRG_ROM32(0x8, bank128k <<2 | bank32k);
			EMU->SetCHR_ROM8(0x0, bank128k <<4 | bank32k <<2 | Latch::data &3);
			for (int bank =0x8; bank <=0xF; bank++) EMU->SetCPUWriteHandler(bank, Latch::write);
			EMU->Mirror_V();
			break;
		case 6: 
			EMU->SetPRG_ROM32(0x8, bank128k <<2 | bank32k);
			MMC1::syncCHR_ROM(0x07, bank128k <<5 | bank32k <<3);
			for (int bank =0x8; bank <=0xF; bank++) EMU->SetCPUWriteHandler(bank, MMC1::write);
			MMC1::syncMirror();
			break;
	}	
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, false);
	MMC1::load(sync, MMC1Type::MMC1B);	
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD)
		game =0;
	else
		game++;
	if (game ==7) game =0;
	Latch::reset(RESET_HARD);
	MMC1::reset(RESET_HARD);
	sync();
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC1::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, game);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =448;
} // namespace

MapperInfo MapperInfo_448 ={
	&mapperNum,
	_T("3927"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	MMC1::cpuCycle,
	NULL,
	saveLoad,
	NULL,
	NULL
};
