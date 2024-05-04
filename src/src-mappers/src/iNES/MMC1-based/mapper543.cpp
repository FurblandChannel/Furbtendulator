#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC1.h"

namespace {
uint8_t		outerBank;
uint8_t		shift;
uint8_t		bits;
	
void	sync (void) {
	/* If MMC1 CHR bit 3 is set, use the battery-backed 8 KiB, otherwise the non-backed 8 KiB
	   The emulator interface requires the battery-backed half first.
	   
	   On CH-501, the arrangement therefore is:
	   
	   Sangokushi		saving 0x08/0x08, normal 0x08/0x00:   chip 0, bank 0/1
	   Genghis Khan		saving 0x09/0x08, normal 0x09/0x00:   chip 0, bank 2/3
	   Final Fantasy II	always 0x0A/0x00: chip 1, bank 0
	   Hanjuku Hero		always 0x0E/0x00: chip 1, bank 2
	   Zelda no Densetsu	always 0x0F/0x00: chip 1, bank 3
	   
	   Outer bank
	   D~3210
	     ----
	     1PpP
	     |||+- PRG A18/SRAM #1 A14, SRAM #2 A13
	     ||+-- PRG A19/SRAM #1/#2 select
	     |+--- PRG A20/SRAM #2 A14
	     +---- 1 during game-play, 0 during menu
	   
	   MMC1 CHR A15 is connected to SRAM #1 A13. */
	int wramBank;
	if (outerBank &2)
		wramBank =outerBank &1 | (outerBank &4? 2: 0) | 4;
	else
		wramBank =(MMC1::getCHRBank(0) &8? 1: 0) | (outerBank &1? 2: 0);
	MMC1::syncWRAM(wramBank);
	MMC1::syncPRG(0x0F, outerBank <<4);
	MMC1::syncCHR_RAM(0x07, 0x00);
	MMC1::syncMirror();
}

void	MAPINT	writeExtra (int bank, int addr, int val) {
	shift |=(val &8? 1: 0) <<bits++;
	if (bits ==4) {
		outerBank =shift;
		shift =0;
		bits =0;
		sync();
	}
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	MMC1::load(sync, MMC1Type::MMC1B);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	outerBank =0;
	shift =0;
	bits =0;
	MMC1::reset(RESET_HARD);
	EMU->SetCPUWriteHandler(0x5, writeExtra);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC1::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, outerBank);
	SAVELOAD_BYTE(stateMode, offset, data, shift);
	SAVELOAD_BYTE(stateMode, offset, data, bits);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =543;
} // namespace

MapperInfo MapperInfo_543 ={
	&mapperNum,
	_T("5-in-1"),
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
