#include	"..\DLL\d_iNES.h"

namespace {
#define		prgMode1M    (mc1Mode >>5)
#define		prgMode2M   !(mc2Mode &0x01)
#define		protectPRG !!(mc1Mode &0x02)
#define		protectCHR !!(prgMode1M >=4)
#define		mirroring    (mc1Mode &0x11)
#define		mirrorS0      0x00
#define		mirrorS1      0x10
#define		mirrorV       0x01
#define		mirrorH       0x11

FCPUWrite	writeAPU;
FCPUWrite	writeCart;

uint8_t		mc1Mode;
uint8_t		mc2Mode;
uint8_t		latch;
uint8_t		chr8K;

uint8_t		prg8K[4];

uint8_t		fdsIO;
int16_t		fdsCounter;
int16_t		sgdCounter;

void	sync (void) {
	EMU->SetPRG_RAM8(0x6, 0);
	if (prgMode2M)
		for (int bank =0; bank <4; bank++) EMU->SetPRG_ROM8(0x8 +bank*2, prg8K[bank]);
	else
	switch(prgMode1M) {
		case 0:	EMU->SetPRG_ROM16(0x8, latch &7);
			EMU->SetPRG_ROM16(0xC, 7);
			break;
		case 1:	EMU->SetPRG_ROM16(0x8, latch >>2 &15);
			EMU->SetPRG_ROM16(0xC, 7);
			chr8K =latch &3;
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

	EMU->SetCHR_RAM8(0x0, chr8K);
	if (protectCHR) protectCHRRAM();
	
	switch(mirroring) {
		case mirrorS0: EMU->Mirror_S0(); break;
		case mirrorS1: EMU->Mirror_S1(); break;
		case mirrorV:  EMU->Mirror_V (); break;
		case mirrorH:  EMU->Mirror_H (); break;
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
		case 0x100:
			sgdCounter =(sgdCounter &~0x00FF) |val;
			EMU->SetIRQ(1);
			if (val ==0) sgdCounter =0;
			break;
		case 0x101:
			sgdCounter =(sgdCounter &~0xFF00) |(val <<8);
			EMU->SetIRQ(1);
			break;
		case 0x2FC: case 0x2FD: case 0x2FE: case 0x2FF:
			mc1Mode =val &0xF0 | addr &0x03;
			sync();
			break;
		case 0x3FC: case 0x3FD: case 0x3FE: case 0x3FF:
			mc2Mode =val &0xF0 | addr &0x03;
			chr8K =val &0x03;
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

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();	
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE ResetType) {
	if (ResetType ==RESET_HARD) {
		// (G026) Crazy Climber copies its bank 0 to bank 15 to have mode 3 (resembles iNES mapper #97) resemble the original iNES mapper #180.
		// This requires extending the PRG address space beyond the actual ROM size.
		if (ROM->INES2_SubMapper ==3 && ROM->PRGROMSize <262144) EMU->SetPRGROMSize(262144);
		
		// Game Doctors only have CHR-RAM, that can be write-protected. Many .NES ROM files supply CHR-ROM data instead, so copy it to CHR-RAM space
		if (ROM->CHRROMSize) {
			if (ROM->CHRRAMSize ==0) EMU->SetCHRRAMSize(ROM->CHRROMSize);
			for (unsigned int i =0; i <ROM->CHRROMSize; i++) ROM->CHRRAMData[i] =ROM->CHRROMData[i];
		}
		
		// Initial banking mode, mirroring and PRG memory protection
		mc1Mode =ROM->INES2_SubMapper <<5
		      |(ROM->INES_Flags &1? 0x01: 0x11)
		      | 0x02
		;
		// Disable 2M mode
		mc2Mode =0x03; 
		latch =0;
		chr8K =0;
		for (int bank =0; bank <4; bank++) prg8K[3 -bank] =0x1F -bank;

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
			for (unsigned int i =0; i <ROM->MiscROMSize; i++) ROM->PRGRAMData[(trainerAddr &0x1FFF) +i] =trainerData[i];
			if (trainerInit) {
				sync();
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
	sgdCounter =0;
	fdsIO =0;
	fdsCounter =0;
	
	writeAPU  =EMU->GetCPUWriteHandler(0x4);
	writeCart =EMU->GetCPUWriteHandler(0x8);
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
	if (sgdCounter <0 && !++sgdCounter) EMU->SetIRQ(0);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, mc1Mode);
	SAVELOAD_BYTE(stateMode, offset, data, mc2Mode);
	SAVELOAD_BYTE(stateMode, offset, data, latch);
	SAVELOAD_BYTE(stateMode, offset, data, chr8K);
	for (auto& c: prg8K) SAVELOAD_BYTE(stateMode, offset, data, c);

	SAVELOAD_WORD(stateMode, offset, data, sgdCounter);
	SAVELOAD_BYTE(stateMode, offset, data, fdsIO);
	SAVELOAD_WORD(stateMode, offset, data, fdsCounter);

	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =561;
} // namespace

MapperInfo MapperInfo_561 ={
	&mapperNum,
	_T("Bung Super Game Doctor"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	cpuCycle,
	NULL,
	saveLoad,
	NULL,
	NULL
};

