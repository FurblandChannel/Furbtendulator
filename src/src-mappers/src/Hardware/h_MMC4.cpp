#include	"h_MMC4.h"
#include	"../Dll/d_iNES.h"

namespace MMC4 {
uint8_t		prg;
uint8_t		chr[4];
uint8_t		state[2];
uint8_t		mirroring;
FSync		sync;
FPPURead	readCHR;

void	syncPRG (int AND, int OR) {
	EMU->SetPRG_ROM16(0x8, prg  &AND |OR);
	EMU->SetPRG_ROM16(0xC, 0xFF &AND |OR);
}

void	syncCHR (int AND, int OR) {
	iNES_SetCHR_Auto4(0, chr[state[0] |0] &AND |OR);
	iNES_SetCHR_Auto4(4, chr[state[1] |2] &AND |OR);
}

void	syncMirror (void) {
	if (mirroring &1)
		EMU->Mirror_H();
	else	
		EMU->Mirror_V();
}

int	MAPINT	trapCHRRead (int bank, int addr) {
	int result =readCHR(bank, addr); // Switch occurs *after* byte is read
	switch (addr &0x3F8) {
		case 0x3D8: state[bank >>2] =0; sync(); break; 
		case 0x3E8: state[bank >>2] =1; sync(); break; 
	}
	return result;
}

void	MAPINT	writePRG (int bank, int addr, int val) {
	prg =val;
	sync();
}

void	MAPINT	writeCHR (int bank, int addr, int val) {
	chr[bank -0xB] =val;
	sync();
}

void	MAPINT	writeMirroring (int bank, int addr, int val) {
	mirroring =val;
	sync();
}

void	MAPINT	load (FSync cSync) {
	sync =cSync;
}
void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		prg =0;
		for (auto& c: chr) c =0;
		for (auto& c: state) c =0;
		mirroring =0;
	}
	sync();
	
	EMU->SetCPUWriteHandler(0xA, writePRG);
	for (int bank =0xB; bank<=0xE; bank++) EMU->SetCPUWriteHandler(bank, writeCHR);
	EMU->SetCPUWriteHandler(0xF, writeMirroring);
	
	readCHR =EMU->GetPPUReadHandler(0x3);
	EMU->SetPPUReadHandler(0x3, trapCHRRead);
	EMU->SetPPUReadHandler(0x7, trapCHRRead);
	EMU->SetPPUReadHandlerDebug(0x3, readCHR);
	EMU->SetPPUReadHandlerDebug(0x7, readCHR);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, prg);
	for (auto& c: chr) SAVELOAD_BYTE(stateMode, offset, data, c);
	for (auto& c: state) SAVELOAD_BYTE(stateMode, offset, data, c);
	SAVELOAD_BYTE(stateMode, offset, data, mirroring);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}
} // namespace MMC4