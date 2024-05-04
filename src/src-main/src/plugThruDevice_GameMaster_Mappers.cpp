#include "stdafx.h"

#include "resource.h"
#include "Nintendulator.h"
#include "MapperInterface.h"
#include "CPU.h"
#include "PPU.h"
#include "plugThruDevice.hpp"
#include "plugThruDevice_GameMaster.hpp"
#include "Hash.h"
#include "NES.h"

namespace PlugThruDevice {
namespace GameMaster {
uint8_t	fusemapLetter;

int	ntRead (int bank, int addr, int mirroring) {
	switch (mirroring &3) {
		default: // Not necessary; just to suppress compiler warning
		case 0:	return PPU::PPU[0]->VRAM[0][addr];
		case 1:	return PPU::PPU[0]->VRAM[1][addr];
		case 2:	return PPU::PPU[0]->VRAM[bank &1][addr];
		case 3:	return PPU::PPU[0]->VRAM[(bank >>1) &1][addr];
	}
}
void	ntWrite (int bank, int addr, int val, int mirroring) {
	if (bank <0xF || addr <0xF00) switch (mirroring &3) {
		case 0:	PPU::PPU[0]->VRAM[0][addr] =val; break;
		case 1:	PPU::PPU[0]->VRAM[1][addr] =val; break;
		case 2:	PPU::PPU[0]->VRAM[bank &1][addr] =val; break;
		case 3:	PPU::PPU[0]->VRAM[(bank >>1) &1][addr] =val; break;
	}
}

// ------------ Super Game Doctor 4M
namespace SGD4 {
uint8_t		modeGD;
bool		modeMC;
bool		protectPRG;
uint8_t		prgGD[4];
uint8_t		prgMC[4];
uint8_t		chrGD;
uint8_t		latchGD;
uint8_t		mirroring;
uint16_t	counter;

void	sync() {
	switch(modeGD) {
		case 0:	prgGD[0] =((latchGD &7) <<1) |0;
			prgGD[1] =((latchGD &7) <<1) |1;
			prgGD[2] =14;
			prgGD[3] =15;
			chrGD =0;
			break;
		case 1: prgGD[0] =((latchGD &~3) >>1) |0;
			prgGD[1] =((latchGD &~3) >>1) |1;
			prgGD[2] =14;
			prgGD[3] =15;
			chrGD =latchGD &3;
			break;
		case 2:	prgGD[0] =((latchGD &15) <<1) |0;
			prgGD[1] =((latchGD &15) <<1) |1;
			prgGD[2] =30;
			prgGD[3] =31;
			chrGD =0;
			break;
		case 3:	prgGD[0] =30;
			prgGD[1] =31;
			prgGD[2] =((latchGD &15) <<1) |0;
			prgGD[3] =((latchGD &15) <<1) |1;
			chrGD =latchGD >>4 &3;
			break;
		case 4:	prgGD[0] =((latchGD &0x30) >>2) |0;
			prgGD[1] =((latchGD &0x30) >>2) |1;
			prgGD[2] =((latchGD &0x30) >>2) |2;
			prgGD[3] =((latchGD &0x30) >>2) |3;
			chrGD =latchGD &3;
			break;
		case 5:	prgGD[0] =12;
			prgGD[1] =13;
			prgGD[2] =14;
			prgGD[3] =15;
			chrGD =latchGD &3;
			break;
		case 6:	prgGD[0] =latchGD &0xF;
			prgGD[1] =latchGD >>4;
			prgGD[2] =14;
			prgGD[3] =15;
			break;
		case 7:	prgGD[0] =latchGD &0xE;
			prgGD[1] =latchGD >>4 |1;
			prgGD[2] =14;
			prgGD[3] =15;
			chrGD =0;
			break;
	}
	if (modeMC) chrGD =latchGD &3;
}

int	MAPINT	cpuRead (int bank, int addr) {
	if (bank >=0x8) {
		int index =(bank &7) >>1;
		if (modeMC)
			return prgram.k8[prgMC[index]][bank &1][addr];
		else
			return prgram.k8[prgGD[index]][bank &1][addr];
	} else
	if (bank ==0x6 || bank ==0x7)
		return stateRAM.k4[bank &7][addr];
	else
		return passCPURead(bank, addr);
}
int	MAPINT	cpuReadDebug (int bank, int addr) {
	if (bank >=0x6)
		return cpuRead(bank, addr);
	else
		return passCPUReadDebug(bank, addr);
}
void	MAPINT	cpuWrite (int bank, int addr, int val) {
	if (bank ==0x4) switch(addr) {
		case 0x100:	
			counter =(counter &0xFF00) |val;
			pdSetIRQ(1);
			if (val ==0) counter =0;
			break;
		case 0x101:
			counter =(counter &0x00FF) |(val <<8);
			pdSetIRQ(1);
			break;
		case 0x2FC: case 0x2FD: case 0x2FE: case 0x2FF:
			modeGD =val >>5;
			protectPRG =!!(addr &2);
			mirroring =(addr &1? 2: 0) | (val &0x10? 1: 0);
			sync();
			break;
		case 0x3FE: case 0x3FF:
			modeMC =!(addr &1);
			latchGD =(latchGD &~3) | (val &3);
			sync();
			break;
		default:
			passCPUWrite(bank, addr, val);
			break;
	} else
	if (bank ==0x6 || bank ==0x7)
		stateRAM.k4[bank &7][addr] =val;
	else
	if (bank >=0x8) {
		int index =(bank &7) >>1;
		if (protectPRG) {
			latchGD =val;
			prgMC[index] =val >>2;
			sync();
		} else
		if (modeMC)	
			prgram.k8[prgMC[index]][bank &1][addr] =val;
		else
			prgram.k8[prgGD[index]][bank &1][addr] =val;
	} else
		passCPUWrite(bank, addr, val);
}
int	MAPINT	ppuRead (int bank, int addr) {
	if (bank >=0x8)
		return ntRead(bank, addr, mirroring);
	else
		return chrram.k8[chrGD][bank][addr];
}
void	MAPINT	ppuWrite (int bank, int addr, int val) {
	if (bank >=0x8)
		ntWrite(bank, addr, val, mirroring);
	else
	if (modeGD <4)
		chrram.k8[chrGD][bank][addr] =val;
}

void	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		modeGD =0;
		modeMC =false;
		protectPRG =true;
		for (int i =0; i <4; i++) prgMC[i] =i;
		chrGD =0;
		latchGD =0;
		mirroring =2;
	}
	counter =0;
	sync();
}

void	cpuCycle (void) {
	if (counter &0x8000 && !++counter) pdSetIRQ(0);

}

int	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, modeGD);
	SAVELOAD_BOOL(stateMode, offset, data, modeMC);
	SAVELOAD_BOOL(stateMode, offset, data, protectPRG);
	for (auto& c: prgGD) SAVELOAD_BYTE(stateMode, offset, data, c);
	for (auto& c: prgMC) SAVELOAD_BYTE(stateMode, offset, data, c);
	SAVELOAD_BYTE(stateMode, offset, data, chrGD);
	SAVELOAD_BYTE(stateMode, offset, data, latchGD);
	SAVELOAD_BYTE(stateMode, offset, data, mirroring);
	SAVELOAD_WORD(stateMode, offset, data, counter);
	return offset;
}
} // namespace SGD4

