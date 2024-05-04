#include	"..\DLL\d_iNES.h"
#include	"smc5000.h"

namespace {
FCPURead	readAPU;
FPPURead	readCHR;
FCPUWrite	writeAPU;
FCPUWrite	writeCart;
FPPUWrite	writeCHR;

uint8_t		mc1Mode;
uint8_t		mc2Mode;
uint8_t		smcMode;
uint8_t		latch;
bool		lockCHR;

uint8_t		prg8K[4];
uint8_t		chr1K[12];
uint8_t		latchMMC4[2];

bool		smcIRQ;
uint16_t	smcPrevPA;
int16_t		smcCounter;

uint8_t		fdsIO;
int16_t		fdsCounter;

uint8_t		scratchRAM[0x1000];

int	MAPINT	readCHR_MMC4 (int, int);
void	MAPINT	interceptCHRWrite (int, int, int);

void	sync (void) {
	EMU->SetPRG_Ptr4(0x5, scratchRAM, TRUE);
	EMU->SetPRG_RAM8(0x6, 0);
	if (~mc2Mode &0x01) /* 2M or 4M PRG mode */
		for (int bank =0; bank <4; bank++) EMU->SetPRG_ROM8(0x8 +bank*2, prg8K[bank]);
	else                /* 1M PRG mode */
	switch(mc1Mode >>5) {
		case 0:	EMU->SetPRG_ROM16(0x8, latch &7);
			EMU->SetPRG_ROM16(0xC, 7);
			break;
		case 1:	EMU->SetPRG_ROM16(0x8, latch >>2 &15);
			EMU->SetPRG_ROM16(0xC, 7);
			break;
		case 2:	EMU->SetPRG_ROM16(0x8, latch &15);
			EMU->SetPRG_ROM16(0xC, 15);
			break;
		case 3:	EMU->SetPRG_ROM16(0x8, 15);
			EMU->SetPRG_ROM16(0xC, latch &15);
			break;
		case 4:	EMU->SetPRG_ROM32(0x8, latch >>4 &3);
			break;
		case 5:	EMU->SetPRG_ROM32(0x8, 3);
			break;
		case 6:	EMU->SetPRG_ROM32(0x8, 3);
			break;
		case 7:	EMU->SetPRG_ROM32(0x8, 3);
			break;
	}
	/* Write-enable PRG DRAM if the PRG write-protect bit is not set */
	if (~mc1Mode &0x02) for (int bank =0x8; bank<=0xF; bank++) EMU->SetPRG_Ptr4(bank, (EMU->GetPRG_Ptr4(bank)), TRUE);

	/* Mode-specific CHR-RAM banking */
	if (smcMode &0x01) { /* Super Magic Card CHR mode */
		if (smcMode &0x04) /* 1 KiB CHR mode */
			for (int bank =0; bank <8; bank++) EMU->SetCHR_RAM1(bank, chr1K[bank]);
		else               /* MMC4-like CHR mode */
			for (int bank =0; bank <8; bank +=4) EMU->SetCHR_RAM4(bank, chr1K[bank | latchMMC4[bank >>2]] >>2);
	} else               /* 1M-mode-specific CHR banking */
	switch(mc1Mode >>5) {
		case 0: case 2:
			EMU->SetCHR_RAM8(0x0, 0);
			break;
		case 1: case 4: case 5:
			EMU->SetCHR_RAM8(0x0, latch &3);
			break;
		case 3:
			EMU->SetCHR_RAM8(0x0, latch >>4 &3);
			break;
		case 6:
			EMU->SetCHR_RAM8(0x0, latch &1);
			break;
		case 7:
			EMU->SetCHR_RAM8(0x0, 3);
			break;
	}
	/* CHR-RAM is write-protected ...
	   - ... if 1M mode GNROM with two-screen mirroring is active;
	   - ... if 1M mode (C)NROM is active;
	   - ... if CHR-RAM has been locked before. */
	if ((mc1Mode &0xE1) >=0x81 || lockCHR) protectCHRRAM();

	if (smcMode &0x02) switch(mc1Mode &0x11) { /* CIRAM nametables */
		case 0x00: EMU->Mirror_S0(); break;
		case 0x10: EMU->Mirror_S1(); break;
		case 0x01: EMU->Mirror_V (); break;
		case 0x11: EMU->Mirror_H (); break;
	} else                                    /* CHR-RAM nametables */
		for (int bank =8; bank <16; bank++) iNES_SetCHR_Auto1(bank, chr1K[bank &~4]);

	for (int bank =0; bank <8; bank++) {
		EMU->SetPPUReadHandler(bank, (smcMode &0x05) ==0x01? readCHR_MMC4: readCHR);
		EMU->SetPPUReadHandlerDebug(bank, readCHR);
		EMU->SetPPUWriteHandler(bank, mc1Mode >=0xA0 && ~mc1Mode &1? interceptCHRWrite: writeCHR); /* In modes 5-7 with single-screen mirroring, PPU writes lock CHR-RAM. */
	}
}

int	MAPINT	readCHR_MMC4 (int bank, int addr) {
	int result =readCHR(bank, addr);
	if ((bank &3) ==3 && ((addr &0x3F8) ==0x3D8 || (addr &0x3F8) ==0x3E8)) {
		latchMMC4[bank >>2 &1] =addr >>4 &2;
		sync();
	}
	return result;
}

int	MAPINT	readReg (int bank, int addr) {
	switch(addr) {
		case 0x500:
			return smcMode;
		default:
			return readAPU(bank, addr);
	}
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	writeAPU(bank, addr, val);
	switch (addr) {
		case 0x024:
			EMU->SetIRQ(1);
			break;
		case 0x025:
			EMU->SetIRQ(1);
			if (!smcIRQ) {
				fdsIO =val;
				if (val &0x42) fdsCounter =0;
			}
			break;
		case 0x2FC: case 0x2FD: case 0x2FE: case 0x2FF:
			mc1Mode =val &0xF0 | addr &0x03;
			if (mc1Mode >=0x80) lockCHR =false; /* Setting modes 4-7 releases the CHR-RAM lock */
			sync();
			break;
		case 0x3FC: case 0x3FD: case 0x3FE: case 0x3FF:
			mc2Mode =val &0xF0 | addr &0x03;
			latch =val;
			sync();
			break;
		case 0x500:
			smcMode =val;
			sync();
			break;
		case 0x501:
			smcIRQ =false;
			EMU->SetIRQ(1);
			break;
		case 0x502:
			smcCounter =smcCounter &0xFF00 | val;
			EMU->SetIRQ(1);
			break;
		case 0x503:
			smcIRQ =true;
			smcCounter =smcCounter &0x00FF | val <<8 ;
			EMU->SetIRQ(1);
			break;
		case 0x504: case 0x505: case 0x506: case 0x507:
			if (mc2Mode &0x02) val >>=2; /* The two bottom bits are unused when 4M mode is not active */
			prg8K[addr &3] =val;
			sync();
			break;
		case 0x510: case 0x511: case 0x512: case 0x513: case 0x514: case 0x515: case 0x516: case 0x517: case 0x518: case 0x519: case 0x51A: case 0x51B:
			chr1K[addr &0x0F] =val;
			sync();
			break;
	}
}

void	MAPINT	writeLatch (int bank, int addr, int val) {
	if (mc1Mode &0x02) {
		latch =val;
		if (mc2Mode &0x03) prg8K[bank >>1 &3] =val >>2; /* The two bottom bits are unused when 4M mode is not active */
		sync();
	} else
		writeCart(bank, addr, val);
}

void	MAPINT	interceptCHRWrite (int bank, int addr, int val) {
	/* Writing anything to CHR-RAM during modes 5-7 with one-screen mirroring locks (CIRAM page 1) or unlocks (CIRAM page 0) CHR memory in modes 0-3. Needed for (F4040) Karnov). */
	lockCHR =!!(mc1Mode &0x10);
	writeCHR(bank, addr, val);
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	// Mapper 12 puts CHR data in the PRG address space
	if (ROM->INES_MapperNum ==12) {
		EMU->SetPRGROMSize(0x80000);
		for (unsigned int i =0; i <ROM->CHRROMSize; i++) ROM->PRGROMData[0x40000 +i] =ROM->CHRROMData[i];
		EMU->SetCHRROMSize(0);
		ROM->INES_CHRSize =0;
	}
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE ResetType) {
	readCHR =EMU->GetPPUReadHandler(0x0);
	writeCHR =EMU->GetPPUWriteHandler(0x0);

	if (ResetType ==RESET_HARD) {
		// The Front Fareast Super Magic Card only has CHR-RAM, that can be write-protected. Many .NES ROM files supply CHR-ROM data instead, so copy it to CHR-RAM space
		if (ROM->CHRROMSize) {
			if (ROM->CHRRAMSize ==0) EMU->SetCHRRAMSize(ROM->CHRROMSize);
			for (unsigned int i =0; i <ROM->CHRROMSize; i++) ROM->CHRRAMData[i] =ROM->CHRROMData[i];
		}

		// iNES 1.0 to NES 2.0 translation
		if (ROM->INES_Version <2 && ROM->INES_MapperNum ==6) {
			// iNES 1.0 Mapper 6 means mode 1
			ROM->INES2_SubMapper =1;
		}
		if (ROM->INES_MapperNum ==8) {
			// iNES Mapper 8 means mode 4
			ROM->INES_MapperNum =6;
			ROM->INES2_SubMapper =4;
		}

		// Initial banking mode, mirroring and PRG memory protection
		mc1Mode =(ROM->INES_MapperNum ==6? ROM->INES2_SubMapper: 1) <<5
		      |(ROM->INES_Flags &1? 0x01: 0x11)
		      | 0x02
		;
		lockCHR =false;
		
		// 4M Mode is initially set for Magic Card 4M files (mapper 12) and Super Magic Card files (mapper 17).
		mc2Mode =ROM->INES_MapperNum ==12 || ROM->INES_MapperNum ==17? 0x00: 0x03;
		// Mapper 17 files start with CHR 1 KiB mode enabled; CIRAM enabled, MMC4 mode disabled
		smcMode =ROM->INES_MapperNum ==17? 0x47: 0x42;
		latch =0;

		for (int bank =0; bank <4; bank++) prg8K[3 -bank] =ROM->INES_PRGSize *2 -bank -1;
		for (int bank =0; bank <8; bank++) chr1K[bank] =bank;

		// Initialize scratch RAM
		// A few games jump to an NMI routine in scratch RAM
		sync(); // Set up the cart mapping to get the NMI vector
		uint16_t nmiHandler =EMU->GetPRG_Ptr4(0xF)[0xFFA] | EMU->GetPRG_Ptr4(0xF)[0xFFB] <<8;
		if (nmiHandler ==0x5032) {
			// If the NMI handler has been replaced already, put the original one back.
			EMU->GetPRG_Ptr4(0xF)[0xFFA] =scratchRAM[0x4F];
			EMU->GetPRG_Ptr4(0xF)[0xFFB] =scratchRAM[0x50];
		}
		// Copy data from BIOS
		for (unsigned int i =0; i <4096; i++) scratchRAM[i] =smc5000[i];
		// Put original NMI vector into scratch RAM
		scratchRAM[0x4F] =EMU->GetPRG_Ptr4(0xF)[0xFFA];
		scratchRAM[0x50] =EMU->GetPRG_Ptr4(0xF)[0xFFB];
		if (ROM->INES_MapperNum ==17) {
			// The original Super Magic Card BIOS redirects the NMI handler to a BIOS routine.
			// This redirection must be emulated to get the timing of M2 counter interrupts correct in Digital Devil Story: Megami Tensei II.
			EMU->GetPRG_Ptr4(0xF)[0xFFA] =0x32;
			EMU->GetPRG_Ptr4(0xF)[0xFFB] =0x50;
		}

		// Initialize the trainer
		if (ROM->MiscROMData && ROM->MiscROMSize) {
			uint16_t trainerAddr =ROM->INES_MapperNum !=17? 0x7000: // Always $7000 for Magic Card files (mapper not 17)
			                      ROM->INES2_SubMapper ==0? 0x7000: // Also $7000 for standard Super Magic Card files (mapper 17.0)
					      ROM->INES2_SubMapper <<8 |0x5C00; // Otherwise submapper denotes page from $5C00+.

			uint8_t* trainerData =trainerAddr <0x6000? &scratchRAM[trainerAddr &0xF00]: &ROM->PRGRAMData[trainerAddr &0x1F00];
			for (unsigned int i =0; i <ROM->MiscROMSize; i++) trainerData[i] =ROM->MiscROMData[i];

			// Super Magic Card trainers can be JMP'd to directly.
			// Magic Card trainers must be JSR'd to, and can change the reset vector. That code is in scratch RAM at $5000.
			EMU->SetPC(ROM->INES_MapperNum ==17? trainerAddr: 0x5000);
		}
		(EMU->GetCPUWriteHandler(0x4))(0x4, 0x017, 0x40); // Disable the frame IRQ. The Magic Card BIOS already does that, and some games depend on it.
	}
	sync();

	EMU->SetIRQ(1);
	smcIRQ =false;
	smcPrevPA =0;
	smcCounter =0;
	fdsIO =0;
	fdsCounter =0;

	readAPU   =EMU->GetCPUReadHandler(0x4);
	writeAPU  =EMU->GetCPUWriteHandler(0x4);
	writeCart =EMU->GetCPUWriteHandler(0x8);
	EMU->SetCPUReadHandler(0x4, readReg);
	EMU->SetCPUWriteHandler(0x4, writeReg);
	for (int bank =0x8; bank<=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeLatch);
}

void	clockSMCCounter (void) {
	if (smcIRQ) {
		if (smcCounter ==-1) {
			smcIRQ =false;
			EMU->SetIRQ(0);
		} else
			smcCounter++;
	}
}

void	MAPINT	cpuCycle (void) {
	// Simulate FDS Data IRQ, used by Magic Card 1M and 2M games for frame timing
	fdsCounter +=3;
	while (fdsCounter >=448 && fdsIO &0x80) {
		EMU->SetIRQ(0); // If the SMC IRQ is enabled, the FDS IRQ is blocked (->Rolling Thunder)
		fdsCounter -=448;
	}
	if (~smcMode &0x08) clockSMCCounter();
}

void	MAPINT	ppuCycle (int addr, int scanline, int cycle, int isRendering) {
	if (smcMode &0x08 && addr &0x1000 && ~smcPrevPA &0x1000) clockSMCCounter();
	smcPrevPA =addr;
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, mc1Mode);
	SAVELOAD_BYTE(stateMode, offset, data, mc2Mode);
	SAVELOAD_BYTE(stateMode, offset, data, smcMode);
	SAVELOAD_BYTE(stateMode, offset, data, latch);
	SAVELOAD_BOOL(stateMode, offset, data, lockCHR);
	for (auto& c: prg8K) SAVELOAD_BYTE(stateMode, offset, data, c);
	for (auto& c: chr1K) SAVELOAD_BYTE(stateMode, offset, data, c);
	for (auto& c: latchMMC4) SAVELOAD_BYTE(stateMode, offset, data, c);

	SAVELOAD_BOOL(stateMode, offset, data, smcIRQ);
	SAVELOAD_WORD(stateMode, offset, data, smcPrevPA);
	SAVELOAD_WORD(stateMode, offset, data, smcCounter);

	SAVELOAD_BYTE(stateMode, offset, data, fdsIO);
	SAVELOAD_WORD(stateMode, offset, data, fdsCounter);

	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum6 =6;
uint16_t mapperNum12 =12;
uint16_t mapperNum17 =17;
} // namespace

MapperInfo MapperInfo_006 ={
	&mapperNum6,
	_T("Front Fareast Magic Card 1M/2M"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	cpuCycle,
	ppuCycle,
	saveLoad,
	NULL,
	NULL
};
MapperInfo MapperInfo_012_1 ={
	&mapperNum12,
	_T("Front Fareast Magic Card 4M"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	cpuCycle,
	ppuCycle,
	saveLoad,
	NULL,
	NULL
};
MapperInfo MapperInfo_017 ={
	&mapperNum17,
	_T("Front Fareast Super Magic Card"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	cpuCycle,
	ppuCycle,
	saveLoad,
	NULL,
	NULL
};
