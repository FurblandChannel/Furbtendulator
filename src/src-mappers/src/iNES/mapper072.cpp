#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\Sound\s_waveFile.h"
#include	"..\Hardware\Sound\s_upd7756.h"

#define loadPRG     change &0x80
#define loadCHR     change &0x40
#define speechStart ~val &0x10
#define speechReset ~val &0x20

namespace {
uint8_t		prg;
uint8_t		chr;
uint8_t		prev;

uint8_t		message;
bool		playing;
WaveFiles	waveFiles;

void	sync (void) {
	if (ROM->INES_MapperNum ==92) {
		EMU->SetPRG_ROM16(0x8, 0x00);
		EMU->SetPRG_ROM16(0xC, prg);
	} else {
		EMU->SetPRG_ROM16(0x8, prg);
		EMU->SetPRG_ROM16(0xC, 0xFF);
	}
	EMU->SetCHR_ROM8(0, chr);
	iNES_SetMirroring();
}

void	MAPINT	writeLatch (int bank, int addr, int val) {
	// Emulate AND bus conflicts
	val &=EMU->GetCPUReadHandler(bank)(bank, addr);
	
	// Get bits changed from 0 to 1
	uint8_t change =(prev ^val) &val;
	prev =val;
	
	if (loadPRG) prg =val;
	if (loadCHR) chr =val;
	sync();
	
	if (speechStart) {
		message =addr &0x1F;
		waveFiles[message].restart();
		playing =true;
	}
	if (speechReset) playing =false;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		if (ROM->INES2_SubMapper ==0 && ROM->MiscROMSize >=32768) {
			UPD7756::loadSampleROM (waveFiles, ROM->MiscROMData, ROM->MiscROMSize);
		} else {
			TCHAR *dirName;
			switch(ROM->PRGROMCRC32) {
				case 0x598A7398: dirName =_T("JF-17"); break;
				case 0x9F50A100: dirName =_T("JF-19"); break;
				default:         dirName =_T(""); break;
			}
			loadWaveFiles(waveFiles, dirName, 32);
		}
		prg =0;
		chr =0;
		prev =0;
		playing =false;
	}
	sync();
	
	for (int bank =0x8; bank<=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeLatch);
}

void	MAPINT	unload (void) {
	waveFiles.clear();
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, prg);
	SAVELOAD_BYTE(stateMode, offset, data, chr);
	SAVELOAD_BYTE(stateMode, offset, data, prev);
	SAVELOAD_BYTE(stateMode, offset, data, message);
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

uint16_t mapperNum072 =72;
uint16_t mapperNum092 =92;
} // namespace

MapperInfo MapperInfo_072 ={
	&mapperNum072,
	_T("Jaleco JF-17"),
	COMPAT_FULL,
	NULL,
	reset,
	unload,
	NULL,
	NULL,
	saveLoad,
	mapperSnd,
	NULL
};
MapperInfo MapperInfo_092 ={
	&mapperNum092,
	_T("Jaleco JF-19"),
	COMPAT_FULL,
	NULL,
	reset,
	unload,
	NULL,
	NULL,
	saveLoad,
	mapperSnd,
	NULL
};