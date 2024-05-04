#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\Sound\s_FDS.h"

#define	horizontalMirroring mirroring &0x08
#define romSelect          (mode      &0x10)

namespace {
uint8_t		prg;
uint8_t		mirroring;
uint8_t		mode;
FCPURead	readCart;

void	sync (void) {
	if (romSelect)
		EMU->SetPRG_ROM8(0x6, prg);
	else
		EMU->SetPRG_RAM8(0x6, 0);
	EMU->SetPRG_ROM32(0x8, 3);
	
	EMU->SetCHR_RAM8(0, 0);
	
	if (horizontalMirroring)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

void	MAPINT	writePRG (int bank, int addr, int val) {
	prg =val;
	sync();
}

int	MAPINT	trapCartRead (int bank, int addr) {
	addr |=bank <<12;
	if (!romSelect && addr >=0xB800 && addr <=0xD7FF)
		return ROM->PRGRAMData[addr -0xB800 +0x2000];
	else
		return readCart(bank, addr &0xFFF);
}

void	MAPINT	trapCartWrite (int bank, int addr, int val) {
	addr |=bank <<12;
	if (addr >=0x6000 && addr <=0x7FFF) ROM->PRGRAMData[addr -0x6000        ] =val; else
	if (addr >=0xB800 && addr <=0xD7FF) ROM->PRGRAMData[addr -0xB800 +0x2000] =val;
}

void	MAPINT	writeMirroring (int bank, int addr, int val) {
	mirroring =val;
	sync();
}

void	MAPINT	writeMode (int bank, int addr, int val) {
	mode =val;
	sync();
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE ResetType) {
	if (ResetType ==RESET_HARD) {
		prg =0;
		mirroring =0;
		mode =0;
	}
	sync();

	// Writes always go to WRAM regardless of mode register setting
	for (int bank =0x6; bank <=0x7; bank++) EMU->SetCPUWriteHandler(bank, trapCartWrite);
	for (int bank =0xB; bank <=0xD; bank++) EMU->SetCPUWriteHandler(bank, trapCartWrite);

	// Reads from B800-D7FF must be handled manually because of the odd address boundary
	readCart =EMU->GetCPUReadHandler(0x8);
	for (int bank =0xB; bank <=0xD; bank++) {
		EMU->SetCPUReadHandler     (bank, trapCartRead);
		EMU->SetCPUReadHandlerDebug(bank, trapCartRead);
	}
	EMU->SetCPUWriteHandler(0x8, writePRG);
	EMU->SetCPUWriteHandler(0xE, writeMirroring);
	EMU->SetCPUWriteHandler(0xF, writeMode);
	
	FDSsound::ResetBootleg(ResetType);
}

int	MAPINT	SaveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, prg);
	SAVELOAD_BYTE(stateMode, offset, data, mirroring);
	SAVELOAD_BYTE(stateMode, offset, data, mode);
	offset =FDSsound::SaveLoad(stateMode, offset, data);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =103;
} // namespace

MapperInfo MapperInfo_103 ={
	&mapperNum,
	_T("Whirlwind Manu LH30"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	NULL,
	NULL,
	SaveLoad,
	FDSsound::GetBootleg,
	NULL
};