#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\Sound\s_waveFile.h"
#include	"..\Hardware\Sound\s_upd7756.h"

namespace {
uint8_t		prg[4];
uint8_t		chr[8];
uint16_t	latch;
uint16_t	counter;
uint8_t		mirroring;
uint8_t		irq;

uint8_t		speech;
bool		playing;
WaveFiles	waveFiles;

#define	wramEnabled    (prg[3] &0x01)
#define	wramProtected !(prg[3] &0x02)
#define	speechStart     speech &2
#define	speechStop      speech &1
#define message        (speech >>2)

void	sync (void) {
	if (wramEnabled) {
		EMU->SetPRG_RAM8(0x6, 0);
		if (wramProtected) for (int bank =0x6; bank<=0x7; bank++) EMU->SetPRG_Ptr4(bank, EMU->GetPRG_Ptr4(bank), FALSE);
	} else
		for (int bank =0x6; bank<=0x7; bank++) EMU->SetPRG_OB4(bank);
	for (int bank =0; bank <3; bank++) EMU->SetPRG_ROM8(0x8 | bank<<1, prg[bank]);
	EMU->SetPRG_ROM8(0xE, 0xFF);

	for (int bank =0; bank <8; bank++) EMU->SetCHR_ROM1(bank, chr[bank]);	
	switch(mirroring &3) {
		case 0:	EMU->Mirror_H();  break;
		case 1:	EMU->Mirror_V();  break;
		case 2:	EMU->Mirror_S0(); break;
		case 3:	EMU->Mirror_S1(); break;
	}
}

void	MAPINT	writePRG (int bank, int addr, int val) {
	int index =bank <<1 &2 | addr >>1 &1;
	if (addr &1)
		prg[index] =prg[index] &0x0F | val <<4;
	else
		prg[index] =prg[index] &0xF0 | val &0xF;
	sync();
}

void	MAPINT	writeCHR (int bank, int addr, int val) {
	int index =(bank -0xA) <<1 | addr >>1 &1;
	if (addr &1)
		chr[index] =chr[index] &0x0F | val <<4;
	else
		chr[index] =chr[index] &0xF0 | val &0xF;
	sync();
}

void	MAPINT	writeLatch (int bank, int addr, int val) {
	addr &=3;
	val &=0xF;
	int bits =addr <<2;
	int mask =0xF <<bits;
	latch =latch &~mask | val <<bits;
}

void	MAPINT	writeMisc (int bank, int addr, int val) {
	switch (addr &3) {
		case 0:	counter =latch;
			EMU->SetIRQ(1);
			break;
		case 1:	irq =val;
			EMU->SetIRQ(1);
			break;
		case 2:	mirroring =val;
			sync();
			break;
		case 3:	speech =val;
			if (speechStart) {
				waveFiles[message].restart();
				playing =true;
			}
			if (speechStop) playing =false;
			break;
	}
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		prg[0] =0x00;
		prg[1] =0x01;
		prg[2] =0xFE;
		prg[3] =0x00;
		for (auto& c: chr) c =0;
		latch =0;
		counter =0;
		mirroring =0;
		irq =0;		
		
		if (ROM->INES2_SubMapper ==0 && ROM->MiscROMSize >=32768) {
			UPD7756::loadSampleROM (waveFiles, ROM->MiscROMData, ROM->MiscROMSize);
		} else {
			TCHAR *dirName;
			switch(ROM->PRGROMCRC32) {
				case 0x142F7F3F: dirName =_T("JF-23"); break;
				case 0x3C361B36: dirName =_T("JF-24"); break;
				case 0xC2222BB1: dirName =_T("JF-29"); break;
				case 0x035CC54B: dirName =_T("JF-33"); break;
				default:         dirName =_T(""); break;
			}
			loadWaveFiles(waveFiles, dirName, 64);
		}
	}
	sync();
	EMU->SetIRQ(1);

	for (int bank =0x8; bank <=0x9; bank++) EMU->SetCPUWriteHandler(bank, writePRG);
	for (int bank =0xA; bank <=0xD; bank++) EMU->SetCPUWriteHandler(bank, writeCHR);
	EMU->SetCPUWriteHandler(0xE, writeLatch);
	EMU->SetCPUWriteHandler(0xF, writeMisc);
}

void	MAPINT	unload (void) {
	waveFiles.clear();
}

void	MAPINT	cpuCycle (void) {
	if (irq &1 && (counter-- &(irq &8? 0xF: 0xFFFF) &(irq &4? 0xFF: 0xFFFF) &(irq &2? 0xFFF: 0xFFFF)) ==0) EMU->SetIRQ(0);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	for (auto& c: prg) SAVELOAD_BYTE(stateMode, offset, data, c);
	for (auto& c: chr) SAVELOAD_BYTE(stateMode, offset, data, c);
	SAVELOAD_WORD(stateMode, offset, data, latch);
	SAVELOAD_WORD(stateMode, offset, data, counter);
	SAVELOAD_BYTE(stateMode, offset, data, mirroring);
	SAVELOAD_BYTE(stateMode, offset, data, irq);
	SAVELOAD_BYTE(stateMode, offset, data, speech);
	SAVELOAD_BOOL(stateMode, offset, data, playing);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

int	MAPINT	mapperSnd (int cycles) {
	int result =0;
	if (playing) while (cycles--) {
		result += waveFiles[message].getNextSample();
		playing =!waveFiles[message].isFinished();
	}
	return result;
}

uint16_t mapperNum =18;
} // namespace

MapperInfo MapperInfo_018 ={
	&mapperNum,
	_T("Jaleco SS8806"),
	COMPAT_FULL,
	load,
	reset,
	unload,
	cpuCycle,
	NULL,
	saveLoad,
	mapperSnd,
	NULL
};