// ------------ MMC1
namespace MMC1 {
int		prgMask;
int		chrMask;
bool		protectCHR;
uint8_t		latch;
uint8_t		count;
uint8_t		prg;
uint8_t		chr[2];
uint8_t		mirroring;
uint8_t		prgMode;
bool		chr4K;

int	MAPINT	cpuRead (int bank, int addr) {
	if (bank >=0x8) {
		int extra =fusemapLetter =='B'? chr[0] &0x10: 0;
		switch (prgMode) {
			case 2:
				return prgram.k16[(bank &4? prg:   0) &prgMask |extra][bank &3][addr];
			case 3:
				return prgram.k16[(bank &4? ~0 : prg) &prgMask |extra][bank &3][addr];
			default:
				return prgram.k32[(prg &prgMask |extra) >>1][bank &7][addr];
		}
	} else
	if (bank ==0x6 || bank ==0x7)
		return stateRAM.k4[bank &7][addr];
	else
		return passCPURead(bank, addr);
}
int	MAPINT	cpuReadDebug (int bank, int addr) {
	if (bank >=0x6)
		return cpuRead(bank, addr);
	else
		return passCPUReadDebug(bank, addr);
}
void	MAPINT	cpuWrite (int bank, int addr, int val) {
	if (bank ==0x4 && addr ==0x2FF) {
		prgMask =(val &0x70 | 0x0F) >>3;
		chrMask =((val &0x0F) <<1) |1;
		protectCHR =!!(val &0x80);
	} else
	if (bank >=0x8) {
		if (val &0x80) {
			latch =0;
			count =0;
			prgMode =3;
		} else {
			latch |=(val &1) <<count++;
			if (count ==5) {
				switch (bank &6) {
					case 0: mirroring =latch &3;
						prgMode =(latch >>2) &3;
						chr4K =!!(latch &0x10);
						break;
					case 2:	chr[0] =latch;
						break;
					case 4: chr[1] =latch;
						break;
					case 6:	prg =latch &0xF;
						break;
				}
				latch =0;
				count =0;
			}
		}
	} else
	if (bank ==0x6 || bank ==0x7)
		stateRAM.k4[bank &7][addr] =val;
	else
		passCPUWrite(bank, addr, val);
}

int	MAPINT	ppuRead (int bank, int addr) {
	if (bank >=0x8)
		return ntRead(bank, addr, mirroring);
	else
	if (chr4K)
		return chrram.k4[chr[bank &4? 1: 0] &chrMask][bank &3][addr];
	else
		return chrram.k8[(chr[0] &chrMask) >>1][bank &7][addr];
}
void	MAPINT	ppuWrite (int bank, int addr, int val) {
	if (bank >=0x8)
		ntWrite(bank, addr, val, mirroring);
	else
	if (!protectCHR) {
		if (chr4K)
			chrram.k4[chr[bank >>2] &chrMask][bank &3][addr] =val;
		else
			chrram.k8[(chr[0] &chrMask) >>1][bank &7][addr] =val;
	}
}

void	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		latch =0;
		count =0;
		prg =0;
		for (auto& c: chr) c =0;
		mirroring =0;
		prgMode =3;
		chr4K =false;
	}
}

int	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_LONG(stateMode, offset, data, prgMask);
	SAVELOAD_LONG(stateMode, offset, data, chrMask);
	SAVELOAD_BOOL(stateMode, offset, data, protectCHR);
	SAVELOAD_BYTE(stateMode, offset, data, latch);
	SAVELOAD_BYTE(stateMode, offset, data, count);
	SAVELOAD_BYTE(stateMode, offset, data, prg);
	SAVELOAD_BYTE(stateMode, offset, data, chr[0]);
	SAVELOAD_BYTE(stateMode, offset, data, chr[1]);
	SAVELOAD_BYTE(stateMode, offset, data, mirroring);	
	SAVELOAD_BYTE(stateMode, offset, data, prgMode);
	SAVELOAD_BOOL(stateMode, offset, data, chr4K);
	return offset;
}
} // namespace MMC1

// ------------ MMC3
namespace MMC3 {
int		prgMask;
int		chrMask;
uint8_t		flipPRG;
uint8_t		flipCHR;
uint8_t		index;
uint8_t		reg[8];
bool		protectCHR;
bool		horizontalMirroring;

uint8_t		reloadValue;
uint8_t		irqCounter;
bool		irqEnabled;
uint8_t		pa12Filter;

int	MAPINT	cpuRead (int bank, int addr) {
	if (bank >=0x8) {
		int  b =0xFF; // not necessary to assign this; just to suppress a compiler warning
		if (~bank &2) bank ^=flipPRG;
		switch (bank &6) {
			case 0: b =reg[6]; break;
			case 2: b =reg[7]; break;
			case 4: b =0xFE; break;
			case 6: b =0xFF; break;
		}
		return prgram.k8[b &prgMask][bank &1][addr];
	} else
	if (bank ==0x6 || bank ==0x7)
		return stateRAM.k4[bank &7][addr];
	else
		return passCPURead(bank, addr);
}
int	MAPINT	cpuReadDebug (int bank, int addr) {
	if (bank >=0x6)
		return cpuRead(bank, addr);
	else
		return passCPUReadDebug(bank, addr);
}
void	MAPINT	cpuWrite (int bank, int addr, int val) {
	if (bank ==0x4 && addr ==0x2FF) {
		prgMask =(val &0x70 | 0x0F) >>2;
		protectCHR =true;
		switch(fusemapLetter) {
			case 'C':
			case 'M': chrMask =0x7F;
			          break;
			case 'N':
			case 'O': chrMask =0xFF;
			          break;
			case 'P': prgMask =0x3F; 
				  chrMask =0x07; 
				  protectCHR =false;
				  break;
			case 'T': chrMask =0x1F;
			          break;
		}
	} else
	if (bank ==0x6 || bank ==0x7)
		stateRAM.k4[bank &7][addr] =val;
	else
	if (bank >=0x8) {
		switch ((bank &6) | (addr &1)) {
			case 0:	index =val &7;
				flipPRG =val &0x40? 4: 0;
				flipCHR =val &0x80? 4: 0;
				break;
			case 1:	reg[index] =val;
				break;
			case 2:	horizontalMirroring =!!(val &1);
				break;
			case 4:	reloadValue =val;
				break;
			case 5:	irqCounter =0;
				break;
			case 6:	irqEnabled =false;
				pdSetIRQ(1);
				break;
			case 7:	irqEnabled =true;
				break;			
		}
	} else
		passCPUWrite(bank, addr, val);
}

int	MAPINT	ppuRead (int bank, int addr) {
	if (bank >=0x8)
		return ntRead(bank, addr, horizontalMirroring +2);
	else {
		if ((bank &4) ^flipCHR)
			return chrram.k1[ reg[(bank &3)  +2] &chrMask     ][addr];
		else
			return chrram.k2[(reg[(bank &3) >>1] &chrMask) >>1][bank &1][addr];		
	}
}
void	MAPINT	ppuWrite (int bank, int addr, int val) {
	if (bank >=0x8)
		ntWrite(bank, addr, val, horizontalMirroring +2);
	else
	if (!protectCHR) {
		if ((bank &4) ^flipCHR)
			chrram.k1[ reg[(bank &3)  +2] &chrMask     ][addr] =val;
		else
			chrram.k2[(reg[(bank &3) >>1] &chrMask) >>1][bank &1][addr] =val;	
	}
}

void	cpuCycle (void) {
	if (pa12Filter) pa12Filter--;
}

void	ppuCycle (int addr, int scanline, int cycle, int isRendering) {
	if (addr &0x1000) {
		if (!pa12Filter) {
			irqCounter =!irqCounter? reloadValue: --irqCounter;
			if (!irqCounter && irqEnabled) pdSetIRQ(0);
		}
		pa12Filter =5;
	}
}

void	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		flipPRG =0;
		flipCHR =0;
		index =0;
		reg[0] =0x00; reg[1] =0x02;
		reg[2] =0x04; reg[3] =0x05; reg[4] =0x06; reg[5] =0x07;
		reg[6] =0x00; reg[7] =0x01;
		horizontalMirroring =false;
		reloadValue =0;
		irqCounter =0;
		pa12Filter =0;
	}
	irqEnabled =false;
}

int	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_LONG(stateMode, offset, data, prgMask);
	SAVELOAD_LONG(stateMode, offset, data, chrMask);
	SAVELOAD_BYTE(stateMode, offset, data, flipPRG);
	SAVELOAD_BYTE(stateMode, offset, data, flipCHR);
	SAVELOAD_BYTE(stateMode, offset, data, index);
	for (auto& c: reg) SAVELOAD_BYTE(stateMode, offset, data, c);	
	SAVELOAD_BOOL(stateMode, offset, data, protectCHR);
	SAVELOAD_BOOL(stateMode, offset, data, horizontalMirroring);

	SAVELOAD_BYTE(stateMode, offset, data, reloadValue);
	SAVELOAD_BYTE(stateMode, offset, data, irqCounter);
	SAVELOAD_BOOL(stateMode, offset, data, irqEnabled);
	SAVELOAD_BYTE(stateMode, offset, data, pa12Filter);	
	return offset;
}
} // namespace MMC3

