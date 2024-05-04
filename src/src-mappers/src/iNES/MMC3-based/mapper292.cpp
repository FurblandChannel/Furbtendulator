#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
uint8_t		prg;
uint8_t		spriteXOR;
uint8_t		spriteOR;
uint8_t		bgCHR;
uint8_t		latch;
bool		loadBG;
FCPUWrite	passWrite;

void	sync (void) {
	MMC3::syncPRG(0xFF, 0x00);
	EMU->SetPRG_ROM8(0x8, prg);
	
	EMU->SetCHR_ROM2(0x0, (MMC3::getCHRBank(0) >>1) ^spriteXOR);
	EMU->SetCHR_ROM2(0x2, (MMC3::getCHRBank(2) >>1) ^spriteOR);
	EMU->SetCHR_ROM4(0x4, bgCHR);
	MMC3::syncMirror();
}

void	MAPINT	writeLatch (int bank, int addr, int val) {
	latch =val;
	passWrite(bank, addr, val);
}

int	MAPINT	applyLatch (int bank, int addr) {
	if (loadBG) {
		bgCHR =latch &0x3F;
		spriteOR =(latch &0x40) <<1;
	} else
		spriteXOR =latch;
	sync();
	return *EMU->OpenBus;
}

void	MAPINT	writePRG (int bank, int addr, int val) {
	prg =val &0x1F;
	loadBG =!!(val &0x20);
	sync();
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		prg =0;
		spriteXOR =0;
		spriteOR =0;
		bgCHR =0;
		latch =0;
		loadBG =false;		
	}
	MMC3::reset(resetType);
	MMC3::setWRAMCallback(applyLatch, writePRG);
	
	passWrite =EMU->GetCPUWriteHandler(0x0);
	EMU->SetCPUWriteHandler(0x0, writeLatch);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, prg);
	SAVELOAD_BYTE(stateMode, offset, data, spriteXOR);
	SAVELOAD_BYTE(stateMode, offset, data, spriteOR);
	SAVELOAD_BYTE(stateMode, offset, data, bgCHR);
	SAVELOAD_BYTE(stateMode, offset, data, latch);
	SAVELOAD_BOOL(stateMode, offset, data, loadBG);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t MapperNum = 292;
} // namespace

MapperInfo MapperInfo_292 = {
	&MapperNum,
	_T("BMW8544"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	MMC3::cpuCycle,
	MMC3::ppuCycle,
	saveLoad,
	NULL,
	NULL
};