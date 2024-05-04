#include	"h_OneBus.h"
#include	"h_Latch.h"

#define BK16EN   !!(reg2000[0x10] &0x02)
#define SP16EN   !!(reg2000[0x10] &0x04)
#define SPEXTEN  !!(reg2000[0x10] &0x08)
#define BKEXTEN  !!(reg2000[0x10] &0x10)
#define V16BEN   !!(reg2000[0x10] &0x40 || reg4100[0x2B] ==0x61) //&& ROM->ConsoleType ==CONSOLE_VT09
#define VRWB       (reg2000[0x18] &0x07)
#define BKPAGE   !!(reg2000[0x18] &0x08)
#define VA18       (reg2000[0x18] >>4 &7)
#define VB0S       (reg2000[0x1A] &0x07)
#define RV6        (reg2000[0x1A] &0xF8)
#define VA21       (reg4100[0x00] &0x0F)
#define PA21       (reg4100[0x00] >>4)
#define pointer    (reg4100[0x05] &0x07)
#define COMR6    !!(reg4100[0x05] &0x40)
#define COMR7    !!(reg4100[0x05] &0x80)
#define HV         (reg4100[0x06] &0x01)
#define PQ0         reg4100[0x07]
#define PQ1         reg4100[0x08]
#define PQ2         reg4100[0x09]
#define PQ3         reg4100[0x0A]
#define	PS         (reg4100[0x0B] &0x07)
#define	FWEN     !!(reg4100[0x0B] &0x08)
#define	PQ2EN    !!(reg4100[0x0B] &0x40)
#define	TSYNEN   !!(reg4100[0x0B] &0x80)

