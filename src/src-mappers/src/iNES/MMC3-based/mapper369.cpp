#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
FCPUWrite writeAPU;
bool m2Counting;
uint16_t m2Counter;
uint8_t outerBank;
uint8_t smb2jBank;

void	sync (void) {
	switch(outerBank) {
		case 0x00:	EMU->SetPRG_ROM32(0x8, 0x00);
				EMU->SetCHR_ROM8 (0x0, 0x00);
				break;
		case 0x01:	EMU->SetPRG_ROM32(0x8, 0x01);
				EMU->SetCHR_ROM8 (0x0, 0x01);
				break;
		case 0x13:	EMU->SetPRG_ROM8 (0x6, 0x0E);
				EMU->SetPRG_ROM8 (0x8, 0x0C);
				EMU->SetPRG_ROM8 (0xA, 0x0D);
				EMU->SetPRG_ROM8 (0xC, 0x08 |(smb2jBank &0x03));
				EMU->SetPRG_ROM8 (0xE, 0x0F);
				EMU->SetCHR_ROM8 (0x0, 0x03);
				break;
		case 0x37:	MMC3::syncPRG(0x0F, 0x10);
				MMC3::syncCHR_ROM(0x7F, 0x080);
				MMC3::syncWRAM();
				break;
		case 0xFF:	MMC3::syncPRG(0x1F, 0x20);
				MMC3::syncCHR_ROM(0x7F, 0x100);
				MMC3::syncWRAM();
				break;
	}
	MMC3::syncMirror();
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	writeAPU(bank, addr, val);
	if (addr &0x100) {
		outerBank =val;
		sync();
	}
}

void	MAPINT	writeIRQAcknowledge (int bank, int addr, int val) {
	if (outerBank ==0x13) {
		EMU->SetIRQ(1);
		m2Counting =false;
	}
	MMC3::writeReg(bank, addr, val);
}

void	MAPINT	writeIRQEnable (int bank, int addr, int val) {
	if (outerBank ==0x13) m2Counting =val &&2;
	MMC3::writeMirroringWRAM(bank, addr, val);
}

void	MAPINT	writeSMB2JBank (int bank, int addr, int val) {
	if (outerBank ==0x13) {
		smb2jBank =val;
		sync();
	} else
		MMC3::writeIRQEnable(bank, addr, val);
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) outerBank =0;
	m2Counting =false;
	m2Counter =0;
	MMC3::reset(resetType);	

	writeAPU =EMU->GetCPUWriteHandler(0x4);
	EMU->SetCPUWriteHandler(0x4, writeReg);
	for (int bank =0x8; bank<=0x9; bank++) EMU->SetCPUWriteHandler(bank, writeIRQAcknowledge);
	for (int bank =0xA; bank<=0xB; bank++) EMU->SetCPUWriteHandler(bank, writeIRQEnable);
	for (int bank =0xE; bank<=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeSMB2JBank);
}

void	MAPINT	cpuCycle (void) {
	if (outerBank ==0x13) {
		if (m2Counting && !(++m2Counter &0xFFF)) EMU->SetIRQ(0);
	} else
		MMC3::cpuCycle();
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, outerBank);
	SAVELOAD_BYTE(stateMode, offset, data, smb2jBank);
	SAVELOAD_BOOL(stateMode, offset, data, m2Counting);
	SAVELOAD_WORD(stateMode, offset, data, m2Counter);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t MapperNum =369;
} // namespace

MapperInfo MapperInfo_369 ={
	&MapperNum,
	_T("N49C-300"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	cpuCycle,
	MMC3::ppuCycle,
	saveLoad,
	NULL,
	NULL
};
