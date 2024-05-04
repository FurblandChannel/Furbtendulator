#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\Sound\s_hc55516.h"
#include	"..\Hardware\Sound\s_upd7756.h"
#include	"..\Hardware\Sound\s_waveFile.h"

#define	speechStart ~speech &0x10
#define	speechStop  ~speech &0x20
#define message     (speech &0xF)

namespace {
uint8_t		reg;

uint8_t		speech;
bool		playing;
WaveFiles	waveFiles;
int	MAPINT	upd_mapperSnd (int);

#define		um_period  (um_rate &0x80? 176: 88)
uint8_t		um_rate;
uint32_t	um_sample;
uint32_t	um_count;
hc55516_device	um_chip(0);
int16_t		um_output[256];
int	MAPINT	um_mapperSnd (int);

void	sync (void) {
	EMU->SetPRG_ROM32(0x8, reg >>4 &3);
	EMU->SetCHR_ROM8(0, reg >>4 &4 | reg &3);
	iNES_SetMirroring();
}

void	MAPINT	um_writeRate (int bank, int addr, int val) {
	um_rate =val;
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	reg =val;
	sync();
}

void	MAPINT	um_writeMessage (int bank, int addr, int val) {
	speech =val;
	if (speechStart) {
		um_chip.device_start();
		um_chip.device_reset();
		um_sample =0;
		playing =true;
	}
}

void	MAPINT	upd_writeMessage (int bank, int addr, int val) {
	speech =val;
	if (speechStart) {
		waveFiles[message].restart();
		playing =true;
	}
	if (speechStop) playing =false;
}

BOOL	MAPINT	load (void) {
	MapperInfo_086.Description =ROM->INES2_SubMapper ==1? _T("Bootleg JF-13"): _T("Jaleco JF-13");
	MapperInfo_086.GenSound    =ROM->INES2_SubMapper ==1 && ROM->MiscROMSize >=32768? um_mapperSnd: upd_mapperSnd;
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) { 
	if (resetType ==RESET_HARD) {
		reg =0;
		um_rate =0;
		um_sample =0;
		um_count =0;
		playing =false;
		if (ROM->INES2_SubMapper ==0 && ROM->MiscROMSize >=32768) {
			UPD7756::loadSampleROM (waveFiles, ROM->MiscROMData, ROM->MiscROMSize);
		} else
		if (ROM->INES2_SubMapper ==0 && ROM->MiscROMSize <32768) {
			TCHAR *dirName;
			switch(ROM->PRGROMCRC32) {
				case 0x430C05A1:
				case 0xDB53A88D:
				case 0xE30B210E:
				case 0xE374C3E7:
				case 0x93B9B15C:
				case 0xBFBF5B8B:
					dirName =_T("JF-13"); break;
				default:
					dirName =_T(""); break;
			}
			loadWaveFiles(waveFiles, dirName, 16);
		}
	}
	sync();

	EMU->SetCPUWriteHandler(0x6, writeReg);
	if (ROM->INES2_SubMapper ==1 && ROM->MiscROMSize >=32768) {
		EMU->SetCPUWriteHandler(0x5, um_writeRate);
		EMU->SetCPUWriteHandler(0x7, um_writeMessage);
	} else
	if (ROM->INES2_SubMapper ==0)
		EMU->SetCPUWriteHandler(0x7, upd_writeMessage);
}

void	MAPINT	unload (void) {
	waveFiles.clear();
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	SAVELOAD_BYTE(stateMode, offset, data, speech);
	SAVELOAD_BOOL(stateMode, offset, data, playing);
	if (ROM->INES2_SubMapper ==1 && ROM->MiscROMSize >=32768) {
		SAVELOAD_BYTE(stateMode, offset, data, um_rate);
		SAVELOAD_LONG(stateMode, offset, data, um_sample);
		SAVELOAD_LONG(stateMode, offset, data, um_count);
	}
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

int	MAPINT	upd_mapperSnd (int cycles) {
	int result =0;
	if (playing) while (cycles--) {
		result += waveFiles[message].getNextSample();
		playing =!waveFiles[message].isFinished();
	}
	return result;
}

int	MAPINT	um_mapperSnd (int cycles) {
	if (playing) while (cycles--) {
		if ((um_count %um_period) ==0) {
			um_chip.digit_w(ROM->MiscROMData[message <<11 &0x7800 | um_sample >>3] >>(~um_sample &7));		
			um_chip.clock_w(1);
			um_chip.clock_w(0);
			um_chip.sound_stream_update(um_output, um_period);
			if (++um_sample &0x4000) playing =false;
		}
	}
	return playing? um_output[um_count++ %um_period]: 0;
}

uint16_t mapperNum =86;
} // namespace

MapperInfo MapperInfo_086 ={
	&mapperNum,
	_T("Jaleco JF-13"),
	COMPAT_FULL,
	load,
	reset,
	unload,
	NULL,
	NULL,
	saveLoad,
	upd_mapperSnd,
	NULL
};