namespace OneBus {
uint8_t*	chrData =NULL;
uint8_t*	chrLow =NULL;
uint8_t*	chrHigh =NULL;
uint8_t*	chrLow16 =NULL;
uint8_t*	chrHigh16 =NULL;
uint32_t	CHRROMSize;

uint8_t		reg2000[0x100];
uint8_t		reg4100[0x100];
uint32_t	relative8K;

uint8_t		counter;
uint8_t		reloadValue;
bool		enableIRQ;
uint8_t		pa12Filter;
uint8_t		irqDelay;

FSync		sync;
FCPURead	_readAPU;
FCPURead	_readAPUDebug;
FPPURead	_readNT;
FCPUWrite	_writeAPU;
FCPUWrite	_writePPU;
FCPUWrite	_writeROM;
FPPUWrite	_writeNT;


GPIO		gpio[4];

int	getPRGBank (int bank) {
	int prgAND =PS ==7? 0xFF: 0x3F >>PS;
	int prgOR  =(PQ3 | PA21 <<8) &~prgAND;
	int flip   =COMR6? 2: 0;
	
	if (~bank &1) bank ^=flip;
	switch (bank &3) {
		case 0: return (        PQ0        &prgAND |prgOR) +relative8K; break;
		case 1: return (        PQ1        &prgAND |prgOR) +relative8K; break;
		case 2: return ((PQ2EN? PQ2: 0xFE) &prgAND |prgOR) +relative8K; break;
		case 3: return (             0xFF  &prgAND |prgOR) +relative8K; break;
	}
}

void	syncPRG (int AND, int OR) {
	int prgAND =PS ==7? 0xFF: 0x3F >>PS;
	int prgOR  =(PQ3 | PA21 <<8) &~prgAND;
	int flip   =COMR6? 4: 0;
	
	if (ROM->ConsoleType ==CONSOLE_VT369 && reg4100[0x1C] &0x40)
		EMU->SetPRG_ROM8(0x6, ((reg4100[0x12] &prgAND |prgOR) +relative8K) &AND |OR);
	else
	if (ROM->PRGRAMSize >=8192)
		EMU->SetPRG_RAM8(0x6, 0);
	
	EMU->SetPRG_ROM8(0x8 ^flip,((        PQ0        &prgAND |prgOR) +relative8K) &AND |OR);
	EMU->SetPRG_ROM8(0xA      ,((        PQ1        &prgAND |prgOR) +relative8K) &AND |OR);
	EMU->SetPRG_ROM8(0xC ^flip,(((PQ2EN? PQ2: 0xFE) &prgAND |prgOR) +relative8K) &AND |OR);
	EMU->SetPRG_ROM8(0xE      ,((             0xFF  &prgAND |prgOR) +relative8K) &AND |OR);
	
	*EMU->multiPRGSize  =(prgAND +1) <<13;
	*EMU->multiPRGStart =ROM->PRGROMData + ((relative8K +prgOR +OR) <<13 &(ROM->PRGROMSize -1));
	if (reg2000[0x1E] !=0) *EMU->multiPRGSize  =4*1024*1024;
	
	ROM->vt369relative =(relative8K &0xFFF &AND |OR) <<13;
}

void	syncPRG16 (int bank0, int bank1, int AND, int OR) {
	int prgAND =PS ==7? 0xFF: 0x3F >>PS;
	int prgOR  =(PQ3 | PA21 <<8) &~prgAND;

	if (ROM->ConsoleType ==CONSOLE_VT369 && reg4100[0x1C] &0x40)
		EMU->SetPRG_ROM8(0x6, (reg4100[0x12] &prgAND |prgOR) &AND |OR);
	else
	if (ROM->PRGRAMSize >=8192)
		EMU->SetPRG_RAM8(0x6, 0);
	
	EMU->SetPRG_ROM8(0x8,((bank0 <<1 |0) &prgAND |prgOR) &AND |OR);
	EMU->SetPRG_ROM8(0xA,((bank0 <<1 |1) &prgAND |prgOR) &AND |OR);
	EMU->SetPRG_ROM8(0xC,((bank1 <<1 |0) &prgAND |prgOR) &AND |OR);
	EMU->SetPRG_ROM8(0xE,((bank1 <<1 |1) &prgAND |prgOR) &AND |OR);
}

void	setCHR (int bank, uint8_t* base, bool bit4pp, bool extended, int EVA, int AND, int OR) {
	static const uint8_t VB0STable[8] = { 0, 1, 2, 0, 3, 4, 5, 0 };
	int chrAND =0xFF >>VB0STable[VB0S];
	int chrOR =RV6 &~chrAND;
	int flip   =COMR7? 4: 0;

	int chrMask  =CHRROMSize -1;
	int multiCHRStart =chrOR;
	int relative =relative8K <<3;
	if (bank ==0x20) *EMU->multiCHRSize =(chrAND +1) <<10;
	if (bit4pp) {
		relative >>=1;
		chrMask >>=1;
		AND >>=1;
		OR >>=1;
		multiCHRStart <<=1;
		if (bank ==0x20) *EMU->multiCHRSize <<=1;
	} else
		base =chrData;

	if (extended) {
		multiCHRStart <<=3;
		if (bank ==0x20) *EMU->multiCHRSize <<=3;
		EMU->SetCHR_Ptr1(bank |0x0 ^flip, &base[(((((reg2000[0x16] &~1)&chrAND |chrOR) <<3 |EVA |VA21 <<11) &AND |OR) +relative) <<10 &chrMask], FALSE);
		EMU->SetCHR_Ptr1(bank |0x1 ^flip, &base[(((((reg2000[0x16] | 1)&chrAND |chrOR) <<3 |EVA |VA21 <<11) &AND |OR) +relative) <<10 &chrMask], FALSE);
		EMU->SetCHR_Ptr1(bank |0x2 ^flip, &base[(((((reg2000[0x17] &~1)&chrAND |chrOR) <<3 |EVA |VA21 <<11) &AND |OR) +relative) <<10 &chrMask], FALSE);
		EMU->SetCHR_Ptr1(bank |0x3 ^flip, &base[(((((reg2000[0x17] | 1)&chrAND |chrOR) <<3 |EVA |VA21 <<11) &AND |OR) +relative) <<10 &chrMask], FALSE);
		EMU->SetCHR_Ptr1(bank |0x4 ^flip, &base[(((( reg2000[0x12]     &chrAND |chrOR) <<3 |EVA |VA21 <<11) &AND |OR) +relative) <<10 &chrMask], FALSE);
		EMU->SetCHR_Ptr1(bank |0x5 ^flip, &base[(((( reg2000[0x13]     &chrAND |chrOR) <<3 |EVA |VA21 <<11) &AND |OR) +relative) <<10 &chrMask], FALSE);
		EMU->SetCHR_Ptr1(bank |0x6 ^flip, &base[(((( reg2000[0x14]     &chrAND |chrOR) <<3 |EVA |VA21 <<11) &AND |OR) +relative) <<10 &chrMask], FALSE);
		EMU->SetCHR_Ptr1(bank |0x7 ^flip, &base[(((( reg2000[0x15]     &chrAND |chrOR) <<3 |EVA |VA21 <<11) &AND |OR) +relative) <<10 &chrMask], FALSE);
	} else {
		chrOR |=VA18 <<8;
		multiCHRStart |=VA18 <<(bit4pp? 9: 8);
		EMU->SetCHR_Ptr1(bank |0x0 ^flip, &base[ ((((reg2000[0x16] &~1)&chrAND |chrOR           |VA21 <<11) &AND |OR) +relative) <<10 &chrMask], FALSE);
		EMU->SetCHR_Ptr1(bank |0x1 ^flip, &base[ ((((reg2000[0x16] | 1)&chrAND |chrOR           |VA21 <<11) &AND |OR) +relative) <<10 &chrMask], FALSE);
		EMU->SetCHR_Ptr1(bank |0x2 ^flip, &base[ ((((reg2000[0x17] &~1)&chrAND |chrOR           |VA21 <<11) &AND |OR) +relative) <<10 &chrMask], FALSE);
		EMU->SetCHR_Ptr1(bank |0x3 ^flip, &base[ ((((reg2000[0x17] | 1)&chrAND |chrOR           |VA21 <<11) &AND |OR) +relative) <<10 &chrMask], FALSE);
		EMU->SetCHR_Ptr1(bank |0x4 ^flip, &base[ ((( reg2000[0x12]     &chrAND |chrOR           |VA21 <<11) &AND |OR) +relative) <<10 &chrMask], FALSE);
		EMU->SetCHR_Ptr1(bank |0x5 ^flip, &base[ ((( reg2000[0x13]     &chrAND |chrOR           |VA21 <<11) &AND |OR) +relative) <<10 &chrMask], FALSE);
		EMU->SetCHR_Ptr1(bank |0x6 ^flip, &base[ ((( reg2000[0x14]     &chrAND |chrOR           |VA21 <<11) &AND |OR) +relative) <<10 &chrMask], FALSE);
		EMU->SetCHR_Ptr1(bank |0x7 ^flip, &base[ ((( reg2000[0x15]     &chrAND |chrOR           |VA21 <<11) &AND |OR) +relative) <<10 &chrMask], FALSE);
	}
	multiCHRStart |=VA21 <<(bit4pp? 12: 11);
	if (bank ==0x20) *EMU->multiCHRStart =ROM->PRGROMData +(((multiCHRStart <<10) +(OR <<10 | relative8K <<13)) &(ROM->PRGROMSize -1));
	if (*EMU->multiCHRStart >=*EMU->multiPRGStart &&
	    *EMU->multiCHRStart <(*EMU->multiPRGStart +*EMU->multiPRGSize)) {
		*EMU->multiCHRSize =0;
	}
}

void	syncCHR (int AND, int OR) {
	setCHR(0x00, V16BEN? chrLow16:  chrLow,  BK16EN || SP16EN, BKEXTEN || SPEXTEN, VRWB,         AND, OR); // 0000-1FFF: 2007 CHR Low
	setCHR(0x10, V16BEN? chrHigh16: chrHigh, BK16EN || SP16EN, BKEXTEN || SPEXTEN, VRWB,         AND, OR); // 4000-5FFF: 2007 CHR High
	setCHR(0x20, V16BEN? chrLow16:  chrLow,  BK16EN,           BKEXTEN           , BKPAGE? 4: 0, AND, OR); // 8000-9FFF: BG   CHR Low
	setCHR(0x28, V16BEN? chrLow16:  chrLow,            SP16EN,            SPEXTEN,            0, AND, OR); // A000-BFFF: SPR  CHR Low
	setCHR(0x30, V16BEN? chrHigh16: chrHigh, BK16EN,           BKEXTEN           , BKPAGE? 4: 0, AND, OR); // C000-BFFF: BG   CHR High
	setCHR(0x38, V16BEN? chrHigh16: chrHigh, SP16EN,                      SPEXTEN,            0, AND, OR); // E000-FFFF: SPR  CHR High
	*EMU->multiMapper =BK16EN || SP16EN? 256: 4;
	ROM->vt369bgData =(((reg2000[0x20] | reg2000[0x21] <<8) +relative8K) &0xFFF &(AND >>3) |(OR >>3)) <<13;
	ROM->vt369sprData=(((reg2000[0x22] | reg2000[0x23] <<8) +relative8K) &0xFFF &(AND >>3) |(OR >>3)) <<13;
}

void	syncMirror () {
	switch (HV) {
		case 0:	EMU->Mirror_V();  break;
		case 1:	EMU->Mirror_H();  break;
	}
}

void	MAPINT	writePPU (int bank, int addr, int val) {
	if (addr >=0x008) { // Don't sync on every 2007 write
		if (ROM->ConsoleType <=CONSOLE_VT09) addr &=~0x40; // for Dance Dance Revolution: Disney Mix
		reg2000[addr &0x0FF] =val;
		sync();
	}
	_writePPU(bank, addr, val);
}

int	MAPINT	readAPU (int bank, int addr) {
	switch(addr) {
		case 0x140: case 0x141: case 0x142: case 0x143: case 0x144: case 0x145: case 0x146: case 0x147:
		case 0x148: case 0x149: case 0x14A: case 0x14B: case 0x14C: case 0x14D: case 0x14E: case 0x14F:
		case 0x150: case 0x151: case 0x152: case 0x153: case 0x154: case 0x155: case 0x156: case 0x157:
		case 0x158: case 0x159: case 0x15A: case 0x15B:             case 0x15D: case 0x15E: case 0x15F:
			return ROM->ConsoleType ==CONSOLE_VT369? gpio[addr >>3 &3].read(addr &7): 0xFF;
		case 0x15C:
			return 0x10;
		case 0x18A:	// Various games, unknown purpose
			return 0x04;
		case 0x1B7:	// Various games, unknown purpose
			return 0x04;
		case 0x1B9:	// Various games, unknown purpose
			return 0x80;
		default:
			if (addr >=0x100 && addr <=0x10D || addr >=0x160 && addr <0x800)
				return reg4100[addr &0xFF];
			else
				return _readAPU(bank, addr);
	}
}

int	MAPINT	readAPUDebug (int bank, int addr) {
	return _readAPUDebug (bank, addr);
}

void	MAPINT	writeAPU (int bank, int addr, int val) {
	if (addr >=0x100 && addr <=0x1FF) {
		switch (addr) {
			case 0x101: reloadValue =val;
			            //reg4100[0x0B] |=0x80; // Turn TSYNEN on automatically when using VT register.
			            break;
			case 0x102: counter =0;
			            break;
			case 0x103: enableIRQ =false;
			            EMU->SetIRQ(1);
				    break;
			case 0x104: enableIRQ =true;
			            break;
			case 0x140: case 0x141: case 0x142: case 0x143: case 0x144: case 0x145: case 0x146: case 0x147:
			case 0x148: case 0x149: case 0x14A: case 0x14B: case 0x14C: case 0x14D: case 0x14E: case 0x14F:
			case 0x150: case 0x151: case 0x152: case 0x153: case 0x154: case 0x155: case 0x156: case 0x157:
			case 0x158: case 0x159: case 0x15A: case 0x15B: case 0x15C: case 0x15D: case 0x15E: case 0x15F:
			            if (ROM->ConsoleType ==CONSOLE_VT369) gpio[addr >>3 &3].write(addr &7, val);
				    break;
		}
		reg4100[addr &0x0FF] =val;
		switch (addr) {
			case 0x160:
			case 0x161: if (ROM->ConsoleType ==CONSOLE_VT369) relative8K =reg4100[0x60] | reg4100[0x61] <<8 &0xF00;
			            break;
		}
		sync();
	}
	_writeAPU(bank, addr, val);
}

void	MAPINT	writeMMC3 (int bank, int addr, int val) {
	if (FWEN)
		_writeROM(bank, addr, val);
	else
	switch (bank &6 | addr &1) {
		case 0:	writeAPU(0x4, 0x105, val &~0x20);
			break;
		case 1:	if (pointer <2)
				writePPU(0x2, 0x016 +pointer, val);
			else
			if (pointer <6)
				writePPU(0x2, 0x010 +pointer, val);
			else
				writeAPU(0x4, 0x101 +pointer, val);
			break;
		case 2:	writeAPU(0x4, 0x106, val &1); break;
		case 4: writeAPU(0x4, 0x101, val);
			//reg4100[0x0B] &=~0x80; // Turn TSYNEN off automatically when using MMC3 register. Must be done after writeAPU, which turns it on.
		        break;
		case 5: writeAPU(0x4, 0x102, val); break;
		case 6: writeAPU(0x4, 0x103, val); break;
		case 7: writeAPU(0x4, 0x104, val); break;
	}
}

// The VT369 maps the PPU's video RAM to CPU $3000-$3FFF, mirrored in the normal way.
int	MAPINT	readNT (int bank, int addr) {
	return _readNT(addr >>10 |8, addr &0x3FF);
}
void	MAPINT	writeNT (int bank, int addr, int val) {
	_writeNT(addr >>10 |8, addr &0x3FF, val);
}

void	MAPINT	load (FSync cSync) {
	sync =cSync;
	if (ROM->CHRROMSize) {
		chrData =ROM->CHRROMData;
		CHRROMSize =ROM->CHRROMSize;
	} else {
		chrData =ROM->PRGROMData;
		CHRROMSize =ROM->PRGROMSize;
	}
	// Round CHR-ROM size up to next power of two, which is necessary for correct CHR address masking
	int bits =0;
	while ((CHRROMSize -1) >>bits) bits++;
	CHRROMSize =1 <<bits;
	
	chrLow    =new uint8_t[CHRROMSize >>1];
	chrHigh   =new uint8_t[CHRROMSize >>1];
	if (ROM->ConsoleType ==CONSOLE_VT09 || ROM->ConsoleType ==CONSOLE_VT369) {
		chrLow16  =new uint8_t[CHRROMSize >>1];
		chrHigh16 =new uint8_t[CHRROMSize >>1];
	}	
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		// Split 4bpp CHR data into 2bpp halves.
		for (unsigned int i =0; i <CHRROMSize; i++) {
			int shiftedAddress =i &0xF | i >>1 &~0xF;
			if (i &0x10)
				chrHigh[shiftedAddress] =chrData[i];
			else
				chrLow[shiftedAddress] =chrData[i];
		}
		if (ROM->ConsoleType ==CONSOLE_VT09 || ROM->ConsoleType ==CONSOLE_VT369) for (unsigned int i =0; i <CHRROMSize; i++) {
			if (i &0x1)
				chrHigh16[i >>1] =chrData[i];
			else
				chrLow16[i >>1] =chrData[i];
		}
		for (auto& c: reg2000) c =0;
		for (auto& c: reg4100) c =0;
	}	
	relative8K =0;
	reg2000[0x10] =0x00; // Reset VT03 extended video modes upon reset, needed for dreamGEAR 75-in-1
	reg2000[0x12] =0x04;
	reg2000[0x13] =0x05;
	reg2000[0x14] =0x06;
	reg2000[0x15] =0x07;
	reg2000[0x16] =0x00;
	reg2000[0x17] =0x02;
	reg2000[0x18] =0x00;
	reg2000[0x1A] =0x00;
	reg4100[0x00] =0x00;
	reg4100[0x05] =0x00; // COMR6, COMR7
	reg4100[0x07] =0x00; // PQ0
	reg4100[0x08] =0x01; // PQ1
	reg4100[0x09] =0xFE; // PQ2
	reg4100[0x0A] =0x00;
	reg4100[0x0B] =0x00;
	reg4100[0x0F] =0xFF;
	reg4100[0x60] =0x00; // Reset VT369 relative bank upon reset
	reg4100[0x61] =0x00; // Reset VT369 relative bank upon reset
	counter =0;
	reloadValue =0;
	enableIRQ =false;
	pa12Filter =0;
	irqDelay =0;
	sync();
	*EMU->multiCanSave =TRUE;
	