// ------------ MMC2
namespace MMC2 {
uint8_t		prg;
uint8_t		chr[4];
uint8_t		state[2];
bool		horizontalMirroring;
int	MAPINT	cpuRead (int bank, int addr) {
	if (bank >=0x8 && fusemapLetter =='D')
		return prgram.k8[(bank <0xA)? prg: ((bank >>1) +0x8)][bank &1][addr];
	else
	if (bank >=0x8 && fusemapLetter =='E')
		return prgram.k16[(bank <0xC)? prg: 0x7][bank &3][addr];
	else
	if (bank ==0x6 || bank ==0x7)
		return stateRAM.k4[bank &7][addr];
	else
		return passCPURead(bank, addr);
}
int	MAPINT	cpuReadDebug (int bank, int addr) {
	if (bank >=0x6)
		return cpuRead(bank, addr);
	else
		return passCPUReadDebug(bank, addr);
}
void	MAPINT	cpuWrite (int bank, int addr, int val) {
	if (bank >=0xA) switch(bank &7) {
		case 2:	prg =val &((fusemapLetter =='E')? 0x7: 0xF);
			break;
		case 3: case 4: case 5: case 6:
			chr[(bank &7) -3] =val &((fusemapLetter =='E')? 0xF: 0x1F);
			break;
		case 7: horizontalMirroring =!!(val &1);
			break;
	} else
	if (bank ==0x6 || bank ==0x7)
		stateRAM.k4[bank &7][addr] =val;
	else
		passCPUWrite(bank, addr, val);
}

int	MAPINT	ppuRead (int bank, int addr) {
	if (bank ==0x3 && fusemapLetter =='D') {
		if ((addr &0x3FF) ==0x3D8) state[0] =0; else
		if ((addr &0x3FF) ==0x3E8) state[0] =1;
	} else
	if (bank ==0x7 || bank ==0x3 && fusemapLetter =='E') {
		if ((addr &0x3F8) ==0x3D8) state[1] =0; else
		if ((addr &0x3F8) ==0x3E8) state[1] =1;
	}
	
	if (bank >=0x8)
		return ntRead(bank, addr, horizontalMirroring +2);
	else
		return chrram.k4[chr[(bank &4? 2: 0) + state[bank >>2]]][bank &3][addr];
}
void	MAPINT	ppuWrite (int bank, int addr, int val) {
	if (bank >=0x8)
		ntWrite(bank, addr, val, horizontalMirroring +2);
}

void	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		prg =0;
		for (auto& c: chr) c =0;
		for (auto& c: state) c =0;
		horizontalMirroring =false;
	}
}

int	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, prg);
	for (auto& c: chr) SAVELOAD_BYTE(stateMode, offset, data, c);	
	for (auto& c: state) SAVELOAD_BYTE(stateMode, offset, data, c);	
	SAVELOAD_BOOL(stateMode, offset, data, horizontalMirroring);
	return offset;
}
} // namespace MMC2

// ------------ N163
namespace N163 {
uint8_t		prg[4];
uint8_t		chr[12];

uint8_t		irqCounter;
bool		irqEnabled;

int	MAPINT	cpuRead (int bank, int addr) {
	if (bank >=0x8) {
		return prgram.k8[prg[(bank &7) >>1] &0xF][bank &1][addr];
	} else
	if (bank ==0x6 || bank ==0x7)
		return stateRAM.k4[bank &7][addr];
	else
		return passCPURead(bank, addr);
}
int	MAPINT	cpuReadDebug (int bank, int addr) {
	if (bank >=0x6)
		return cpuRead(bank, addr);
	else
		return passCPUReadDebug(bank, addr);
}
void	MAPINT	cpuWrite (int bank, int addr, int val) {
	if (bank ==0x6 || bank ==0x7)
		stateRAM.k4[bank &7][addr] =val;
	else
	if (bank >=0x8 && bank <=0xD)
		chr[((bank &7) <<1) | ((addr &0x800)? 1: 0)] =val;
	else
	if (bank >=0xE) {
		int reg =((bank &1) <<1) | ((addr &0x800)? 1: 0);
		if (reg <3)
			prg[reg] =val;
		else {
			if (addr &1)
				irqEnabled =!!(val &2);
			else
				irqCounter =val;
			pdSetIRQ(1);
		}		
	} else
		passCPUWrite(bank, addr, val);
}

int	MAPINT	ppuRead (int bank, int addr) {
	if (bank >=12) bank -=4;
	bool forceVROM =false;
	if (bank <4 && prg[1] &0x40) forceVROM =true; else
	if (bank <8 && prg[1] &0x80) forceVROM =true;
	if (chr[bank] <0xE0 || forceVROM)
		return chrram.k1[chr[bank]][addr];
	else
		return PPU::PPU[0]->VRAM[chr[bank] &1][addr];
}
void	MAPINT	ppuWrite (int bank, int addr, int val) {
	if (bank >=0x8 && (bank <0xF || addr <0xF00) && chr[bank] >=0xE0)
		PPU::PPU[0]->VRAM[chr[bank] &1][addr] =val;
}

void	ppuCycle (int addr, int scanline, int cycle, int isRendering) {
	if (isRendering && cycle ==260 && irqEnabled && !++irqCounter) pdSetIRQ(0);
}

void	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		prg[3] =0x0F;
		for (int i =0; i <12; i++) chr[i] =i;
	}
	irqCounter =0;
	irqEnabled =false;
}

int	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	for (auto& c: prg) SAVELOAD_BYTE(stateMode, offset, data, c);
	for (auto& c: chr) SAVELOAD_BYTE(stateMode, offset, data, c);
	SAVELOAD_BYTE(stateMode, offset, data, irqCounter);
	SAVELOAD_BOOL(stateMode, offset, data, irqEnabled);
	return offset;
}
} // namespace N163

// ------------ N118
namespace N118 {
int		prgMask;
int		chrMask;
uint8_t		index;
uint8_t		reg[8];

bool		horizontalMirroring;
bool		extendedMirroring;

int	MAPINT	cpuRead (int bank, int addr) {
	if (bank >=0x8) {
		int  b =0xFF; // not necessary to assign this; just to suppress a compiler warning
		switch (bank &6) {
			case 0: b =reg[6]; break;
			case 2: b =reg[7]; break;
			case 4: b =0xFE; break;
			case 6: b =0xFF; break;
		}
		return prgram.k8[b &prgMask][bank &1][addr];
	} else
	if (bank ==0x6 || bank ==0x7)
		return stateRAM.k4[bank &7][addr];
	else
		return passCPURead(bank, addr);
}
int	MAPINT	cpuReadDebug (int bank, int addr) {
	if (bank >=0x6)
		return cpuRead(bank, addr);
	else
		return passCPUReadDebug(bank, addr);
}
void	MAPINT	cpuWrite (int bank, int addr, int val) {
	if (bank ==0x4 && addr ==0x2FF) {
		prgMask =val &0x0C |0x03;
		chrMask =(((val &0x03) <<5) |0x1F) >>(fusemapLetter =='L'? 1: 0);
		horizontalMirroring =!!(val &0x40);
		extendedMirroring =!!(val &0x20);		
	} else
	if (bank ==0x6 || bank ==0x7)
		stateRAM.k4[bank &7][addr] =val;
	else
	if (bank >=0x8) {
		switch ((bank &6) | (addr &1)) {
			case 0:	index =val &7;
				break;
			case 1:	reg[index] =val;
				break;
		}
	} else
		passCPUWrite(bank, addr, val);
}

int	MAPINT	ppuReadG (int bank, int addr) {
	if (bank >=0x8) {
		if (extendedMirroring) {
			if (bank &4)
				return PPU::PPU[0]->VRAM[(reg[(bank &3)  +2] >>5) &1][addr];
			else
				return PPU::PPU[0]->VRAM[(reg[(bank &3) >>1] >>5) &1][addr];
		} else
			return ntRead(bank, addr, horizontalMirroring +2);
	} else {
		if (bank &4)
			return chrram.k1[ reg[(bank &3)  +2] &chrMask     ][addr];
		else
			return chrram.k2[(reg[(bank &3) >>1] &chrMask) >>1][bank &1][addr];		
	}
}
int	MAPINT	ppuReadL (int bank, int addr) {
	if (bank >=0x8) {
		if (extendedMirroring)
			return PPU::PPU[0]->VRAM[(reg[(bank >>1)  +2] >>5) &1][addr];
		else
			return ntRead(bank, addr, horizontalMirroring +2);
	} else {
		return chrram.k2[reg[(bank >>1)+2] &chrMask][bank &1][addr];
	}
}
void	MAPINT	ppuWriteG (int bank, int addr, int val) {
	if (bank >=0x8) {
		if (extendedMirroring && (bank <0xF || addr <0xF00)) {
			if (bank &4)
				PPU::PPU[0]->VRAM[(reg[(bank &3)  +2] >>5) &1][addr] =val;
			else
				PPU::PPU[0]->VRAM[(reg[(bank &3) >>1] >>5) &1][addr] =val;
		} else
			ntWrite(bank, addr, val, horizontalMirroring +2);
	}
}
void	MAPINT	ppuWriteL (int bank, int addr, int val) {
	if (bank >=0x8) {
		if (extendedMirroring && (bank <0xF || addr <0xF00))
			PPU::PPU[0]->VRAM[(reg[(bank >>1) +2] >>5) &1][addr] =val;
		else
			ntWrite(bank, addr, val, horizontalMirroring +2);
	}
}

void	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		index =0;
		reg[0] =0x00; reg[1] =0x02;
		reg[2] =0x04; reg[3] =0x05; reg[4] =0x06; reg[5] =0x07;
		reg[6] =0x00; reg[7] =0x01;
	}
}

