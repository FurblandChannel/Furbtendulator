#include "stdafx.h"
#include "resource.h"
#include "Nintendulator.h"
#include "MapperInterface.h"
#include "Controllers.h"
#include "Settings.h"
#include "GFX.h"
#include "plugThruDevice.hpp"
#include "plugThruDevice_StudyBox.hpp"

#define PAD_PRINTER 0
#define SEEK_PAGE_DELAY 3000000

namespace PlugThruDevice {
namespace StudyBox {

StudyBoxFile::Page::Page(uint32_t _leadIn, uint32_t _start, uint32_t _size):
	leadIn(_leadIn),
	start(_start) {
	data.resize(_size);
}

std::string StudyBoxFile::readFourCC() const {
	uint8_t data[4];
	fread(data, 1, 4, handle);
	std::string result;
	result.push_back(data[0]);
	result.push_back(data[1]);
	result.push_back(data[2]);
	result.push_back(data[3]);
	return result;
}

uint32_t StudyBoxFile::readLSB32() const {
	uint8_t data[4];
	fread(data, 1, 4, handle);
	return data[0] | data[1] <<8 | data[2] <<16 | data[3] <<24;
}

uint32_t StudyBoxFile::readLSB16() const {
	uint8_t data[2];
	fread(data, 1, 2, handle);
	return data[0] | data[1] <<8;
}

StudyBoxFile::StudyBoxFile(FILE* _handle):
	handle(_handle) {
	if (readFourCC() !="STBX" || readLSB32() !=4 || readLSB32() !=0x100) throw std::runtime_error("Not a StudyBox version 1.00 file");
	while(!feof(handle)) {
		std::string fourCC =readFourCC();
		if (feof(handle)) break;
		if (fourCC =="PAGE") {
			uint32_t size =readLSB32();
			if (size <8) throw std::runtime_error("Invalid page size");
			uint32_t leadIn =readLSB32();
			uint32_t start =readLSB32();
			Page page(leadIn, start, size -8);
			fread(&page.data[0], 1, page.data.size(), handle);
			if (pages.size() >0 && page.start <=pages.back().start) throw std::runtime_error("Pages not in ascending order");
			pages.push_back(page);
		} else
		if (fourCC =="AUDI") {
			uint32_t size =readLSB32();
			if (size <0x38) throw std::runtime_error("Invalid audio size");
			uint32_t type =readLSB32();
			if (type ==0) {
				if (readFourCC() !="RIFF") throw std::runtime_error("Not in RIFF format");
				readLSB32(); // Ignore RIFF size
				if (readFourCC() !="WAVE" || readFourCC() !="fmt " || readLSB32() !=16) throw std::runtime_error("Not a WAVE file");
				if (readLSB16() !=1) throw std::runtime_error("Not in PCM format");
				if (readLSB16() !=1) throw std::runtime_error("More than one audio channel");
				rate =readLSB32();
				readLSB32(); // Ignore bytes per second
				readLSB16(); // Ignore block alignment
				if (readLSB16() !=16) throw std::runtime_error("Word length not 16 bit");
				if (readFourCC() !="data") throw std::runtime_error("No data chunk or not at expected position");
				audio.resize(readLSB32() >>1);
				fread(&audio[0], 2, audio.size(), handle);
			} else
				throw std::runtime_error("Unsupported audio format");
		} else
			throw std::runtime_error("Unknown tag " +fourCC);
	}
	if (!pages.size()) throw std::runtime_error("No page data");
	if (!audio.size()) throw std::runtime_error("No audio data");
}

StudyBoxFile*	cassette =NULL;

#define audioEnabled  !(reg[2] &0x04)

uint8_t		reg[4];
uint8_t		command;
uint8_t		commandCounter;
uint8_t		currentPage;
int32_t		pageIndex;
int32_t		pagePosition;
int16_t		seekPage;
uint32_t	seekPageDelay;
uint16_t	byteReadDelay;
uint16_t	processBitDelay;
uint32_t	inDataDelay;
bool		inDataRegion;
bool		ready;
bool		motorDisabled;
bool		pageFound;
bool		enableDecoder;
bool		enableIRQ;
uint32_t	wavePosition;
uint32_t	waveCount;

#if	PAD_PRINTER
uint8_t		reg4016;
FILE		*printer;
uint8_t		printerShift;
uint8_t		printerCount;
#endif

void	sync (void) {
	EI.SetPRG_RAM4(0x5, reg[0] &0x07 |0x08);
	EI.SetPRG_RAM8(0x6, reg[0] >>6);

	EI.SetPRG_ROM16(0x8, reg[1] &0xF);
	EI.SetPRG_ROM16(0xC, 0);

	EI.SetCHR_RAM8(0x0, 0);

	EI.Mirror_4();
}

void	readLeadIn (void) {
	inDataDelay =(uint64_t) (cassette->pages[pageIndex].start -cassette->pages[pageIndex].leadIn) *1789773 /cassette->rate;
	pagePosition =-1;
	byteReadDelay =0;
	motorDisabled =false;
	pageFound =true;
}

int	MAPINT	readRegDebug (int bank, int addr) {
	if (addr >=0x400) // First bank on the second chip is mapped to 4000-4FFF, but the first 1 KiB is not accessible
		return RI.PRGRAMData[addr |0x8000];
	else
	if (addr &0x200) {
		int index =addr &3;
		switch(index) {
			case 0: if (pagePosition >=0 && pagePosition <cassette->pages[pageIndex].data.size())
					return cassette->pages[pageIndex].data[pagePosition];
				else
					return 0xAA;
			case 1: return (inDataRegion?  0x20: 0x00) |
				       (pageFound?     0x40: 0x00) |
			               (enableDecoder? 0x80: 0x00)
				;
			case 2:	return ready? 0x40: 0x00;
			default:return 0x00;
		}
	} else
		return passCPUReadDebug(bank, addr);
}

int	MAPINT	readReg (int bank, int addr) {
	if (addr >=0x400) // First bank on the second chip is mapped to 4000-4FFF, but the first 1 KiB is not accessible
		return RI.PRGRAMData[addr |0x8000];
	else
	if (addr &0x200) {
		int index =addr &3;
		int result;
		switch(index) {
			case 0:	pdSetIRQ(1);
				if (pagePosition >=0 && pagePosition <cassette->pages[pageIndex].data.size())
					return cassette->pages[pageIndex].data[pagePosition];
				else
					return 0xAA;
			case 1:	result =(inDataRegion?  0x20: 0x00) |
				        (pageFound?     0x40: 0x00) |
			                (enableDecoder? 0x80: 0x00) |
					0x10;
				;
				pageFound =false;
				return result;
			case 2:	return ready? 0x40: 0x00;
			default:return 0x00;
		}
	} else
	#if	PAD_PRINTER
	if (addr ==0x16 && GetAsyncKeyState(VK_LBUTTON))
		return passCPURead(bank, addr) |0x02;
	else
	if (addr ==0x17) {
		POINT mousepos;
		GFX::GetCursorPos(&mousepos);		
		if ((reg4016 &0xFE) ==2 && mousepos.x >=0 && mousepos.x <GFX::SIZEX && mousepos.y >=0 && mousepos.y <GFX::SIZEY)
			return (reg4016 &1? mousepos.y /15: mousepos.x /16) <<1;
		else
			return passCPURead(bank, addr);
	} else
	#endif
		return passCPURead(bank, addr);
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	passCPUWrite(bank, addr, val);
	#if	PAD_PRINTER
	if (addr ==0x016 && printer && !GetAsyncKeyState(VK_LBUTTON)) { // Only register printer writes if not reading the touch pad
		if (reg4016 &2 && ~val &2) printerCount =0;
		if (reg4016 &1 && ~val &1) printerShift <<=1;
		if (~val &1) printerShift |=val >>2 &1;
		if (~val &2 && ~reg4016 &1 && val &1 && ++printerCount ==8) {
			printerCount =0;
			fwrite(&printerShift, 1, 1, printer);
		}
		reg4016 =val;
	} else
	#endif
	if (addr >=0x400) // First bank on the second chip is mapped to 4000-4FFF, but the first 1 KiB is not accessible
		RI.PRGRAMData[addr |0x8000] =val;
	else
	if (addr &0x200) {
		int index =addr &3;
		if (index ==2) {
			if (reg[index] &0x10 && ~val &0x10) {
				command =command <<1 | val >>7;
				if (++commandCounter ==8) {
					commandCounter =0;
					if (command ==0x00) { // Move back to the start of this page
						seekPage =currentPage;
						currentPage--;
						seekPageDelay =SEEK_PAGE_DELAY;
						motorDisabled =false;
					} else
					if (command <=0x3F) { // Move forward X pages
						seekPage =currentPage +command;
						seekPageDelay =SEEK_PAGE_DELAY;
						motorDisabled =false;
					} else
					if (command <=0x7F) { // Move back X pages
						seekPage =currentPage -(command &0x3F);
						seekPageDelay =SEEK_PAGE_DELAY;
						motorDisabled =false;
					} else
					if (command ==0x86) { // Re-enable motor and continue to the next page
						if (pageIndex <cassette->pages.size() -1)
							pageIndex++;
						else	//Pretend we go back to the start of the tape (inaccurate)
							pageIndex =0;
						readLeadIn();
					}
				}
			}
			if (val &0x10) {
				ready =false;
				processBitDelay =100;
			}
			if (reg[index] &0x20 && ~val &0x20) {
				command =0;
				commandCounter =0;
				ready =true;
			}
			enableDecoder =!!(val &0x01);
			enableIRQ     =!!(val &0x02);
			pdSetIRQ(1);
		}
		reg[index] =val;
		if (index <2) sync();
	}	
}

BOOL	MAPINT	load (void) {
	if (!cassette) return FALSE;
	Description =_T("福武書店 StudyBox SBX-01");

	EI.DbgOut(L"%d page blocks", cassette->pages.size());
	int maxPage =0;
	for (auto& page: cassette->pages) if (page.data[5] >maxPage) maxPage =page.data[5];
	EI.DbgOut(L"Maximum page number: %d", maxPage +1);
	
	#if	PAD_PRINTER
	printer =fopen("PRINTER.TXT", "wb");
	#endif
	
	if (adMI->Load) adMI->Load();
	if (adMI->Unload) MI->Unload =adMI->Unload;	
	if (adMI->PPUCycle) MI->PPUCycle =adMI->PPUCycle;
	if (adMI->GenSound) MI->GenSound =adMI->GenSound;
	if (adMI->Config) MI->Config =adMI->Config;
	MI->Description =adMI->Description;
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	for (auto& c: reg) c =0;
	command =0;
	commandCounter =0;
	currentPage =0;
	pageIndex =0;
	pagePosition =-1;
	seekPage =0;
	seekPageDelay =0;
	byteReadDelay =0;
	processBitDelay =0;
	inDataDelay =0;
	inDataRegion =false;
	ready =false;
	motorDisabled =false;
	pageFound =false;
	enableDecoder =false;
	enableIRQ =false;
	wavePosition =0;
	waveCount =0;
	pdSetIRQ(1);
	sync();
	
	SetCPUReadHandler(0x4, readReg);
	SetCPUReadHandlerDebug(0x4, readRegDebug);
	SetCPUWriteHandler(0x4, writeReg);
}

void	MAPINT	unload (void) {
	if (cassette) {
		delete cassette;
		cassette =NULL;
	}
	#if	PAD_PRINTER
	if (printer) {
		fclose(printer);
		printer =NULL;
	}
	#endif
}

void	MAPINT	cpuCycle (void) {
	if (processBitDelay && !--processBitDelay) ready =true;
	if (!motorDisabled) {
		if (seekPage !=currentPage) {
			if (!--seekPageDelay) {
				seekPageDelay =SEEK_PAGE_DELAY;
				if (seekPage >currentPage)
					currentPage++;
				else
					currentPage--;
				pageIndex =0;
				for (int i =0; i <cassette->pages.size(); i++) {
					if (cassette->pages[i].data[5] ==currentPage -1) {
						pageIndex =i;
						break;
					}
				}
				readLeadIn();
			}
		} else
		if (inDataDelay >0) {
			inDataRegion =true;
			if (!--inDataDelay) {
				byteReadDelay =7820;
				wavePosition =cassette->pages[pageIndex].start;
				pdSetIRQ(0);
			}
		} else
		if (byteReadDelay && !--byteReadDelay) {
			byteReadDelay =3355;
			pagePosition++;
			if (pagePosition >=cassette->pages[pageIndex].data.size()) {
				pageFound =false;
				inDataRegion =false;
				motorDisabled =true;
			}
			if (enableIRQ) pdSetIRQ(0);
		}
	}
	// Need to make sure that mapperSnd is called even when fast-forwarding to make sure the wavePosition will be correct afterwards
	if (Controllers::capsLock) mapperSnd(1);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	if (adMI->SaveLoad) offset =adMI->SaveLoad(stateMode, offset, data);
	for (auto& c: reg) SAVELOAD_BYTE(stateMode, offset, data, c);
	SAVELOAD_BYTE(stateMode, offset, data, command);
	SAVELOAD_BYTE(stateMode, offset, data, commandCounter);
	SAVELOAD_BYTE(stateMode, offset, data, currentPage);
	SAVELOAD_LONG(stateMode, offset, data, pageIndex);
	SAVELOAD_LONG(stateMode, offset, data, pagePosition);
	SAVELOAD_WORD(stateMode, offset, data, seekPage);
	SAVELOAD_LONG(stateMode, offset, data, seekPageDelay);
	SAVELOAD_WORD(stateMode, offset, data, byteReadDelay);
	SAVELOAD_WORD(stateMode, offset, data, processBitDelay);
	SAVELOAD_LONG(stateMode, offset, data, inDataDelay);
	SAVELOAD_BOOL(stateMode, offset, data, inDataRegion);
	SAVELOAD_BOOL(stateMode, offset, data, ready);
	SAVELOAD_BOOL(stateMode, offset, data, motorDisabled);
	SAVELOAD_BOOL(stateMode, offset, data, pageFound);
	SAVELOAD_BOOL(stateMode, offset, data, enableDecoder);
	SAVELOAD_BOOL(stateMode, offset, data, enableIRQ);
	SAVELOAD_LONG(stateMode, offset, data, wavePosition);
	SAVELOAD_LONG(stateMode, offset, data, waveCount);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

int	MAPINT	mapperSnd (int cycles) {
	int result =0;
	if (!motorDisabled) while (cycles--) {
		if (wavePosition >=cassette->audio.size()) break;
		result +=cassette->audio[wavePosition];
		waveCount +=cassette->rate;
		if (waveCount >=1789773) {
			wavePosition++;
			waveCount -=1789773;
		}
	}
	return audioEnabled? result: 0;
}

uint16_t deviceNum =ID_PLUG_STUDYBOX;
} // namespace StudyBox

MapperInfo plugThruDevice_StudyBox ={
	&StudyBox::deviceNum,
	_T("福武書店 StudyBox SBX-01"),
	COMPAT_FULL,
	StudyBox::load,
	StudyBox::reset,
	StudyBox::unload,
	StudyBox::cpuCycle,
	NULL,
	StudyBox::saveLoad,
	StudyBox::mapperSnd,
	NULL
};
} // namespace PlugThruDevice

/*
Touch Panel:
if ([$4016] &0x02) {	// Touch Panel present? Or button pressed?
	[$4016] =$07; 5xNOP, Wait ~$DD
	[$4016] =$05; 5xNOP
	[$4016] =$07; 5xNOP
	[$4016] =$03; 5xNOP, Wait ~$DD
	[$20] =[$4017] >>1 &0xF
	[$4016] =$01; 5xNOP
	[$4016] =$03; 5xNOP
	[$4016] =$02; 5xNOP, Wait ~$DD
	[$20] |=[$4017] <<3 &0xF0
	[$4016] =$00; 5xNOP
	[$4016] =$02; 5xNOP
}
Printer:
981C:
FEB1:
	4016=0
	WAIT
	4016=1
	ROL	A
	PHA
	ROL	A
	ROL	A
	ROL	A
	AND	#4
	STA	4016
	WAIT
	ORA	1
	STA	4016
	for every bit
	4016=7
*/
