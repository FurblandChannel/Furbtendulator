#include	"..\..\DLL\d_iNES.h"

#define prg                (reg[1] >>5)
#define outerCHR           (reg[1] &0x07 | reg[0] >>3 &0x08)
#define innerCHR           (reg[0] &0x03)
#define cnrom              ~reg[0] &0x80
#define	nrom256             reg[1] &0x10
#define horizontalMirroring reg[1] &0x08
namespace {
uint8_t		reg[2];

void	sync (void) {
	if (nrom256)
		EMU->SetPRG_ROM32(0x8, prg >>1);
	else {
		EMU->SetPRG_ROM16(0x8, prg);
		EMU->SetPRG_ROM16(0xC, prg);
	}
	if (cnrom)
		EMU->SetCHR_ROM8(0x0, innerCHR | outerCHR &~0x3);
	else
		EMU->SetCHR_ROM8(0x0, outerCHR);
	
	if (horizontalMirroring)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

int	MAPINT	readDIP (int bank, int addr) {
	return ROM->dipValue;
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	if (bank &2)
		reg[addr >>11] =reg[addr >>11] &~0x40 | val &0x40; // ???? ABCARD-02/19
	else
		reg[addr >>11] =val;	
	sync();
}

void	MAPINT	reset (RESET_TYPE resetType) {
	for (auto& c: reg) c =0;
	sync();

	EMU->SetCPUReadHandler(0x5, readDIP);
	EMU->SetCPUReadHandler(0x6, readDIP);
	for (int bank =0x8; bank<=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeReg);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, reg[0]);
	SAVELOAD_BYTE(stateMode, offset, data, reg[1]);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =57;
} // namespace

MapperInfo MapperInfo_057 ={
	&mapperNum,
	_T("GK 6-in-1"),
	COMPAT_FULL,
	NULL,
	reset,
	NULL,
	NULL,
	NULL,
	saveLoad,
	NULL,
	NULL
};