int	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_LONG(stateMode, offset, data, prgMask);
	SAVELOAD_LONG(stateMode, offset, data, chrMask);
	SAVELOAD_BYTE(stateMode, offset, data, index);
	for (auto& c: reg) SAVELOAD_BYTE(stateMode, offset, data, c);	
	SAVELOAD_BOOL(stateMode, offset, data, horizontalMirroring);
	SAVELOAD_BOOL(stateMode, offset, data, extendedMirroring);
	return offset;
}
} // namespace N118

// ------------ Sunsoft-4
namespace SUN4 {
uint8_t		prg[2];
uint8_t		chr[4];
uint8_t		nt[2];
uint8_t		mirroring;
bool		vrom;

int	MAPINT	cpuRead (int bank, int addr) {
	if (bank >=0x8) {
		return prgram.k16[prg[(bank &7) >>2] &0x7][bank &3][addr];
	} else
	if (bank ==0x6 || bank ==0x7)
		return stateRAM.k4[bank &7][addr];
	else
		return passCPURead(bank, addr);
}
int	MAPINT	cpuReadDebug (int bank, int addr) {
	if (bank >=0x6)
		return cpuRead(bank, addr);
	else
		return passCPUReadDebug(bank, addr);
}
void	MAPINT	cpuWrite (int bank, int addr, int val) {
	if (bank ==0x6 || bank ==0x7)
		stateRAM.k4[bank &7][addr] =val;
	else
	if (bank >=0x8 && bank <=0xB)
		chr[bank &3] =val;
	else
	if (bank ==0xC || bank ==0xD)
		nt[bank &1] =val;
	else
	if (bank ==0xE) {
		mirroring =(val &3) ^2;
		vrom =!!(val &0x10);
	} else
	if (bank ==0xF)
		prg[0] =val;
	else
		passCPUWrite(bank, addr, val);
}

int	MAPINT	ppuRead (int bank, int addr) {
	if (bank &8) {
		bank &=3;
		if (vrom) switch (mirroring &3) {
			default: // Not necessary; just to suppress compiler warning
			case 0:	return chrram.k1[nt[bank &1      ] |0x80][addr];
			case 1:	return chrram.k1[nt[(bank >>1) &1] |0x80][addr];
			case 2:	return chrram.k1[nt[0            ] |0x80][addr];
			case 3:	return chrram.k1[nt[1            ] |0x80][addr];
		} else
			return ntRead(bank, addr, mirroring ^2);
	} else
		return chrram.k2[chr[bank >>1]][bank &1][addr];
}
void	MAPINT	ppuWrite (int bank, int addr, int val) {
	if (!vrom && bank &8)
		ntWrite(bank, addr, val, mirroring);
}

void	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		prg[1] =0xFF;
		for (int i =0; i <4; i++) chr[i] =i;
		for (int i =0; i <2; i++) nt[i] =i;
		mirroring =2;
		vrom =false;
	}
}

int	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	for (auto& c: prg) SAVELOAD_BYTE(stateMode, offset, data, c);
	for (auto& c: chr) SAVELOAD_BYTE(stateMode, offset, data, c);
	for (auto& c: nt) SAVELOAD_BYTE(stateMode, offset, data, c);
	SAVELOAD_BYTE(stateMode, offset, data, mirroring);
	SAVELOAD_BOOL(stateMode, offset, data, vrom);
	return offset;
}
} // namespace SUN4

// ------------ VRC1
namespace VRC1 {
uint8_t		prg[4];
uint8_t		chr[2];
bool		horizontalMirroring;

int	MAPINT	cpuRead (int bank, int addr) {
	if (bank >=0x8) {
		return prgram.k8[prg[(bank &7) >>1] &0xF][bank &1][addr];
	} else
	if (bank ==0x6 || bank ==0x7)
		return stateRAM.k4[bank &7][addr];
	else
		return passCPURead(bank, addr);
}
int	MAPINT	cpuReadDebug (int bank, int addr) {
	if (bank >=0x6)
		return cpuRead(bank, addr);
	else
		return passCPUReadDebug(bank, addr);
}
void	MAPINT	cpuWrite (int bank, int addr, int val) {
	if (bank ==0x6 || bank ==0x7)
		stateRAM.k4[bank &7][addr] =val;
	else
	if ((bank &~1) ==0xE)
		chr[bank &1] =chr[bank &1] &0xF0 | (val &0xF);
	else
	if (bank ==0x9) {
		horizontalMirroring =!!(val &1);
		chr[0] =chr[0] &0xF | ((val &2)? 0x10: 0x00);
		chr[1] =chr[1] &0xF | ((val &4)? 0x10: 0x00);
	}
	else
	if (bank >=8)
		prg[(bank >>1) &3] =val;
	else
		passCPUWrite(bank, addr, val);
}

int	MAPINT	ppuRead (int bank, int addr) {
	if (bank >=0x8)
		return ntRead(bank, addr, horizontalMirroring +2);
	else
		return chrram.k4[chr[bank >>2]][bank &3][addr];
}
void	MAPINT	ppuWrite (int bank, int addr, int val) {
	if (bank >=0x8)
		ntWrite(bank, addr, val, horizontalMirroring +2);
}

void	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		for (int i =0; i <4; i++) prg[i] =0xFC |i;
		for (int i =0; i <2; i++) chr[i] =i;
		horizontalMirroring =false;
	}
}

int	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	for (auto& c: prg) SAVELOAD_BYTE(stateMode, offset, data, c);
	for (auto& c: chr) SAVELOAD_BYTE(stateMode, offset, data, c);
	SAVELOAD_BOOL(stateMode, offset, data, horizontalMirroring);
	return offset;
}
} // namespace VRC1

