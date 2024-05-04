#include	"..\..\DLL\d_iNES.h"

namespace {
uint8_t		reg[2];
FCPURead	readCart;

#define	mode                (reg[1] >>4 &3)
#define chr                 (ROM->CHRROMSize? (reg[0] &0x0F): 0)
#define	prg                 (ROM->CHRROMSize? (reg[1] &0x0F): (reg[1] &0x07 | reg[0] <<3))
#define horizontalMirroring (reg[0] &0x20)

void	sync (void) {
	switch(mode) {
		case 0:
		case 1:	EMU->SetPRG_ROM16(0x8, prg   );
			EMU->SetPRG_ROM16(0xC, prg |7);
			break;
		case 2: EMU->SetPRG_ROM32(0x8, prg >>1);
			break;
		case 3:	EMU->SetPRG_ROM16(0x8, prg);
			EMU->SetPRG_ROM16(0xC, prg);
			break;
	}
	iNES_SetCHR_Auto8(0, chr);
	if (horizontalMirroring)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

int	MAPINT	trapCartRead (int bank, int addr) {
	if (mode ==1) addr =addr &~0xF | ROM->dipValue;
	return readCart(bank, addr);
}	

void	MAPINT	writeReg (int bank, int addr, int val) {
	reg[bank >>2 &1] =addr &0xFF;
	sync();
}

BOOL	MAPINT	load (void) {
	MapperInfo_236.Description =ROM->INES_CHRSize? (ROM->INES_PRGSize >128/16? _T("Realtec 8099"): _T("Realtec 8031/8155")): _T("Realtec 8106");
	return TRUE;
}	

void	MAPINT	reset (RESET_TYPE resetType) {
	for (auto& c: reg) c =0x00;
	sync();
	
	readCart =EMU->GetCPUReadHandler(0x8);
	for (int bank =0x8; bank <=0xF; bank++) {
		EMU->SetCPUReadHandler(bank, trapCartRead);
		EMU->SetCPUReadHandlerDebug(bank, trapCartRead);
		EMU->SetCPUWriteHandler(bank, writeReg);
	}
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	for (auto& c: reg) SAVELOAD_BYTE(stateMode, offset, data, c);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =236;
} // namespace

MapperInfo MapperInfo_236 = {
	&mapperNum,
	_T("Realtec 8031/8099/8106/8155"),
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