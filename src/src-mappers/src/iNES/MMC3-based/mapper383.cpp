#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
uint8_t		A15;
uint8_t		A16;
uint8_t		A17A18;
FCPURead	readCart;

void	sync (void) {
	if (A17A18 ==0x00)
		MMC3::syncPRG(A16? 0x07: 0x03, (A16? 0x00: A15) |A16 |A17A18);
	else
	if (A17A18 ==0x30) {
		EMU->SetPRG_ROM8(0x6, MMC3::getPRGBank(3) &0xB |A17A18);
		EMU->SetPRG_ROM8(0x8, MMC3::getPRGBank(2) &0xF |A17A18);
		EMU->SetPRG_ROM8(0xA, MMC3::getPRGBank(3) &0xF |A17A18);
		EMU->SetPRG_ROM8(0xC, MMC3::getPRGBank(0) &0xF |A17A18);
		EMU->SetPRG_ROM8(0xE, MMC3::getPRGBank(1) &0xF |A17A18);
	} else
		MMC3::syncPRG(0x0F, A17A18);
	
	MMC3::syncCHR(0x7F, A17A18 <<3);
	MMC3::syncMirror();
}

int	MAPINT	readLatch (int bank, int addr) {
	if (A17A18 ==0x00) {
		A16 =MMC3::getPRGBank(bank &2? 1: 0) &0x08;
		sync();
	}
	return readCart(bank, addr);
}

void	MAPINT	writePAL (int bank, int addr, int val) {
	if (addr &0x100) {
		A15 =bank &2? 0x04: 0x00;
		A17A18 =addr &0x30;
		sync();
	}
	MMC3::write(bank, addr, val);
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	A15 =0x00;
	A16 =0x00;
	A17A18 =0x00;
	MMC3::reset(RESET_HARD);
	
	readCart =EMU->GetCPUReadHandler(0x8);
	for (int bank =0x8; bank<=0xB; bank++) EMU->SetCPUReadHandler(bank, readLatch);
	for (int bank =0x8; bank<=0xF; bank++) EMU->SetCPUWriteHandler(bank, writePAL);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, A15);
	SAVELOAD_BYTE(stateMode, offset, data, A16);
	SAVELOAD_BYTE(stateMode, offset, data, A17A18);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t MapperNum =383;
} // namespace

MapperInfo MapperInfo_383 = {
	&MapperNum,
	_T("晶太 YY840708C"),
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