// ------------ FME7
namespace FME7 {
uint8_t		prg[4];
uint8_t		chr[8];
uint8_t		index;
uint8_t		mirroring;

uint16_t	irqCounter;
bool		counting;
bool		irqEnabled;

int	MAPINT	cpuRead (int bank, int addr) {
	if (bank >=0x8) {
		return prgram.k8[prg[(bank &7) >>1] &0xF][bank &1][addr];
	} else
	if (bank ==0x6 || bank ==0x7)
		return stateRAM.k4[bank &7][addr];
	else
		return passCPURead(bank, addr);
}
int	MAPINT	cpuReadDebug (int bank, int addr) {
	if (bank >=0x6)
		return cpuRead(bank, addr);
	else
		return passCPUReadDebug(bank, addr);
}
void	MAPINT	cpuWrite (int bank, int addr, int val) {
	if (bank ==0x6 || bank ==0x7)
		stateRAM.k4[bank &7][addr] =val;
	else
	if ((bank &~1) ==0x8)
		index =val &0xF;
	else
	if ((bank &~1) ==0xA) switch(index) {
		case 0:	case 1: case 2: case 3: case 4: case 5: case 6: case 7:
			chr[index] =val;
			break;
		case 9: case 10: case 11: 
			prg[index -9] =val;
			break;
		case 12:mirroring =(val &3) ^2;
			break;
		case 13:counting =!!(val &0x80);			
			irqEnabled =!!(val &0x01);
			pdSetIRQ(1);
			break;
		case 14:irqCounter =(irqCounter &0xFF00) |val;
			break;
		case 15:irqCounter =(irqCounter &0x00FF) |(val <<8);
			break;
	} else
		passCPUWrite(bank, addr, val);
}

int	MAPINT	ppuRead (int bank, int addr) {
	if (bank >=0x8)
		return ntRead(bank, addr, mirroring);
	else
		return chrram.k1[chr[bank] &0x7F][addr];
}
void	MAPINT	ppuWrite (int bank, int addr, int val) {
	if (bank >=0x8)
		ntWrite(bank, addr, val, mirroring);
}

void	cpuCycle (void) {
	if (counting && !--irqCounter && irqEnabled) pdSetIRQ(0);
}

void	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		prg[3] =0xFF;
		for (int i =0; i <8; i++) chr[i] =i;
		mirroring =2;
	}
	irqCounter =0;
	counting =false;
	irqEnabled =false;
}

int	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	for (auto& c: prg) SAVELOAD_BYTE(stateMode, offset, data, c);
	for (auto& c: chr) SAVELOAD_BYTE(stateMode, offset, data, c);
	SAVELOAD_BYTE(stateMode, offset, data, index);
	SAVELOAD_BYTE(stateMode, offset, data, mirroring);

	SAVELOAD_WORD(stateMode, offset, data, irqCounter);
	SAVELOAD_BOOL(stateMode, offset, data, counting);
	SAVELOAD_BOOL(stateMode, offset, data, irqEnabled);	
	return offset;
}
} // namespace FME7

// ------------ G101
namespace G101 {
uint8_t		prg[2];
uint8_t		chr[8];
uint8_t		flipPRG;
bool		horizontalMirroring;

int	MAPINT	cpuRead (int bank, int addr) {
	if (bank >=0x8) {
		int  b =0xFF; // not necessary to assign this; just to suppress a compiler warning
		if (~bank &2) bank ^=flipPRG;
		switch (bank &6) {
			case 0: b =prg[0]; break;
			case 2: b =prg[1]; break;
			case 4: b =0xFE; break;
			case 6: b =0xFF; break;
		}
		return prgram.k8[b &0xF][bank &1][addr];
	} else
	if (bank ==0x6 || bank ==0x7)
		return stateRAM.k4[bank &7][addr];
	else
		return passCPURead(bank, addr);
}
int	MAPINT	cpuReadDebug (int bank, int addr) {
	if (bank >=0x6)
		return cpuRead(bank, addr);
	else
		return passCPUReadDebug(bank, addr);
}
void	MAPINT	cpuWrite (int bank, int addr, int val) {
	if (bank ==0x6 || bank ==0x7)
		stateRAM.k4[bank &7][addr] =val;
	else
	if (bank ==0x8 || bank ==0xA)
		prg[(bank >>1) &1] =val;
	else
	if (bank ==0x9) {
		horizontalMirroring =!!(val &1);
		flipPRG =val &2? 4: 0;
	}
	else
	if (bank ==0xB)
		chr[addr &7] =val;
	else
		passCPUWrite(bank, addr, val);
}

int	MAPINT	ppuRead (int bank, int addr) {
	if (bank >=0x8)
		return ntRead(bank, addr, horizontalMirroring +2);
	else
		return chrram.k1[chr[bank]][addr];
}
void	MAPINT	ppuWrite (int bank, int addr, int val) {
	if (bank >=0x8)
		ntWrite(bank, addr, val, horizontalMirroring +2);
}

void	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		for (int i =0; i <2; i++) prg[i] =i;
		for (int i =0; i <8; i++) chr[i] =i;
		horizontalMirroring =false;
	}
}

int	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	for (auto& c: prg) SAVELOAD_BYTE(stateMode, offset, data, c);
	for (auto& c: chr) SAVELOAD_BYTE(stateMode, offset, data, c);
	SAVELOAD_BYTE(stateMode, offset, data, flipPRG);
	SAVELOAD_BOOL(stateMode, offset, data, horizontalMirroring);
	return offset;
}
} // namespace G101

// ------------ AMROM/ANROM/AOROM
namespace AROM {
uint8_t		latch;
int	MAPINT	cpuRead (int bank, int addr) {
	if (bank >=0x8)
		return prgram.k32[latch &3][bank &7][addr];
	else
	if (bank ==0x6 || bank ==0x7)
		return stateRAM.k4[bank &7][addr];
	else
		return passCPURead(bank, addr);
}
int	MAPINT	cpuReadDebug (int bank, int addr) {
	if (bank >=0x6)
		return cpuRead(bank, addr);
	else
		return passCPUReadDebug(bank, addr);
}
void	MAPINT	cpuWrite (int bank, int addr, int val) {
	if (bank >=0x8)
		latch =val;
	else
	if (bank ==0x6 || bank ==0x7)
		stateRAM.k4[bank &7][addr] =val;
	else
		passCPUWrite(bank, addr, val);
}

int	MAPINT	ppuRead (int bank, int addr) {
	if (bank >=0x8)
		return ntRead(bank, addr, latch &0x10? 1: 0);
	else
		return chrram.k8[0][bank &7][addr];
}
void	MAPINT	ppuWrite (int bank, int addr, int val) {
	if (bank >=0x8)
		ntWrite(bank, addr, val, latch &0x10? 1: 0);
	else
		chrram.k8[0][bank &7][addr] =val;
}
void	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) latch =0;
}

int	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, latch);
	return offset;
}
} // namespace AROM

// ------------ Tait TC0x90
namespace TC90 {
uint8_t		reg[8];
bool		horizontalMirroring;

uint8_t		reloadValue;
uint8_t		irqCounter;
bool		irqEnabled;
uint8_t		pa12Filter;

int	MAPINT	cpuRead (int bank, int addr) {
	if (bank >=0x8) {
		int  b =0xFF; // not necessary to assign this; just to suppress a compiler warning
		switch (bank &6) {
			case 0: b =reg[0]; break;
			case 2: b =reg[1]; break;
			case 4: b =0xFE; break;
			case 6: b =0xFF; break;
		}
		return prgram.k8[b &0xF][bank &1][addr];
	} else
	if (bank ==0x6 || bank ==0x7)
		return stateRAM.k4[bank &7][addr];
	else
		return passCPURead(bank, addr);
}
int	MAPINT	cpuReadDebug (int bank, int addr) {
	if (bank >=0x6)
		return cpuRead(bank, addr);
	else
		return passCPUReadDebug(bank, addr);
}
void	MAPINT	cpuWrite (int bank, int addr, int val) {
	if (bank ==0x6 || bank ==0x7)
		stateRAM.k4[bank &7][addr] =val;
	else
	if (bank >=0x8 && bank <=0xB)
		reg[((bank &6) <<1) | (addr &3)] =val;
	else
	if (bank ==0xC || bank ==0xD) {
		pdSetIRQ(1);
		switch (addr &3) {
			case 0:	reloadValue =val; break;
			case 1:	irqCounter =0; break;
			case 2: irqEnabled =true; break;
			case 3:	irqEnabled =false; break;		
		}
	} else
	if (bank ==0xE || bank ==0xF)
		horizontalMirroring =!!(val &0x40);
	else
		passCPUWrite(bank, addr, val);
}

int	MAPINT	ppuRead (int bank, int addr) {
	if (bank >=0x4 && bank <=0x7) {
		if (pa12Filter ==0) {
			irqCounter =!irqCounter? reloadValue: ++irqCounter;
			if (!irqCounter && irqEnabled) pdSetIRQ(0);
		}
		pa12Filter =3;
	}
	
	if (bank >=0x8)
		return ntRead(bank, addr, horizontalMirroring +2);
	else {
		if (bank &4)
			return chrram.k1[reg[bank         ]][addr];
		else
			return chrram.k2[reg[(bank >>1) +2]][bank &1][addr];
	}
}
void	MAPINT	ppuWrite (int bank, int addr, int val) {
	if (bank >=0x8)
		ntWrite(bank, addr, val, horizontalMirroring +2);
}

void	cpuCycle (void) {
	if (pa12Filter) pa12Filter--;
}

void	ppuCycle (int addr, int scanline, int cycle, int isRendering) {
	if (addr &0x1000) {
		if (!pa12Filter) {
			irqCounter =!irqCounter? reloadValue: --irqCounter;
			if (!irqCounter && irqEnabled) pdSetIRQ(0);
		}
		pa12Filter =5;
	}
}

void	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		reg[0] =0x00; reg[1] =0x01;
		reg[2] =0x04; reg[3] =0x05; reg[4] =0x06; reg[5] =0x07;
		reg[6] =0x00; reg[7] =0x01;
		horizontalMirroring =false;
	}
	reloadValue =0;
	irqCounter =0;
	irqEnabled =false;
	pa12Filter =0;
}

