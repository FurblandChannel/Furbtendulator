#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"
#include	"..\..\Hardware\Sound\s_FDS.h"

namespace {
uint8_t OuterBank;
FCPURead _Read4;
FCPUWrite _Write4;

void	Sync (void) {
	// PRG
	MMC3::syncWRAM();
	if (OuterBank ==2)
		MMC3::syncPRG(0x0F, ((MMC3::reg[0] &0x80)? 0x10: 0x00) | (OuterBank <<5));
	else
		MMC3::syncPRG(0x1F, OuterBank <<5);
	
	if (OuterBank ==3 && ~MMC3::reg[0] &0x80) {
		EMU->SetPRG_ROM8(0xC, MMC3::reg[6] | 0x70);
		EMU->SetPRG_ROM8(0xE, MMC3::reg[7] | 0x70);
	}
	
	// CHR
	if (OuterBank ==2 && MMC3::reg[0] &0x80)
		EMU->SetCHR_RAM8(0, 0);
	else
		MMC3::syncCHR_ROM(0x7F, OuterBank <<7);
	
	// NT
	if (OuterBank ==0)
		MMC3::syncMirrorA17();
	else
		MMC3::syncMirror();
}

int	MAPINT	Read4 (int Bank, int Addr) {
	if (Addr <0x20)
		return _Read4(Bank, Addr);
	else
		return FDSsound::Read((Bank <<12) |Addr);
}

void	MAPINT	Write4 (int Bank, int Addr, int Val) {
	_Write4(Bank, Addr, Val);
	if (Addr !=0x23) // Don't relay Master IO Disable
		FDSsound::Write((Bank <<12) |Addr, Val);
}

void	MAPINT	WriteROM (int Bank, int Addr, int Val) {
	if (Addr ==0x80) { // Actual mask unknown, but must be more than just AND 0x80.
		OuterBank =(Bank >>1) &3;
		Sync();
	} else
		MMC3::write(Bank, Addr, Val);
}

BOOL	MAPINT	Load (void) {
	MMC3::load(Sync);
	iNES_SetSRAM();
	return TRUE;
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	OuterBank =0; // M2 interruption resets PAL, so reset outer bank even on soft reset
	MMC3::reset(RESET_HARD);
	for (int i =0x8; i<=0xF; i++) EMU->SetCPUWriteHandler(i, WriteROM);

	FDSsound::Reset(ResetType);
	_Read4 =EMU->GetCPUReadHandler(0x4);
	_Write4 =EMU->GetCPUWriteHandler(0x4);
	EMU->SetCPUReadHandler(0x4, Read4);
	EMU->SetCPUWriteHandler(0x4, Write4);
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(mode, offset, data, OuterBank);
	offset = MMC3::saveLoad(mode, offset, data);
	offset = FDSsound::SaveLoad(mode, offset, data);
	if (mode ==STATE_LOAD) Sync();
	return offset;
}

static int MAPINT MapperSnd (int Cycles) {
	return *EMU->BootlegExpansionAudio? FDSsound::Get(Cycles): 0;
}

uint16_t MapperNum =353;
} // namespace

MapperInfo MapperInfo_353 ={
	&MapperNum,
	_T("81-03-05-C"),
	COMPAT_FULL,
	Load,
	Reset,
	NULL,
	MMC3::cpuCycle,
	MMC3::ppuCycle,
	SaveLoad,
	MapperSnd,
	NULL
};
