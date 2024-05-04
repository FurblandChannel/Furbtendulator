#include	"h_N118.h"

namespace N118 {
uint8_t		pointer;
uint8_t		reg[8];
FSync		sync;

void	syncPRG (int AND, int OR) {
	EMU->SetPRG_ROM8(0x8, reg[6] &AND |OR);
	EMU->SetPRG_ROM8(0xA, reg[7] &AND |OR);
	EMU->SetPRG_ROM8(0xC, 0xFE   &AND |OR);
	EMU->SetPRG_ROM8(0xE, 0xFF   &AND |OR);
}

void	syncCHR (int AND, int OR) {
	EMU->SetCHR_ROM1(0, reg[0] &~1 &AND |OR);
	EMU->SetCHR_ROM1(1, reg[0] | 1 &AND |OR);
	EMU->SetCHR_ROM1(2, reg[1] &~1 &AND |OR);
	EMU->SetCHR_ROM1(3, reg[1] | 1 &AND |OR);
	EMU->SetCHR_ROM1(4, reg[2]     &AND |OR);
	EMU->SetCHR_ROM1(5, reg[3]     &AND |OR);
	EMU->SetCHR_ROM1(6, reg[4]     &AND |OR);
	EMU->SetCHR_ROM1(7, reg[5]     &AND |OR);
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	if (addr &1)
		reg[pointer &7] =val;
	else 
		pointer =val;
	sync();
}

void	MAPINT	load (FSync cSync) {
	sync =cSync;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		pointer =0x00;
		reg[0] =0x00; reg[1] =0x02;
		reg[2] =0x04; reg[3] =0x05; reg[4] =0x06; reg[5] =0x07;
		reg[6] =0x00; reg[7] =0x01;
	}
	for (int bank =0x8; bank<=0x9; bank++) EMU->SetCPUWriteHandler(bank, writeReg);
	sync();
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, pointer);
	for (auto& c: reg) SAVELOAD_BYTE(stateMode, offset, data, c);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

} // namespace N118