int	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	for (auto& c: reg) SAVELOAD_BYTE(stateMode, offset, data, c);
	SAVELOAD_BOOL(stateMode, offset, data, horizontalMirroring);

	SAVELOAD_BYTE(stateMode, offset, data, reloadValue);
	SAVELOAD_BYTE(stateMode, offset, data, irqCounter);
	SAVELOAD_BOOL(stateMode, offset, data, irqEnabled);
	SAVELOAD_BYTE(stateMode, offset, data, pa12Filter);
	return offset;
}
} // namespace TC90

// ------------ AVE NINA-003
namespace NINA {
bool		chrRAM;
bool		horizontalMirroring;
uint8_t		latch;
bool		raiseIRQ;

int	MAPINT	cpuRead (int bank, int addr) {
	if (bank ==0xF && addr ==0xFFF) pdSetIRQ(1);
	if (bank >=0x8)
		return prgram.k32[0][bank &7][addr];
	else
	if (bank ==0x6 || bank ==0x7)
		return stateRAM.k4[bank &7][addr];
	else
		return passCPURead(bank, addr);
}
int	MAPINT	cpuReadDebug (int bank, int addr) {
	if (bank >=0x6)
		return cpuRead(bank, addr);
	else
		return passCPUReadDebug(bank, addr);
}
void	MAPINT	cpuWrite (int bank, int addr, int val) {
	if (bank ==0x4 && addr ==0x2FF) {
		horizontalMirroring =!!(val &0x10);
		chrRAM =!!(val &0x80);
	} else
	if (bank ==0x4 && addr &0x100 || bank >=0x8) {
		latch =val;
		raiseIRQ =true;
	} else
	if (bank ==0x6 || bank ==0x7)
		stateRAM.k4[bank &7][addr] =val;
	else
		passCPUWrite(bank, addr, val);
}

int	MAPINT	ppuRead (int bank, int addr) {
	if (bank >=0x8)
		return ntRead(bank, addr, horizontalMirroring +2);
	else
		return chrram.k8[chrRAM? 0: latch &7][bank &7][addr];
}
void	MAPINT	ppuWrite (int bank, int addr, int val) {
	if (bank >=0x8)
		ntWrite(bank, addr, val, horizontalMirroring +2);
	else
	if (chrRAM)
		chrram.k8[0][bank &7][addr] =val;
}
void	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) latch =0;
	raiseIRQ =false;
}

void	ppuCycle (int addr, int scanline, int cycle, int isRendering) {
	if (raiseIRQ && scanline ==108) {
		pdSetIRQ(0);
		raiseIRQ =false;
	}
}

int	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BOOL(stateMode, offset, data, chrRAM);
	SAVELOAD_BOOL(stateMode, offset, data, horizontalMirroring);
	SAVELOAD_BYTE(stateMode, offset, data, latch);
	return offset;
}
} // namespace NINA

// ------------ Bandai FCG1/LZ93D50
namespace FCG1 {
int		prgMask;
int		chrMask;
uint8_t		prg;
uint8_t		chr[8];
uint8_t		mirroring;

uint16_t	irqCounter;
uint16_t	irqLatch;
bool		irqEnabled;

int	MAPINT	cpuRead (int bank, int addr) {
	if (bank >=0x8) {
		return prgram.k16[(bank &4? 0xFF: prg) &prgMask][bank &3][addr];
	} else
	if (fusemapLetter =='Y' && (bank ==0x6 || bank ==0x7))
		return stateRAM.k4[bank &7][addr];
	else
		return passCPURead(bank, addr);
}
int	MAPINT	cpuReadDebug (int bank, int addr) {
	if (bank >=(fusemapLetter =='Y'? 0x6: 0x8))
		return cpuRead(bank, addr);
	else
		return passCPUReadDebug(bank, addr);
}
void	MAPINT	cpuWrite (int bank, int addr, int val) {
	if (bank ==0x4 && addr ==0x2FF) {
		prgMask =val &0x80? 0x0F: 0x07;
		chrMask =val &0x04? 0xFF: 0x7F;
	} else
	if (fusemapLetter =='Y' && (bank ==0x6 || bank ==0x7))
		stateRAM.k4[bank &7][addr] =val;
	else
	if (fusemapLetter =='V' && (bank ==0x6 || bank ==0x7) || bank >=0x8) {
		switch (addr &0xF) {
			case 0:	case 1: case 2: case 3: case 4: case 5: case 6: case 7:
				chr[addr &7] =val;
				break;
			case 8:	prg =val;
				break;
			case 9:	mirroring =(val &3) ^2;
				break;
			case 10:irqEnabled =!!(val &1);
				irqCounter =irqLatch;
				pdSetIRQ(irqEnabled && !irqCounter? 0: 1);			
				break;
			case 11:irqLatch =(irqLatch &0xFF00) |val;
				break;
			case 12:irqLatch =(irqLatch &0x00FF) |(val <<8);
				break;
		}
	} else
		passCPUWrite(bank, addr, val);
}

int	MAPINT	ppuRead (int bank, int addr) {
	if (bank >=0x8)
		return ntRead(bank, addr, mirroring);
	else
		return chrram.k1[chr[bank] &chrMask][addr];
}
void	MAPINT	ppuWrite (int bank, int addr, int val) {
	if (bank >=0x8)
		ntWrite(bank, addr, val, mirroring);
}

void	cpuCycle (void) {
	if (irqEnabled && !--irqCounter) pdSetIRQ(0);
}

void	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		prg =0;
		for (int i =0; i <8; i++) chr[i] =i;
		mirroring =2;
	}
	irqCounter =0;
	irqLatch =0;
	irqEnabled =false;
}

int	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_LONG(stateMode, offset, data, prgMask);
	SAVELOAD_LONG(stateMode, offset, data, chrMask);
	SAVELOAD_BYTE(stateMode, offset, data, prg);
	for (auto& c: chr) SAVELOAD_BYTE(stateMode, offset, data, c);
	SAVELOAD_BYTE(stateMode, offset, data, mirroring);

	SAVELOAD_WORD(stateMode, offset, data, irqCounter);
	SAVELOAD_WORD(stateMode, offset, data, irqLatch);
	SAVELOAD_BOOL(stateMode, offset, data, irqEnabled);
	return offset;
}
} // namespace FCG1

