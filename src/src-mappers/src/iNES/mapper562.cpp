#include	"..\DLL\d_iNES.h"
#include	"tgd4800.h"

namespace {
#define		prgMode1M    (mc1Mode >>5)
#define		prgMode2M   !(mc2Mode &0x01)
#define		prgMode4M  !!(tgdMode &0x80)
#define		chrMode1K  !!(tgdMode &0x40)
#define		protectPRG !!(mc1Mode &0x02)
#define		protectCHR !!(prgMode1M >=4 || lockCHR)
#define		mirroring    (mc1Mode &0x11)
#define		mirrorS0      0x00
#define		mirrorS1      0x10
#define		mirrorV       0x01
#define		mirrorH       0x11

FCPURead	readAPU;
FCPUWrite	writeAPU;
FCPUWrite	writeCart;
FPPUWrite	writeCHR;

uint8_t		mc1Mode;
uint8_t		mc2Mode;
uint8_t		tgdMode;
uint8_t		latch;
uint8_t		chr8K;
bool		lockCHR;

uint8_t		prg8K[4];
uint8_t		chr1K[8];
uint8_t		lastCHRBank;

uint8_t		fdsIO;
int16_t		fdsCounter;
uint16_t	tgdCounter;
uint16_t	tgdTarget;

void	MAPINT	interceptCHRWrite (int, int, int);

void	sync (void) {
	EMU->SetPRG_RAM8(0x6, 0);
	if (prgMode4M)
		for (int bank =0; bank <4; bank++) EMU->SetPRG_ROM8(0x8 +bank*2, prg8K[bank]);
	else
	if (prgMode2M)
		for (int bank =0; bank <4; bank++) EMU->SetPRG_ROM8(0x8 +bank*2, prg8K[bank] &0x0F | (bank &2? 0: mc2Mode >>2 &0x10));
	else
	switch(prgMode1M) {
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
		case 6:	EMU->SetPRG_ROM8 (0x8, latch &0xF);
			EMU->SetPRG_ROM8 (0xA, latch >>4);
			EMU->SetPRG_ROM16(0xC, 7);
			break;
		case 7:	EMU->SetPRG_ROM8 (0x8, latch &0xF &~1);
			EMU->SetPRG_ROM8 (0xA, latch >>4  | 1);
			EMU->SetPRG_ROM16(0xC, 7);
			break;
	}
	if (!protectPRG) for (int bank =0x8; bank<=0xF; bank++) EMU->SetPRG_Ptr4(bank, (EMU->GetPRG_Ptr4(bank)), TRUE);

	if (chrMode1K)
		for (int bank =0; bank <8; bank++) EMU->SetCHR_RAM1(bank, chr1K[bank]);
	else
		EMU->SetCHR_RAM8(0x0, chr8K);
	if (protectCHR) protectCHRRAM();
	
	switch(mirroring) {
		case mirrorS0: EMU->Mirror_S0(); break;
		case mirrorS1: EMU->Mirror_S1(); break;
		case mirrorV:  EMU->Mirror_V (); break;
		case mirrorH:  EMU->Mirror_H (); break;
	}		
	
	for (int bank =0; bank <8; bank++) EMU->SetPPUWriteHandler(bank, prgMode1M >=5 && ~mirroring &1? interceptCHRWrite: writeCHR);
}

int	MAPINT	readReg (int bank, int addr) {
	switch(addr) {
		case 0x400: case 0x401: case 0x402: case 0x403: case 0x404: case 0x405: case 0x406: case 0x407:
			return chr1K[addr &7];
		case 0x408: case 0x409: case 0x40A: case 0x40B:
			return prg8K[addr &3] <<2 | latch &3;
		case 0x40C:
			return tgdCounter >>8;
		case 0x40D:
			return tgdCounter &0xFF;
		case 0x411:
			return tgdMode;
		case 0x415:
			return mc1Mode;
		case 0x420:
			return chr1K[lastCHRBank];
		default:
			return addr &0x800? tgd4800[addr &0x7FF]: readAPU(bank, addr);
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
			fdsIO =val;
			if (val &0x42) fdsCounter =0;
			break;
		case 0x2FC: case 0x2FD: case 0x2FE: case 0x2FF:
			mc1Mode =val &0xF0 | addr &0x03;
			if (prgMode1M >=4) lockCHR =false;
			sync();
			break;
		case 0x3FC: case 0x3FD: case 0x3FE: case 0x3FF:
			mc2Mode =val &0xF0 | addr &0x03;
			chr8K =val &0x03;
			sync();
			break;
		case 0x400: case 0x401: case 0x402: case 0x403: case 0x404: case 0x405: case 0x406: case 0x407:
			chr1K[addr &7] =val;
			sync();
			break;
		case 0x40C:
			EMU->SetIRQ(1);
			if (~val &0x80) tgdCounter =0x8000;
			tgdTarget =tgdTarget &0x00FF | val <<8;
			break;
		case 0x40D:
			EMU->SetIRQ(1);
			tgdTarget =tgdTarget &0xFF00 | val;
			break;
		case 0x411:
			tgdMode =val;
			sync();
			break;
	}
}

void	MAPINT	writeLatch (int bank, int addr, int val) {
	if (protectPRG) {
		latch =val;
		switch(prgMode1M) {
			case 1: case 4: case 5:
				chr8K =latch &3;
				break;
			case 3:
				chr8K =latch >>4 &3;
				break;
		}
		prg8K[bank >>1 &3] =val >>2;
		sync();
	} else
		writeCart(bank, addr, val);
}

void	MAPINT	interceptCHRWrite (int bank, int addr, int val) {
	/* Writing anything to CHR memory during modes 5-7 with one-screen mirroring locks (CIRAM page 1) or unlocks (CIRAM page 0) CHR memory in modes 0-3. Needed for (F4040) Karnov). */
	lockCHR =!!(mirroring &0x10);
	writeCHR(bank, addr, val);
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();	
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	writeCHR =EMU->GetPPUWriteHandler(0x0);
	
	if (resetType ==RESET_HARD) {
		// Game Doctors only have CHR-RAM, that can be write-protected. Many .NES ROM files supply CHR-ROM data instead, so copy it to CHR-RAM space
		if (ROM->CHRROMSize) {
			if (ROM->CHRRAMSize ==0) EMU->SetCHRRAMSize(ROM->CHRROMSize);
			for (unsigned int i =0; i <ROM->CHRROMSize; i++) ROM->CHRRAMData[i] =ROM->CHRROMData[i];
		}
		
		// Initial mode
		mc1Mode =ROM->INES2_SubMapper <<5                                         // Initial banking mode
		      |(ROM->INES_Flags &1? 0x01: 0x11)                                  // Initial mirroring (V or H)
		      | 0x04                                                             // Not 8 KiB mode
		      | 0x02                                                             // PRG protected
		;
		lockCHR =false;
		// Disable PRG 2M/4M and CHR 1K modes
		mc2Mode =0x03; 
		tgdMode =0x03; // WRAM bank 3 is initially active, since it maps to FDS RAM originally at address $6000
		
		latch =0;
		chr8K =0;
		for (int bank =0; bank <4; bank++) prg8K[3 -bank] =0x1F -bank;
		for (int bank =0; bank <8; bank++) chr1K[bank] =bank;

		// Initialize the trainer
		if (ROM->MiscROMData && ROM->MiscROMSize >=4) {
			uint16_t trainerAddr =0x7000;
			uint16_t trainerInit =0x7003;
			size_t   trainerSize =512;
			uint8_t* trainerData =ROM->MiscROMData;

			if (ROM->MiscROMSize !=512) {
				trainerAddr =ROM->MiscROMData[0] | ROM->MiscROMData[1] <<8;		
				trainerInit =ROM->MiscROMData[2] | ROM->MiscROMData[3] <<8;
				trainerSize =ROM->MiscROMSize -4;
				trainerData =ROM->MiscROMData +4;
			}
			sync();
			for (unsigned int i =0; i <ROM->MiscROMSize; i++) (EMU->GetCPUWriteHandler(trainerAddr >>12))(trainerAddr >>12, (trainerAddr &0x0FFF) +i, trainerData[i]);
			if (trainerInit) {
				// JSR trainerInit
				(EMU->GetCPUWriteHandler(0x0))(0x0, 0x700, 0x20);
				(EMU->GetCPUWriteHandler(0x0))(0x0, 0x701, trainerInit &0xFF);
				(EMU->GetCPUWriteHandler(0x0))(0x0, 0x702, trainerInit >>8);
				// JMP ($FFFC)
				(EMU->GetCPUWriteHandler(0x0))(0x0, 0x703, 0x6C);
				(EMU->GetCPUWriteHandler(0x0))(0x0, 0x704, 0xFC);
				(EMU->GetCPUWriteHandler(0x0))(0x0, 0x705, 0xFF);
				EMU->SetPC(0x700);
			}
		}
		(EMU->GetCPUWriteHandler(0x4))(0x4, 0x017, 0x40); // Disable the frame IRQ. The Game Doctor BIOS already does that, and some games depend on it.
	}
	sync();
	
	EMU->SetIRQ(1);
	fdsIO =0;
	fdsCounter =0;
	tgdCounter =0xFFFF;
	tgdTarget =0;
	
	readAPU   =EMU->GetCPUReadHandler(0x4);
	writeAPU  =EMU->GetCPUWriteHandler(0x4);
	writeCart =EMU->GetCPUWriteHandler(0x8);
	EMU->SetCPUReadHandler(0x4, readReg);
	EMU->SetCPUWriteHandler(0x4, writeReg);
	for (int bank =0x8; bank<=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeLatch);
}

void	MAPINT	cpuCycle (void) {
	// Simulate FDS Data IRQ, used by Magic Card 1M and 2M games for frame timing
	fdsCounter +=3;
	while (fdsCounter >=448 && fdsIO &0x80) {
		EMU->SetIRQ(0);
		fdsCounter -=448;
	}
	
	if (tgdTarget &0x8000) {
		if (tgdCounter ==tgdTarget && tgdCounter !=0xFFFF)
			EMU->SetIRQ(0);
		else
			tgdCounter++;
	}
}

void	MAPINT	ppuCycle (int addr, int scanline, int cycle, int isRendering) {
	if (addr <0x2000) lastCHRBank =addr >>10;
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, mc1Mode);
	SAVELOAD_BYTE(stateMode, offset, data, mc2Mode);
	SAVELOAD_BYTE(stateMode, offset, data, tgdMode);
	SAVELOAD_BYTE(stateMode, offset, data, latch);
	SAVELOAD_BYTE(stateMode, offset, data, chr8K);
	SAVELOAD_BOOL(stateMode, offset, data, lockCHR);
	for (auto& c: prg8K) SAVELOAD_BYTE(stateMode, offset, data, c);
	for (auto& c: chr1K) SAVELOAD_BYTE(stateMode, offset, data, c);

	SAVELOAD_BYTE(stateMode, offset, data, fdsIO);
	SAVELOAD_WORD(stateMode, offset, data, fdsCounter);

	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =562;
} // namespace

MapperInfo MapperInfo_562 ={
	&mapperNum,
	_T("Venus Turbo Game Doctor"),
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

