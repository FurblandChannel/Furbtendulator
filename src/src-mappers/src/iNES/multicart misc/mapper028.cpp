#include	"..\..\DLL\d_iNES.h"

namespace {
uint8_t		index;
uint8_t		reg[4];

void	sync (void) {
	int prgAND =15 >>(~reg[2] >>4 &3);
	int prgOR  =reg[3] <<1;

	EMU->SetPRG_RAM8(0x6, 0);	
	switch (reg[2] >>2 &3) {
		case 0:	case 1:
			EMU->SetPRG_ROM16(0x8, reg[1] <<1 &prgAND |0 | prgOR &~prgAND);
			EMU->SetPRG_ROM16(0xC, reg[1] <<1 &prgAND |1 | prgOR &~prgAND);
			break;
		case 2: EMU->SetPRG_ROM16(0x8,                     0 | prgOR         );
			EMU->SetPRG_ROM16(0xC, reg[1]     &prgAND    | prgOR &~prgAND);
			break;
		case 3: EMU->SetPRG_ROM16(0x8, reg[1]     &prgAND    | prgOR &~prgAND);
			EMU->SetPRG_ROM16(0xC,                     1 | prgOR         );
			break;
	}
	EMU->SetCHR_RAM8(0x0, reg[0] &3);
	switch (reg[2] &3) {
		case 0:	EMU->Mirror_S0(); break;
		case 1:	EMU->Mirror_S1(); break;
		case 2:	EMU->Mirror_V (); break;
		case 3:	EMU->Mirror_H (); break;
	}
}

void	MAPINT	writeIndex (int bank, int addr, int val) {
	index =val >>6 &2 | val &1;
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	reg[index] =val;
	
	// Update one-screen mirroring mode bit, if applicable
	if (~index &2 && ~reg[2] &2) reg[2] =reg[2] &~1 | val >>4 &1;
	
	sync();
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) for (auto& c: reg) c =0xFF;
	sync();
	
	EMU->SetCPUWriteHandler(0x5, writeIndex);
	for (int bank =0x8; bank<=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeReg);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, index);
	for (auto& c: reg) SAVELOAD_BYTE(stateMode, offset, data, c);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =28;
} // namespace

MapperInfo MapperInfo_028 ={
	&mapperNum,
	_T("Action 53"),
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