// ------------ VRC4-like
namespace VRC4 {
int		prgMask;
uint8_t		prg[4];
uint8_t		chr[8];
uint8_t		mirroring;

uint8_t		vrcLatch;
uint8_t		vrcEnabled;
uint8_t		vrcCounter;
int16_t		vrcCycles;

uint8_t		pa12Counter;
uint8_t		pa12Filter;
bool		pa12Enabled;

int	MAPINT	cpuRead (int bank, int addr) {
	if (bank >=0x8) {
		return prgram.k8[prg[(bank &7) >>1] &prgMask][bank &1][addr];
	} else
	if (bank ==0x6 || bank ==0x7)
		return stateRAM.k4[bank &7][addr];
	else
		return passCPURead(bank, addr);
}
int	MAPINT	cpuReadDebug (int bank, int addr) {
	if (bank >=0x6)
		return cpuRead(bank, addr);
	else
		return passCPUReadDebug(bank, addr);
}
void	MAPINT	cpuWrite (int bank, int addr, int val) {
	if (bank ==0x4 && addr ==0x2FF)
		prgMask =val &0x20? 0x1F: 0x0F;
	else
	if (bank ==0x6 || bank ==0x7)
		stateRAM.k4[bank &7][addr] =val;
	else
	if ((bank &~1) ==0x8) {
		if ((addr &3) !=3)
			prg[addr &3] =val;
		else
			mirroring =(val &3) ^2;
	} else
	if (bank >=0xA && bank <=0xD)
		chr[(bank &4) | (addr &3)] =val;
	else
	if (bank ==0xE && fusemapLetter =='X') {
		if (addr &1)
			pa12Enabled =!!(val &2);
		else
			pa12Counter =val;
		pdSetIRQ(1);
	} else
	if (bank ==0xE && fusemapLetter =='Z') switch (addr &3) {
		case 0:	pa12Enabled =false;
			pdSetIRQ(1);
			break;
		case 1:	pa12Enabled =true;
			break;
		case 2:	pa12Counter +=val +1;
			break;
		case 3:	pa12Counter =0;
			break;
	} else
	if (bank ==0xF) switch(((addr &4)? 2: 0) | ((addr &8)? 1: 0)) {
		case 0:	vrcLatch = (vrcLatch &0xF0) | (val &0xF);
			break;
		case 1:	vrcLatch = (vrcLatch &0x0F) | (val <<4);
			break;
		case 2:	vrcEnabled = val &0x7;
			if (vrcEnabled &0x2) {
				vrcCounter =vrcLatch;
				vrcCycles =341;
			}
			pdSetIRQ(1);
			break;
		case 3:	if (vrcEnabled &0x1)
				vrcEnabled |= 0x2;
			else
				vrcEnabled &=~0x2;
			pdSetIRQ(1);
			break;
	} else
		passCPUWrite(bank, addr, val);
}

int	MAPINT	ppuRead (int bank, int addr) {
	if (bank >=0x8)
		return ntRead(bank, addr, mirroring);
	else
		return chrram.k1[chr[bank]][addr];
}
void	MAPINT	ppuWrite (int bank, int addr, int val) {
	if (bank >=0x8)
		ntWrite(bank, addr, val, mirroring);
}

void	cpuCycle (void) {
	if (vrcEnabled &2 && (vrcEnabled &4 || (vrcCycles -=3) <=0)) {
		if (~vrcEnabled &4) vrcCycles +=341;
		if (vrcCounter ==0xFF) {
			vrcCounter =vrcLatch;
			pdSetIRQ(0);
		} else
			vrcCounter++;
	}
}

void	ppuCycleX (int addr, int scanline, int cycle, int isRendering) {
	if (pa12Filter) pa12Filter--;
	if (addr &0x1000) {
		if (!pa12Filter && pa12Enabled && !++pa12Counter) pdSetIRQ(0);
		pa12Filter =8;
	}	
}

void	ppuCycleZ (int addr, int scanline, int cycle, int isRendering) {
	if (pa12Filter) pa12Filter--;
	if (addr &0x1000) {
		if (!pa12Filter && !--pa12Counter && pa12Enabled) pdSetIRQ(0);
		pa12Filter =8;
	}
}

void	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		for (int i =0; i <4; i++) prg[i] =0xFC |i;
		for (int i =0; i <8; i++) chr[i] =i;
		mirroring =2;
	}
	vrcLatch =0;
	vrcEnabled =0;
	vrcCounter =0;
	vrcCycles =0;
	
	pa12Counter =0;
	pa12Filter =0;
	pa12Enabled =false;
	
}

int	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_LONG(stateMode, offset, data, prgMask);
	for (auto& c: prg) SAVELOAD_BYTE(stateMode, offset, data, c);
	for (auto& c: chr) SAVELOAD_BYTE(stateMode, offset, data, c);
	SAVELOAD_BYTE(stateMode, offset, data, mirroring);

	SAVELOAD_BYTE(stateMode, offset, data, vrcLatch);
	SAVELOAD_BYTE(stateMode, offset, data, vrcEnabled);
	SAVELOAD_BYTE(stateMode, offset, data, vrcCounter);
	SAVELOAD_WORD(stateMode, offset, data, vrcCycles);

	SAVELOAD_BYTE(stateMode, offset, data, pa12Counter);
	SAVELOAD_BYTE(stateMode, offset, data, pa12Filter);
	SAVELOAD_BOOL(stateMode, offset, data, pa12Enabled);
	return offset;
}

} // namespace VRC4

// ------------ Common
FCPURead	cpuRead_mapper;
FCPURead	cpuReadDebug_mapper;
FCPUWrite	cpuWrite_mapper;
FPPURead	ppuRead_mapper;
FPPURead	ppuReadDebug_mapper;
FPPUWrite	ppuWrite_mapper;
void		(*reset_mapper)(RESET_TYPE);
void		(*cpuCycle_mapper)(void);
void		(*ppuCycle_mapper)(int, int, int, int);
int		(*saveLoad_mapper)(STATE_TYPE, int, unsigned char *);

void	none (void) {
};
void	none (RESET_TYPE) {
};
void	none (int, int, int, int) {
};
int	none (STATE_TYPE, int offset, unsigned char *) {
	return offset;
};

