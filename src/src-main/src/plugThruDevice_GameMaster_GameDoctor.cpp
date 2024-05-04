#include "stdafx.h"

#include "resource.h"
#include "Nintendulator.h"
#include "MapperInterface.h"
#include "CPU.h"
#include "PPU.h"
#include "plugThruDevice.hpp"
#include "plugThruDevice_GameMaster.hpp"

#include "Settings.h"
#include "NES.h"
#include "FDS.h"
#include <algorithm>

namespace PlugThruDevice {
namespace GameMaster {
#define		GDSTATE_WAIT 0
#define		GDSTATE_LOAD 1
#define		GDSTATE_PLAY 2
uint8_t		state;
uint8_t		prg;
uint8_t		chr;
uint8_t		reg4100;
uint8_t		reg4101;

bool		fusemapWriteGate;
uint8_t		fusemapData[4096];
int		fusemapPosition;

// Fastload callbacks
void	cbPrebootDone (void) {
	NES::eject28();
	NES::next28();
	NES::insert28();
	FDS::diskPosition =0;
	RI.changedDisk28 =false;
	CPU::CPU[0]->PC =0xDF98;
	CPU::callbacks.erase(std::remove_if(CPU::callbacks.begin(), CPU::callbacks.end(), [](CPU::Callback& cb){ return cb.pc ==0xDF73; }), CPU::callbacks.end());
}
void	cbSkipGDHeader (void) {
	FDS::putCPUByte(0x2001, 0x00);
	FDS::putCPUByte(0x99, FDS::getCPUByte(0x87));
	FDS::putCPUByte(0x96, 0x00);
	FDS::putCPUByte(0x9A, 0x00);
	FDS::putCPUByte(0x06, FDS::getCPUByte(0x06) -1);
	CPU::CPU[0]->PC =v20? 0xF442: 0xF417;
}
void	cbCheckCRC (void) {
	CPU::CPU[0]->IN_RTS();
}

void	cbReadFileHeader (void) {
	std::vector<uint8_t> header =FDS::diskReadBlock(3, 15);
	if (FDS::lastErrorCode ==0) for (int i =10; i <15; i++) FDS::putCPUByte(0x88 +i -10, header[i]);
	CPU::CPU[0]->IN_RTS();
}
void	cbReadFileCPU (void) {
	uint16_t size =v20? FDS::getCPUWord(0x00AC): 0x8000;
	std::vector<uint8_t> data =FDS::diskReadBlock(4, size);
	if (FDS::lastErrorCode ==0) {	
		int i =0;
		int addr =FDS::getCPUWord(0x0088);
		while (size) {
			FDS::putCPUByte(addr++, data[i++]);
			if (addr ==0xE000) addr =0x6000;
			size--;
		}
		if (v20) {
			FDS::putCPUWord(0x0088, addr);
			FDS::putCPUWord(0x00AC, size);
		}
	}
	if (v20) {
		if (FDS::getCPUWord(0x0088) ==0x8000)
			CPU::CPU[0]->PC =0xF5C6;
		else
			CPU::CPU[0]->IN_RTS();
	} else
		CPU::CPU[0]->PC =0xF55E;
}
void	MAPINT	ppuWrite_sgd (int bank, int addr, int val);
void	cbReadFilePPU (void) {
	int sizes[8] ={ 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x4000, 0x2000 };
	uint16_t size =v20? FDS::getCPUWord(0x00AC): sizes[FDS::getCPUByte(0x81) >>5];
	std::vector<uint8_t> data =FDS::diskReadBlock(4, size);
	if (FDS::lastErrorCode ==0) {	
		FDS::putCPUByte(0x2001, 0x00);
		uint8_t reg413F =v20? FDS::getCPUByte(0x0113): (FDS::getCPUByte(0x90) +1) &3;
		
		int addr =0;
		for (int i =0; i <size; i++) {
			addr =i &0x1FFF;
			if (addr ==0) FDS::putCPUByte(0x413F, reg413F++);
			ppuWrite_sgd(addr >>10, addr &0x3FF, data[i]);
			if (!v20) reg413F &=3;
		}
		if (v20) {
			FDS::putCPUByte(0x0113, reg413F);
			FDS::putCPUWord(0x0088, addr);
			FDS::putCPUWord(0x00AC, size);
		} else
			FDS::putCPUByte(0x90, FDS::getCPUByte(0x90) &~3 | reg413F &3);
	}
	CPU::CPU[0]->PC =v20? 0xF665: 0xF55E;
}
void	cbReadFFETrainer (void) {
	std::vector<uint8_t> data =FDS::diskReadBlock(5, 512);
	if (FDS::lastErrorCode ==0) {
		for (int i =0; i <512; i++) FDS::putCPUByte(0xF000 +i, data[i]);
	}
	CPU::CPU[0]->PC =v20? 0xF4C8: 0xF4B2;
}
void	cbNextSide (void) {
	NES::eject28();
	NES::next28();
	NES::insert28();
	FDS::diskPosition =0;
	RI.changedDisk28 =false;
	
	std::vector<uint8_t> header =FDS::diskReadBlock(1, 55);
	if (FDS::lastErrorCode ==0) {
		std::vector<uint8_t> temp =FDS::diskReadBlock(2, 1);
		if (FDS::lastErrorCode ==0) FDS::putCPUByte(0x0006, temp[0]);
	}
	CPU::CPU[0]->PC =v20? 0xF819: 0xF640;
}
void	cbSkipPrompt (void) {
	CPU::CPU[0]->PC =v20? 0xF8A6: 0xF726;
}
void	initCallbacks (void) {
	if (Settings::FastLoad && RI.ROMType ==ROM_FDS) {
		CPU::CPU[0]->PC =v20? 0xF400: 0xF6F0;
		CPU::CPU[0]->A =FDS::getCPUByte(0x0080);
		CPU::callbacks.clear();
		if (v20) {
			CPU::callbacks.push_back({0, 0xF425, cbSkipGDHeader});
			CPU::callbacks.push_back({0, 0xE050, cbReadFileHeader});
			CPU::callbacks.push_back({0, 0xE1CD, cbCheckCRC});
			CPU::callbacks.push_back({0, 0xE658, cbNextSide});
			CPU::callbacks.push_back({0, 0xF4AC, cbReadFFETrainer});
			CPU::callbacks.push_back({0, 0xF59E, cbReadFileCPU});
			CPU::callbacks.push_back({0, 0xF619, cbReadFilePPU});
			CPU::callbacks.push_back({0, 0xF892, cbSkipPrompt});
		} else {
			CPU::callbacks.push_back({0, 0xF404, cbSkipGDHeader});
			CPU::callbacks.push_back({0, 0xE050, cbReadFileHeader});
			CPU::callbacks.push_back({0, 0xE1CD, cbCheckCRC});
			CPU::callbacks.push_back({0, 0xE658, cbNextSide});
			CPU::callbacks.push_back({0, 0xF496, cbReadFFETrainer});
			CPU::callbacks.push_back({0, 0xF4F6, cbReadFileCPU});
			CPU::callbacks.push_back({0, 0xF517, cbReadFilePPU});
			CPU::callbacks.push_back({0, 0xF71B, cbSkipPrompt});
		}
	}
}

// Handlers
int	MAPINT	cpuRead_sgd (int bank, int addr) {
	if (state !=GDSTATE_WAIT && bank ==0x4) {
		switch (addr) {
			case 0x100:
				return reg4100;
			case 0x101:
				return reg4101;
			default:
				return passCPURead(bank, addr);
		}
	}
	if (state ==GDSTATE_LOAD) {
		if (bank >=0x6 && bank <=0xD)
			return prgram.k32[prg][bank &7][addr];
		else
		if (bank >=0xE)
			return bios.k8[1][bank &1][addr];
		else
			return passCPURead(bank, addr);
	} else
	if (state ==GDSTATE_PLAY)
		return cpuRead_mapper(bank, addr);
	else
		return passCPURead(bank, addr);
}

int	MAPINT	cpuReadDebug_sgd (int bank, int addr) {
	if (state !=GDSTATE_WAIT && bank ==0x4) {
		switch (addr) {
			case 0x100:
				return reg4100;
			case 0x101:
				return reg4101;
			default:
				return passCPURead(bank, addr);
		}
	}
	if (state ==GDSTATE_LOAD) {
		if (bank >=0x6 && bank <=0xD)
			return prgram.k32[prg][bank &7][addr];
		else
		if (bank >=0xE)
			return bios.k8[1][bank &1][addr];
		else
			return passCPUReadDebug(bank, addr);
	} else
	if (state ==GDSTATE_PLAY)
		return cpuReadDebug_mapper(bank, addr);
	else
		return passCPUReadDebug(bank, addr);
}

void	MAPINT	cpuWrite_sgd (int bank, int addr, int val) {
	if (bank ==0x4) switch(addr) {
		case 0x100:
			if (fusemapWriteGate) {
				fusemapData[fusemapPosition++] =val;
				if (fusemapPosition ==sizeof(fusemapData)) {
					fusemapPosition =0;
					parseFusemap();
					if (Settings::FastLoad) {
						CPU::callbacks.erase(std::remove_if(CPU::callbacks.begin(), CPU::callbacks.end(), [](CPU::Callback& cb){ return cb.pc ==0xDF73; }), CPU::callbacks.end());
						CPU::callbacks.push_back({0, 0xDF73, cbPrebootDone});
					}
				}				
			} else
			if (state ==GDSTATE_PLAY)
				cpuWrite_mapper(bank, addr, val);
			else
				passCPUWrite(bank &7, addr, val);
			break;
		case 0x13F:
			if (state ==GDSTATE_PLAY) state =GDSTATE_LOAD;
			fusemapWriteGate =!!(val &0x80);
			if (fusemapWriteGate)
				fusemapPosition =0;
			else
				chr =val &0x7F;
			break;
		case 0x2FC: case 0x2FD: case 0x2FE: case 0x2FF:
			if (state ==GDSTATE_LOAD) {
				state =GDSTATE_PLAY;
				CPU::callbacks.clear();
			}
			reg4100 =(reg4100 & 9) | (val &~9);
			reg4100 =(reg4100 &~8) | (addr &1? 8: 0);
			cpuWrite_mapper(bank, addr, val);
			break;
		case 0x3FE: case 0x3FF:
			reg4100 =(reg4100 &~9) | (addr &9);
			reg4101 =val;
			if (state ==GDSTATE_WAIT) {
				state =GDSTATE_LOAD;
				reset_mapper(RESET_HARD);
				initCallbacks();
			}
			if (state ==GDSTATE_PLAY) cpuWrite_mapper(bank, addr, val);
			prg =((val &0x30) >>4) |
			     ((val &0x08) >>1) |
			     ((val &0x04) <<1);
			break;
		case 0xFFF:
			if (state ==GDSTATE_WAIT) {
				state =GDSTATE_LOAD;
				reset_mapper(RESET_HARD);
				initCallbacks();
			}
			break;
		default:
			if (state ==GDSTATE_PLAY)
				cpuWrite_mapper(bank, addr, val);
			else
				passCPUWrite(bank, addr, val);
			break;
	} else
	if (state ==GDSTATE_LOAD && bank >=0x6 && bank <=0xD)
		prgram.k32[prg][bank &7][addr] =val;
	else
	if (state ==GDSTATE_LOAD && bank >=0xE)		// Magic Card block 5 trainers: written by BIOS to $F000-F1FF
		prgram.k32[pageWrite][bank &7][addr] =val;
	else
	if (state ==GDSTATE_PLAY)
		cpuWrite_mapper(bank, addr, val);
	else
		passCPUWrite(bank, addr, val);
}

int	MAPINT	ppuRead_sgd (int bank, int addr) {
	switch (state) {
		case GDSTATE_LOAD:
			if (bank <0x8)
				return chrram.k8[chr][bank &7][addr];
			else
				return passPPURead(bank, addr);
			break;
		case GDSTATE_PLAY:
			return ppuRead_mapper(bank, addr);
		default:
			return passPPURead(bank, addr);
	}
}
int	MAPINT	ppuReadDebug_sgd (int bank, int addr) {
	switch (state) {
		case GDSTATE_LOAD:
			if (bank <0x8)
				return chrram.k8[chr][bank &7][addr];
			else
				return passPPUReadDebug(bank, addr);
			break;
		case GDSTATE_PLAY:
			return ppuReadDebug_mapper(bank, addr);
		default:
			return passPPUReadDebug(bank, addr);
	}
}
void	MAPINT	ppuWrite_sgd (int bank, int addr, int val) {
	switch (state) {
		case GDSTATE_WAIT:
			passPPUWrite(bank, addr, val);
			// Also shadow writes to FDS CHR-RAM
			if (bank <0x8) chrram.k8[chr][bank &7][addr] =val;
			break;
		case GDSTATE_LOAD:
			if (bank <0x8)
				chrram.k8[chr][bank &7][addr] =val;
			else
				passPPUWrite(bank, addr, val);
			break;
		case GDSTATE_PLAY:
			ppuWrite_mapper(bank, addr, val);
			break;
	}
}

void	reset_sgd (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		state =GDSTATE_WAIT;
		prg =0;
		chr =0;
		reg4100 =0;
		reg4101 =0;
		
		for (auto& c: fusemapData) c =0x00;
		fusemapWriteGate =false;
		fusemapPosition =0;
		loadFusemap(0xF7389C96); // Initialize handlers with SGD fusemap
		
		pdSetIRQ(1);		
	}
	if (state ==GDSTATE_PLAY) reset_mapper(resetType);
}

void	cpuCycle_sgd (void) {
	if (state ==GDSTATE_PLAY) cpuCycle_mapper();
}

void	ppuCycle_sgd (int addr, int scanline, int cycle, int isRendering) {
	if (state ==GDSTATE_PLAY) ppuCycle_mapper(addr, scanline, cycle, isRendering);
}

int	saveLoad_sgd (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, state);
	SAVELOAD_BYTE(stateMode, offset, data, prg);
	SAVELOAD_BYTE(stateMode, offset, data, chr);
	SAVELOAD_BYTE(stateMode, offset, data, reg4100);
	SAVELOAD_BYTE(stateMode, offset, data, reg4101);
	SAVELOAD_BOOL(stateMode, offset, data, fusemapWriteGate);
	for (auto& c: fusemapData) SAVELOAD_BYTE(stateMode, offset, data, c);
	SAVELOAD_LONG(stateMode, offset, data, fusemapPosition);
	
	if (stateMode ==STATE_LOAD) parseFusemap();
	offset =saveLoad_mapper(stateMode, offset, data);
	return offset;
}

} // namespace GameMaster
} // namespace PlugThruDevice