	_writePPU =EMU->GetCPUWriteHandler(0x2);
	EMU->SetCPUWriteHandler(0x2, writePPU);

	_readAPU      =EMU->GetCPUReadHandler     (0x4);
	_readAPUDebug =EMU->GetCPUReadHandlerDebug(0x4);
	_writeAPU     =EMU->GetCPUWriteHandler    (0x4);
	EMU->SetCPUReadHandler     (0x4, readAPU);
	EMU->SetCPUReadHandlerDebug(0x4, readAPUDebug);
	EMU->SetCPUWriteHandler    (0x4, writeAPU);
	
	// MMC3 to native OneBus register translation handler
	_writeROM     =EMU->GetCPUWriteHandler(0x8);
	for (int bank =0x8; bank<=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeMMC3);

	if (ROM->ConsoleType ==CONSOLE_VT369) {
		// Turn off the sound CPU
		writeAPU(0x4, 0x162, 0x00);
		
		// Attach security devices to I/O ports
		for (auto& port: gpio) port.reset();

		// The VT369 maps the PPU's video RAM to CPU $3000-$3FFF, mirrored in the normal way.
		_readNT  =EMU->GetPPUReadHandler(0x8);
		_writeNT =EMU->GetPPUWriteHandler(0x8);
		EMU->SetCPUReadHandler     (0x3, readNT);
		EMU->SetCPUReadHandlerDebug(0x3, readNT);
		EMU->SetCPUWriteHandler    (0x3, writeNT);
	}
}