const struct {
	uint32_t	crc;
	uint8_t		letter;
	uint8_t		val55A9;
	FCPURead	cpuRead;
	FCPURead	cpuReadDebug;
	FCPUWrite	cpuWrite;
	FPPURead	ppuRead;
	FPPURead	ppuReadDebug;
	FPPUWrite	ppuWrite;
	void		(*reset)(RESET_TYPE);
	void		(*cpuCycle)(void);
	void		(*ppuCycle)(int, int, int, int);
	int		(*saveLoad)(STATE_TYPE, int, unsigned char *);
} fusemapInfo[] ={
	{ 0x407730AE, '0', 0x00, SGD4::cpuRead, SGD4::cpuReadDebug, SGD4::cpuWrite, SGD4::ppuRead, SGD4::ppuRead, SGD4::ppuWrite, SGD4::reset, SGD4::cpuCycle, none,           SGD4::saveLoad },
	{ 0x9C3E9D6B, '0', 0x00, SGD4::cpuRead, SGD4::cpuReadDebug, SGD4::cpuWrite, SGD4::ppuRead, SGD4::ppuRead, SGD4::ppuWrite, SGD4::reset, SGD4::cpuCycle, none,           SGD4::saveLoad },
	{ 0xD64B23A6, '0', 0x00, SGD4::cpuRead, SGD4::cpuReadDebug, SGD4::cpuWrite, SGD4::ppuRead, SGD4::ppuRead, SGD4::ppuWrite, SGD4::reset, SGD4::cpuCycle, none,           SGD4::saveLoad },
	{ 0xF7389C96, '0', 0x00, SGD4::cpuRead, SGD4::cpuReadDebug, SGD4::cpuWrite, SGD4::ppuRead, SGD4::ppuRead, SGD4::ppuWrite, SGD4::reset, SGD4::cpuCycle, none,           SGD4::saveLoad },
	{ 0x7C56037D, 'A', 0x01, MMC1::cpuRead, MMC1::cpuReadDebug, MMC1::cpuWrite, MMC1::ppuRead, MMC1::ppuRead, MMC1::ppuWrite, MMC1::reset, none,           none,           MMC1::saveLoad },
	{ 0xCFE7ED88, 'B', 0x01, MMC1::cpuRead, MMC1::cpuReadDebug, MMC1::cpuWrite, MMC1::ppuRead, MMC1::ppuRead, MMC1::ppuWrite, MMC1::reset, none,           none,           MMC1::saveLoad },
	{ 0x417FC571, 'C', 0x02, MMC3::cpuRead, MMC3::cpuReadDebug, MMC3::cpuWrite, MMC3::ppuRead, MMC3::ppuRead, MMC3::ppuWrite, MMC3::reset, MMC3::cpuCycle, MMC3::ppuCycle, MMC3::saveLoad },
	{ 0x0AE23F6A, 'D', 0x03, MMC2::cpuRead, MMC2::cpuReadDebug, MMC2::cpuWrite, MMC2::ppuRead, MMC2::ppuRead, MMC2::ppuWrite, MMC2::reset, none,           none,           MMC2::saveLoad },
	{ 0xC770D585, 'E', 0x03, MMC2::cpuRead, MMC2::cpuReadDebug, MMC2::cpuWrite, MMC2::ppuRead, MMC2::ppuRead, MMC2::ppuWrite, MMC2::reset, none,           none,           MMC2::saveLoad },
	{ 0xD7AFB8DA, 'F', 0x04, N163::cpuRead, N163::cpuReadDebug, N163::cpuWrite, N163::ppuRead, N163::ppuRead, N163::ppuWrite, N163::reset, none,           N163::ppuCycle, N163::saveLoad },
	{ 0xA23E27FE, 'G', 0x05, N118::cpuRead, N118::cpuReadDebug, N118::cpuWrite, N118::ppuReadG,N118::ppuReadG,N118::ppuWriteG,N118::reset, none,           none,           N118::saveLoad },
	{ 0xEA728D7C, 'I', 0x07, VRC1::cpuRead, VRC1::cpuReadDebug, VRC1::cpuWrite, VRC1::ppuRead, VRC1::ppuRead, VRC1::ppuWrite, VRC1::reset, none,           none,           VRC1::saveLoad },
	{ 0x94E218FF, 'J', 0x07, FME7::cpuRead, FME7::cpuReadDebug, FME7::cpuWrite, FME7::ppuRead, FME7::ppuRead, FME7::ppuWrite, FME7::reset, FME7::cpuCycle, none,           FME7::saveLoad },
	{ 0xD5BBF47D, 'K', 0x09, G101::cpuRead, G101::cpuReadDebug, G101::cpuWrite, G101::ppuRead, G101::ppuRead, G101::ppuWrite, G101::reset, none,           none,           G101::saveLoad },
	{ 0xB3938667, 'L', 0x05, N118::cpuRead, N118::cpuReadDebug, N118::cpuWrite, N118::ppuReadL,N118::ppuReadL,N118::ppuWriteL,N118::reset, none,           none,           N118::saveLoad },	
	{ 0x6BEAE324, 'M', 0x02, MMC3::cpuRead, MMC3::cpuReadDebug, MMC3::cpuWrite, MMC3::ppuRead, MMC3::ppuRead, MMC3::ppuWrite, MMC3::reset, MMC3::cpuCycle, MMC3::ppuCycle, MMC3::saveLoad },
	{ 0x6FBE1136, 'N', 0x02, MMC3::cpuRead, MMC3::cpuReadDebug, MMC3::cpuWrite, MMC3::ppuRead, MMC3::ppuRead, MMC3::ppuWrite, MMC3::reset, MMC3::cpuCycle, MMC3::ppuCycle, MMC3::saveLoad },
	{ 0x452B3763, 'O', 0x02, MMC3::cpuRead, MMC3::cpuReadDebug, MMC3::cpuWrite, MMC3::ppuRead, MMC3::ppuRead, MMC3::ppuWrite, MMC3::reset, MMC3::cpuCycle, MMC3::ppuCycle, MMC3::saveLoad },
	{ 0x717D64AF, 'P', 0x02, MMC3::cpuRead, MMC3::cpuReadDebug, MMC3::cpuWrite, MMC3::ppuRead, MMC3::ppuRead, MMC3::ppuWrite, MMC3::reset, MMC3::cpuCycle, MMC3::ppuCycle, MMC3::saveLoad },
	{ 0xC751EF4B, 'Q', 0x02, SUN4::cpuRead, SUN4::cpuReadDebug, SUN4::cpuWrite, SUN4::ppuRead, SUN4::ppuRead, SUN4::ppuWrite, SUN4::reset, none,           none,           SUN4::saveLoad },
	{ 0x8DC2E00B, 'R', 0x0F, AROM::cpuRead, AROM::cpuReadDebug, AROM::cpuWrite, AROM::ppuRead, AROM::ppuRead, AROM::ppuWrite, AROM::reset, none,           none,           AROM::saveLoad },
	{ 0xB20DA5E1, 'S', 0x0B, TC90::cpuRead, TC90::cpuReadDebug, TC90::cpuWrite, TC90::ppuRead, TC90::ppuRead, TC90::ppuWrite, TC90::reset, TC90::cpuCycle, none,           TC90::saveLoad },
	{ 0x62EEBF0A, 'T', 0x02, MMC3::cpuRead, MMC3::cpuReadDebug, MMC3::cpuWrite, MMC3::ppuRead, MMC3::ppuRead, MMC3::ppuWrite, MMC3::reset, MMC3::cpuCycle, MMC3::ppuCycle, MMC3::saveLoad },
	{ 0xABA73422, 'V', 0x0C, FCG1::cpuRead, FCG1::cpuReadDebug, FCG1::cpuWrite, FCG1::ppuRead, FCG1::ppuRead, FCG1::ppuWrite, FCG1::reset, FCG1::cpuCycle, none,           FCG1::saveLoad },
	{ 0x0C4BA4AB, 'W', 0x02, MMC3::cpuRead, MMC3::cpuReadDebug, MMC3::cpuWrite, MMC3::ppuRead, MMC3::ppuRead, MMC3::ppuWrite, MMC3::reset, MMC3::cpuCycle, MMC3::ppuCycle, MMC3::saveLoad },
	{ 0xD9994B70, 'X', 0x0B, VRC4::cpuRead, VRC4::cpuReadDebug, VRC4::cpuWrite, VRC4::ppuRead, VRC4::ppuRead, VRC4::ppuWrite, VRC4::reset, VRC4::cpuCycle, VRC4::ppuCycleX,VRC4::saveLoad },
	{ 0x727087F5, 'Y', 0x06, FCG1::cpuRead, FCG1::cpuReadDebug, FCG1::cpuWrite, FCG1::ppuRead, FCG1::ppuRead, FCG1::ppuWrite, FCG1::reset, FCG1::cpuCycle, none,           FCG1::saveLoad },
	{ 0x076A06C5, 'Z', 0x0B, VRC4::cpuRead, VRC4::cpuReadDebug, VRC4::cpuWrite, VRC4::ppuRead, VRC4::ppuRead, VRC4::ppuWrite, VRC4::reset, VRC4::cpuCycle, VRC4::ppuCycleZ,VRC4::saveLoad },
	{ 0x06231D8B, '3', 0x00, NINA::cpuRead, NINA::cpuReadDebug, NINA::cpuWrite, NINA::ppuRead, NINA::ppuRead, NINA::ppuWrite, NINA::reset, none,           NINA::ppuCycle, NINA::saveLoad },
	{ 0x9688F2DF, '-', 0x00, NINA::cpuRead, NINA::cpuReadDebug, NINA::cpuWrite, NINA::ppuRead, NINA::ppuRead, NINA::ppuWrite, NINA::reset, none,           none,           NINA::saveLoad },
};

void 	parseFusemap (void) {
	uint32_t crc =Hash::CRC32C(0, fusemapData, sizeof(fusemapData));
	loadFusemap(crc);
}

void	loadFusemap (uint32_t crc) {
	for (auto& fs: fusemapInfo) if (fs.crc ==crc) {
		if (fs.letter =='0' && NES::fqdFusemap !=0) {
			for (auto& ft: fusemapInfo) if (ft.letter ==NES::fqdFusemap) {
				loadFusemap(ft.crc);
				workRAM[0x05A9] =ft.val55A9;
				workRAM[0x05AB] =0x80;
				return; 
			}
		}
		EI.DbgOut(L"Loaded fusemap %c", fs.letter);
		fusemapLetter       =fs.letter;
		cpuRead_mapper      =fs.cpuRead;
		cpuReadDebug_mapper =fs.cpuReadDebug;
		cpuWrite_mapper     =fs.cpuWrite;
		ppuRead_mapper      =fs.ppuRead;
		ppuReadDebug_mapper =fs.ppuReadDebug;
		ppuWrite_mapper     =fs.ppuWrite;
		reset_mapper        =fs.reset;
		cpuCycle_mapper     =fs.cpuCycle;
		ppuCycle_mapper     =fs.ppuCycle;
		saveLoad_mapper     =fs.saveLoad;
		return;
	}
	EI.DbgOut(L"Unknown fusemap CRC 0x%08X", crc);
}

void	loadFusemap (uint8_t letter) {
	for (auto& c: fusemapInfo) if (c.letter ==letter) loadFusemap(c.crc);
}

} // namespace GameMaster
} // namespace PlugThruDevice