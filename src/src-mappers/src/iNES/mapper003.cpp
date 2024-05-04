#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_Latch.h"
#include	"..\Hardware\Sound\s_waveFile.h"

#define	speechStart ~speech &0x40
#define message     (speech &0x07)
namespace {
uint8_t		speech;
bool		playing;
std::vector<WaveFile> waveFiles;

void	sync (void) {
	EMU->SetPRG_ROM32(0x8, 0);
	EMU->SetPRG_RAM8(0x6, 0); // For 早打ち Super 囲碁. Will be open bus if no RAM specified in NES 2.0 header
	EMU->SetCHR_ROM8(0, Latch::data);
	iNES_SetMirroring();
}

void	MAPINT	writeSpeech (int bank, int addr, int val) {
	speech =val;	
	if (speechStart && !playing) {	
		waveFiles[speech].restart();	
		playing =true;	
	}	
}

BOOL	MAPINT	load (void) {
	MapperInfo_003.Description =ROM->INES_CHRSize ==128/8? _T("Bandai CNROM"): ROM->INES_CHRSize >32/8? _T("CNROM (nonstandard)"): ROM->INES_CHRSize==32/8? _T("Nintendo CNROM-256"): _T("Nintendo CNROM-128");
	Latch::load(sync, ROM->INES2_SubMapper ==2);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) playing =false;
	Latch::reset(resetType);
	
	if (ROM->PRGROMCRC32 ==0xF8DA2506) {
		loadWaveFiles(waveFiles, _T("FT-03"), 8);
		for (int bank =0x6; bank<=0x7; bank++) EMU->SetCPUWriteHandler(bank, writeSpeech);
	}
}

void	MAPINT	unload (void) {
	waveFiles.clear();
}

int	MAPINT	mapperSnd (int cycles) {
	int result =0;
	if (playing) while (cycles--) {
		result += waveFiles[message].getNextSample();
		playing =!waveFiles[message].isFinished();
	}
	return result;
}

uint16_t mapperNum =3;
} // namespace

MapperInfo MapperInfo_003 ={
	&mapperNum,
	_T("Nintendo CNROM"),
	COMPAT_FULL,
	load,
	reset,
	unload,
	NULL,
	NULL,
	Latch::saveLoad_D,
	mapperSnd,
	NULL
};
