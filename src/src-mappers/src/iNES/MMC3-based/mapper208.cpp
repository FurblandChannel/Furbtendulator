#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

#define prg       (reg &1 | reg >>3 &2)
#define mirrorH !!(reg &0x20)
namespace {
uint8_t		reg;
uint8_t		protectionIndex;
uint8_t		protectionData[4];

void	sync (void) {
	if (ROM->INES2_SubMapper ==1) {
		EMU->SetPRG_ROM32(0x8, MMC3::reg[6] >>2);
		MMC3::syncMirror();
	} else {
		EMU->SetPRG_ROM32(0x8, prg);
		if (mirrorH)
			EMU->Mirror_H();
		else
			EMU->Mirror_V();
	}
	MMC3::syncCHR_ROM(0xFF, 0);
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync);
	return TRUE;
}

int	MAPINT	readProtection (int bank, int addr) {
	return addr &0x800? protectionData[addr &3]: *EMU->OpenBus;
}

#define i0 !!(val &0x01)
#define i1 !!(val &0x02)
#define i2 !!(val &0x04)
#define i3 !!(val &0x08)
#define i4 !!(val &0x10)
#define i5 !!(val &0x20)
#define i6 !!(val &0x40)
#define i7 !!(val &0x80)
uint8_t	protectionXOR (uint8_t val) {
	return 0x01*( i7 && !i3 || !i7 && !i6) |
	       0x08*( i4 && !i3 || !i6 && !i4) |
	       0x10*(!i6 && !i0 || !i3 &&  i0) |
	       0x40*(!i6 && !i1 || !i3 &&  i1);
}
uint8_t	protectionAND (uint8_t val) {
	return 0xA6 |
	       0x01*(!i7 && !i6 && !i5 || i7 && !i3 && !i2 || !i7 &&  i6 &&  i5 || i7 & i3 & i2) |
	       0x08*(!i6 && !i5 && !i4 || i6 &&  i5 && !i4 ||  i4 && !i3 && !i2 || i4 & i3 & i2) |
	       0x10*(!i6 && !i5 && !i0 || i6 &&  i5 && !i0 || !i3 && !i2 &&  i0 || i3 & i2 & i0) |
	       0x40*(!i6 && !i5 && !i1 || i6 &&  i5 && !i1 || !i3 && !i2 &&  i1 || i3 & i2 & i1);	       
}

void	MAPINT	writeProtection (int bank, int addr, int val) {	
	if (addr &0x800)
		protectionData[addr &3] =(val &protectionAND(protectionIndex)) ^protectionXOR(protectionIndex);
	else
		protectionIndex =val;
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	EMU->WriteAPU(bank, addr, val);
	if (addr &0x800) {
		reg =val;
		sync();
	}
}

void	MAPINT	reset (RESET_TYPE resetType) {
	reg =0;
	protectionIndex =0;
	for (auto& r: protectionData) r =0;
	if (ROM->INES2_SubMapper !=1) {
		EMU->SetCPUWriteHandler(0x4, writeReg);
		EMU->SetCPUReadHandler(0x5, readProtection);
		EMU->SetCPUReadHandlerDebug(0x5, readProtection);
		EMU->SetCPUWriteHandler(0x5, writeProtection);		
	}
	MMC3::reset(resetType);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	if (ROM->INES2_SubMapper !=1) {
		SAVELOAD_BYTE(stateMode, offset, data, reg);
		SAVELOAD_BYTE(stateMode, offset, data, protectionIndex);
		for (auto& r: protectionData) SAVELOAD_BYTE(stateMode, offset, data, r);
	}
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =208;
} // namespace

MapperInfo MapperInfo_208 ={
	&mapperNum,
	_T("哥德 SL-37017"),
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
