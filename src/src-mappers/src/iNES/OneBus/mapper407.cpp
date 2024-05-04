#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_OneBus.h"

namespace {
void	sync () { 
	OneBus::syncPRG(0x0FFF, 0);
	OneBus::syncCHR(0x7FFF, 0);
	OneBus::syncMirror();	
	
	EMU->SetPRG_OB4(0x6);
	EMU->SetPRG_OB4(0x7);
	
	// Make PRG-ROM writable if it points to banks $80-$87
	if ((OneBus::reg4100[0x07] &0xF8) ==0x80) for (int bank =0x8; bank<=0x9; bank++) EMU->SetPRG_Ptr4(bank, EMU->GetPRG_Ptr4(bank), TRUE);
	if ((OneBus::reg4100[0x08] &0xF8) ==0x80) for (int bank =0xA; bank<=0xB; bank++) EMU->SetPRG_Ptr4(bank, EMU->GetPRG_Ptr4(bank), TRUE);
}

void	MAPINT	writeRAM (int bank, int addr, int val) {
	// First, write to PRG-"ROM" itself
	OneBus::writeMMC3 (bank, addr, val);
	
	// Update the chrLow and chrHigh arrays if necessary
	int a13a20 =OneBus::reg4100[0x07 +(bank >=0xA? 1: 0)];
	if ((a13a20 &0xF8) ==0x80) {
		int romAddr =a13a20 <<13 | (bank &1) <<12 | addr;
		int shiftedAddress =romAddr &0xF | romAddr >>1 &~0xF;
		if (romAddr &0x10)
			OneBus::chrHigh[shiftedAddress] =val;
		else
			OneBus::chrLow[shiftedAddress] =val;
	}
}

BOOL	MAPINT	load (void) {
	// Force a 2 MiB address space
	EMU->SetPRGROMSize(0x200000);
	OneBus::load(sync);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	// Duplicate the first 512 KiB to the last within the 2 MiB address space
	if (resetType ==RESET_HARD) for (int i =0; i <0x80000; i++) {
		uint8_t c =ROM->PRGROMData[i +0x000000];
		ROM->PRGROMData[i +0x080000] =0;
		ROM->PRGROMData[i +0x100000] =0;
		ROM->PRGROMData[i +0x180000] =c;
	}
	OneBus::reset(resetType);
	for (int bank =0x8; bank<=0xD; bank++) EMU->SetCPUWriteHandler(bank, writeRAM);
}

uint16_t mapperNum =407;
} // namespace

MapperInfo MapperInfo_407 = {
	&mapperNum,
	_T("Win, Lose & Draw"),
	COMPAT_FULL,
	load,
	reset,
	OneBus::unload,
	OneBus::cpuCycle,
	OneBus::ppuCycle,
	OneBus::saveLoad,
	NULL,
	NULL
};