void	MAPINT	unload (void) {
	delete[] chrLow;
	delete[] chrHigh;
	chrLow  =NULL;
	chrHigh =NULL;
}

void	clockScanlineCounter(int isRendering) {
	counter =!counter? reloadValue: --counter;
	if (!counter && enableIRQ && isRendering) {
		if (ROM->ConsoleType ==CONSOLE_VT369 && reg4100[0x1C] &0x20)
			irqDelay =24;
		else
			EMU->SetIRQ(0);
	}
}

void	MAPINT	cpuCycle (void) {
	if (pa12Filter) pa12Filter--;
	if (irqDelay && !--irqDelay) EMU->SetIRQ(0);
}

void	MAPINT	ppuCycle (int addr, int scanline, int cycle, int isRendering) {
	if (ROM->ConsoleType ==CONSOLE_VT369 && reg4100[0x1C] &0x80 && scanline <=0) return;
	if (ROM->ConsoleType ==CONSOLE_VT369 && reg4100[0x1C] &0x20 && scanline ==0) return;
	if (TSYNEN || BK16EN) {
		if (scanline <242 && cycle ==(ROM->ConsoleType ==CONSOLE_VT369? 240: 256)) clockScanlineCounter(isRendering);
	} else {
		if (addr &0x1000) {
			if (!pa12Filter) clockScanlineCounter(isRendering);
			pa12Filter =3;
		}
	}
}	

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	for (auto& c: reg2000) SAVELOAD_BYTE(stateMode, offset, data, c);
	for (auto& c: reg4100) SAVELOAD_BYTE(stateMode, offset, data, c);
	SAVELOAD_BYTE(stateMode, offset, data, counter);
	SAVELOAD_BYTE(stateMode, offset, data, reloadValue);
	SAVELOAD_BOOL(stateMode, offset, data, enableIRQ);
	SAVELOAD_BYTE(stateMode, offset, data, pa12Filter);
	SAVELOAD_BYTE(stateMode, offset, data, irqDelay);
	if (stateMode ==STATE_LOAD) {
		if (ROM->ConsoleType ==CONSOLE_VT369) relative8K =reg4100[0x60] | reg4100[0x61] <<8 &0xF00;
		sync();
	}
	return offset;
}

} // namespace OneBus