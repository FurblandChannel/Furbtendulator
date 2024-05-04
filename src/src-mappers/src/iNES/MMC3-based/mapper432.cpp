#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
FCPURead	readCart;
uint8_t		reg[2];

int	MAPINT	readPad (int, int);
void	sync (void) {
	int prgAND =reg[1] &0x02? 0x0F: 0x1F;
	int prgOR  =reg[1] <<4 &0x10 | reg[1] <<1 &0x60;
	int chrAND =reg[1] &0x04? 0x7F: 0xFF;
	int chrOR  =reg[1] <<7 &0x80 | reg[1] <<5 &0x100 | reg[1] <<4 &0x200;
	bool pad     =!!(reg[0] &0x01 || ROM->PRGROMSize <1024*1024 && reg[1] &0x20);
	bool nrom    =!!(reg[1] &0x40);
	bool nrom256 =!!(reg[1] &0x80);

	if (nrom)
		MMC3::syncPRG_GNROM_67(nrom256? 2: 0, prgAND, prgOR &~prgAND);
	else
		MMC3::syncPRG(prgAND, prgOR &~prgAND);
	MMC3::syncCHR(chrAND, chrOR &~chrAND);
	MMC3::syncWRAM();
	MMC3::syncMirror();
	for (int bank =0x8; bank <0xF; bank++) EMU->SetCPUReadHandler(bank, pad? readPad: readCart);
}

int	MAPINT	readPad (int bank, int addr) {
	return ROM->dipValue;
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	reg[addr &1] =val &0xFF;
	if (ROM->PRGROMSize <1024*1024 && ~addr &1 && ~val &1) reg[1] &=~0x20;
	sync();
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	readCart =EMU->GetCPUReadHandler(0x8);
	for (auto& c: reg) c =0;
	MMC3::reset(resetType);
	MMC3::setWRAMCallback(NULL, writeReg);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	for (auto& c: reg) SAVELOAD_BYTE(stateMode, offset, data, c);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =432;
} // namespace

MapperInfo MapperInfo_432 = {
	&mapperNum,
	_T("Realtec 8023/8043/8086/8090"),
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
