#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
uint8_t		reg[4];
FCPURead	readCart;

#define locked  (reg[0] &0x80)
#define readDIP (reg[0] &0x40)
#define	prgAND  (reg[1] &0x04? 0x0F: 0x1F)
#define	chrAND  (reg[1] &0x40? 0x7F: 0xFF)
#define prgOR   (reg[1] <<4 &0x030 | reg[0] <<2 &0x040)
#define chrOR   (reg[1] <<3 &0x180 | reg[0] <<4 &0x200)
#define nrom    (reg[1] &0x80)
#define nrom128 (reg[1] &0x08)
#define cnrom   (reg[0] &0x08)

void	sync (void) {
	if (nrom)
		MMC3::syncPRG_GNROM_66 (nrom128? 0: 2, prgAND, prgOR);
	else
		MMC3::syncPRG(prgAND, prgOR);
	
	if (cnrom)
		EMU->SetCHR_ROM8(0, reg[2] &chrAND >>3 | chrOR >>3);
		//MMC3::syncCHR(chrAND, chrOR &~0x18 | reg[2] <<3 &0x18);
	else
		MMC3::syncCHR(chrAND, chrOR);
	
	MMC3::syncMirror();
	MMC3::syncWRAM();
}

int	MAPINT	interceptCartRead (int bank, int addr) {
	if (readDIP)
		return ROM->dipValue;
	else
		return readCart(bank, addr);
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	MMC3::wramWrite(bank, addr, val);
	if (!locked) {
		reg[addr &3] =val;
		sync();
	} else
	if ((addr &3) ==2) {
		reg[2] =reg[2] &~3 | val &3;
		sync();
	}
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	for (auto& r: reg) r =0x00;
	MMC3::reset(resetType);
	MMC3::setWRAMCallback(NULL, writeReg);

	readCart =EMU->GetCPUReadHandler(0x8);
	for (int bank =0x8; bank<=0xF; bank++) {
		EMU->SetCPUReadHandler(bank, interceptCartRead);
		EMU->SetCPUReadHandlerDebug(bank, interceptCartRead);
	}
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	for (auto& r: reg) SAVELOAD_BYTE(stateMode, offset, data, r);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =134;
} // namespace

MapperInfo MapperInfo_134 ={
	&mapperNum,
	_T("WX-KB4K/T4A54A/BS-5652"),
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