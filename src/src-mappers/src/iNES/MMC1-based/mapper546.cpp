#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC1.h"

namespace {
uint8_t		outerBank;
	
void	sync (void) {
	if (outerBank &0x10) {
		MMC1::syncPRG(0x0F, 0x10);
		MMC1::syncWRAM(0);
	} else {
		EMU->SetPRG_OB4(0x6);
		EMU->SetPRG_OB4(0x7);
		if (outerBank &0x20)
			EMU->SetPRG_ROM32(0x8, outerBank >>1);
		else {
			EMU->SetPRG_ROM16(0x8, outerBank);
			EMU->SetPRG_ROM16(0xC, outerBank);
		}
	}
	EMU->SetCHR_RAM8(0x0, 0);
	if (outerBank &0x80) for (int i =0; i <8; i++) EMU->SetCHR_Ptr1(i, EMU->GetCHR_Ptr1(i), FALSE);
	MMC1::syncMirror();
}

void	MAPINT	writeExtra (int bank, int addr, int val) {
	if (addr <0x100) {
		outerBank =addr &0xFF;
		sync();
	}
	MMC1::write(bank, addr, val);
}

BOOL	MAPINT	load (void) {
	MMC1::load(sync, MMC1Type::MMC1B);
	iNES_SetSRAM();
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	outerBank =0;
	MMC1::reset(RESET_HARD);
	EMU->SetCPUWriteHandler(0x8, writeExtra);
	EMU->SetCPUWriteHandler(0x9, writeExtra);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC1::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, outerBank);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =546;
} // namespace

MapperInfo MapperInfo_546 ={
	&mapperNum,
	_T("03-101"),
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
