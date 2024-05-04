#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_EEPROM.h"

#define timerEnabled   !!(timerControl &0x80)
#define timerRepeat    !!(timerControl &0x40)
#define timerScanline  !!(timerControl &0x20)
#define timerPrescaler   (timerControl &0x0F)
namespace {
uint8_t		prg[8];
uint8_t		pcm;
uint8_t		irqMask;
uint8_t		irqStatus;
uint8_t		timerControl;
uint8_t		timerLatch;
uint8_t		timerValue;
uint8_t		prescaler;
int		prevScanline;

// Bandai Gamepad
uint8_t		reg4016;
uint8_t		reg4026;
EEPROM_24C02	*eeprom =NULL;

FCPURead	readAPU;
FCPURead	readAPUDebug;
FCPUWrite	writeAPU;

void	sync (void) {
	if (ROM->INES2_SubMapper ==1)
		for (int bank =0; bank <8; bank++) EMU->SetPRG_ROM4(0x8 +bank, reg4026 <<1 &0x100 |prg[bank]);
	else
		for (int bank =0; bank <8; bank++) EMU->SetPRG_ROM4(0x8 +bank, reg4016 <<7 &0x100 |prg[bank]);
	EMU->SetPRG_RAM8(0x6, 0);
	for (int i =0; i<4; i++) EMU->SetCHR_RAM8(0x20 +i*8, i);
	
	// "Go! Go! Connie" in test mode expects CHR-RAM at $5000-$7FFF. Assume that this is just a mirror of $D000-$FFFF.
	//for (int i =0; i<3; i++) EMU->SetCHR_RAM4(0x14 +i*4, i +5);
}

#define CONTROLLER_A 0x80
#define CONTROLLER_B 0x40
#define CONTROLLER_START 0x10
#define CONTROLLER_SELECT 0x20
#define CONTROLLER_UP 0x08
#define CONTROLLER_DOWN 0x04
#define CONTROLLER_LEFT 0x02
#define CONTROLLER_RIGHT 0x01
/*
$4022:	D~7654 3210
          ---------
	  C... .... 
	  +--------- 0="Go! Go! Connie-chan" something 1 (Keyboard clock)
$4033:	D~7654 3210
          ---------
	  ..K. .... 
	    +------- 0="Go! Go! Connie-chan" something 2 (Keyboard int)
	  
$4033:
$4026:	D~7654 3210
          ---------
	  ...L T..P
	     | |  +- 0="Go! Go! Connie-chan" power-off button
	     | +---- 0="Go! Go! Connie-chan" test button
	     +------ 0="City Patrolman" Light Gun connected?
*/
int	MAPINT	read4 (int bank, int addr) {
	switch(addr) {
		case 0x017:
			if (ROM->INES2_SubMapper ==2) {
				uint8_t temp =0;
				writeAPU(0x4, 0x016, 1);
				writeAPU(0x4, 0x016, 0);
				for (int bit =0; bit <8; bit++) temp =temp <<1 | readAPU(0x4, 0x016) &1;
				int result =(temp &CONTROLLER_START? 0x04:0x00) |
				            (temp &CONTROLLER_SELECT? 0x08:0x00) |
					    (temp &CONTROLLER_B? 0x10:0x00) |
					    (temp &CONTROLLER_RIGHT? 0x00:0x00) |
				            (temp &CONTROLLER_DOWN? 0x00:0x00) |
					    (temp &CONTROLLER_LEFT? 0x00:0x00) |
					    (temp &CONTROLLER_UP? 0x00:0x00)
				;
				return result ^0x00;
			} else
				return readAPU(0x4, 0x017);
		case 0x026:
			if (~reg4026 &0x80 && eeprom) return eeprom->getData()? 0x01: 0x00;

			// Bandai Gamepad reads the controller via $4026.
			// Read standard controller, then remap bits.
			if (reg4026 ==0xFF && reg4016 ==0xF9) {
				uint8_t temp =0;
				writeAPU(0x4, 0x016, 1);
				writeAPU(0x4, 0x016, 0);
				for (int bit =0; bit <8; bit++) temp =temp <<1 | readAPU(0x4, 0x016) &1;
				int result =(temp &CONTROLLER_START? 0x04:0x00);
				return result ^0xFF;
			}
			if (reg4026 ==0xFF && reg4016 ==0xFE) {
				uint8_t temp =0;
				writeAPU(0x4, 0x016, 1);
				writeAPU(0x4, 0x016, 0);
				for (int bit =0; bit <8; bit++) temp =temp <<1 | readAPU(0x4, 0x016) &1;
				int result =(temp &CONTROLLER_B? 0x04:0x00) |
				            (temp &CONTROLLER_A? 0x08:0x00) |
					    (temp &CONTROLLER_RIGHT? 0x10:0x00) |
				            (temp &CONTROLLER_DOWN? 0x20:0x00) |
					    (temp &CONTROLLER_LEFT? 0x40:0x00) |
					    (temp &CONTROLLER_UP? 0x80:0x00)
				;
				return result ^0xFF;
			} else
			if (ROM->INES2_SubMapper ==2) {
				uint8_t temp =0;
				writeAPU(0x4, 0x016, 1);
				writeAPU(0x4, 0x016, 0);
				for (int bit =0; bit <8; bit++) temp =temp <<1 | readAPU(0x4, 0x016) &1;
				int result =(temp &CONTROLLER_B? 0x00:0x00) |
				            (temp &CONTROLLER_A? 0x00:0x00) |
					    (temp &CONTROLLER_RIGHT? 0x80:0x00) |
				            (temp &CONTROLLER_DOWN? 0x04:0x00) |
					    (temp &CONTROLLER_LEFT? 0x10:0x00) |
					    (temp &CONTROLLER_UP? 0x02:0x00)
				;
				return result ^0xFF;
			} else {
				int result =0xFF;
				if (ROM->InputType ==INPUT_CITY_PATROLMAN) result &=~0x10;
				if (GetKeyState('T')) result &=~0x08;
				if (GetKeyState('P')) result &=~0x01;
				return result;
			}
		case 0x027:
			return pcm;
		case 0x032:
			return irqMask;
		case 0x033:
			return readAPU(bank, addr) | irqStatus;
		case 0x034:
			return timerControl;
		case 0x035:
			return timerLatch;
		case 0x036:
			return timerValue;
		case 0x040: case 0x041: case 0x042: case 0x043: case 0x044: case 0x045: case 0x046: case 0x047:
			return prg[addr &7];
		default:
			return readAPU(bank, addr);
	}
}

int	MAPINT	read4Debug (int bank, int addr) {
	switch(addr) {
		case 0x027:
			return pcm;
		case 0x032:
			return irqMask;
		case 0x033:
			return readAPUDebug(bank, addr) | irqStatus;
		case 0x034:
			return timerControl;
		case 0x035:
			return timerLatch;
		case 0x036:
			return timerValue;
		case 0x040: case 0x041: case 0x042: case 0x043: case 0x044: case 0x045: case 0x046: case 0x047:
			return prg[addr &7];
		default:
			return readAPUDebug(bank, addr);
	}
}

uint8_t	adpcmShift;
int	adpcmCount;

void	MAPINT	write4 (int bank, int addr, int val) {
	writeAPU(bank, addr, val);
	switch(addr) {
		case 0x016:
			if (ROM->INES2_SubMapper ==2 && ~reg4016 &1 && val &1) {
				// D0: CLK
				// D1: DAT
				adpcmShift =adpcmShift <<1 | val >>1 &1;
				adpcmCount =(adpcmCount +1) &7;
				if (adpcmCount ==0) EMU->DbgOut(L"ADPCM: %02X", adpcmShift &0xFF);
			}
			reg4016 =val;
			sync();
			break;
		case 0x026:
			adpcmShift =0;
			adpcmCount =0;
			reg4026 =val;
			if (eeprom && ~reg4026 &0x80) eeprom->setPins(false, !!(val &0x02), !!(val &0x01));
			break;
		case 0x027:
			pcm =val;
			break;
		case 0x032:
			irqMask =val;
			irqStatus &=~irqMask;
			break;
		case 0x034:
			timerControl =val;
			if (!timerEnabled) irqStatus &=~0x80; // Disabling the timer immediately clears its IRQ
			break;
		case 0x035:
			timerLatch =val;
			timerValue =0;
			irqStatus &=~0x80; // Because the timerValue is set to 0, the IRQ is immediately cleared as well
			break;
		case 0x040: case 0x041: case 0x042: case 0x043: case 0x044: case 0x045: case 0x046: case 0x047:
			prg[addr &7] =val;
			sync();
			break;
	}
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	if (ROM->INES2_PRGRAM ==0x20) eeprom =new EEPROM_24C02(0, ROM->PRGRAMData);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	for (int bank =0; bank <8; bank++) prg[bank] =bank;
	pcm =0;
	irqMask =0xFF;
	irqStatus =0x00;
	timerControl =0;
	timerLatch =0;
	timerValue =0;
	prescaler =0;
	prevScanline =0;
	reg4016 =0;
	reg4026 =0x80;
	sync();
	
	readAPU      =EMU->GetCPUReadHandler     (0x4);
	readAPUDebug =EMU->GetCPUReadHandlerDebug(0x4);
	writeAPU     =EMU->GetCPUWriteHandler    (0x4);
	
	EMU->SetCPUReadHandler     (0x4, read4);
	EMU->SetCPUReadHandlerDebug(0x4, read4Debug);
	EMU->SetCPUWriteHandler    (0x4, write4);
}

void	MAPINT	unload (void) {
	if (eeprom) {
		delete eeprom;
		eeprom =NULL;
	}
}

void	clockTimer() {
	if (timerEnabled && ++prescaler >=timerPrescaler) {
		prescaler =0;
		if (!timerValue)
			timerValue =timerLatch;
		else
		if (!--timerValue) {
			irqStatus |=0x80;
			if (!timerRepeat) timerControl &=~0x80;
		}
	}
}

void	MAPINT	cpuCycle () {
	if (!timerScanline) clockTimer();
	EMU->SetIRQ(irqStatus &~irqMask? 0: 1);
}

void	MAPINT	ppuCycle (int addr, int scanline, int cycle, int isRendering) {
	if (cycle >=280 && scanline !=prevScanline && timerScanline) {
		prevScanline =scanline;
		clockTimer();
	}
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	for (auto& c: prg) SAVELOAD_BYTE(stateMode, offset, data, c);
	SAVELOAD_BYTE(stateMode, offset, data, irqMask);
	SAVELOAD_BYTE(stateMode, offset, data, irqStatus);
	SAVELOAD_BYTE(stateMode, offset, data, timerControl);
	SAVELOAD_BYTE(stateMode, offset, data, timerLatch);
	SAVELOAD_BYTE(stateMode, offset, data, timerValue);
	SAVELOAD_BYTE(stateMode, offset, data, prescaler);
	SAVELOAD_LONG(stateMode, offset, data, prevScanline);
	if (eeprom) offset =eeprom->saveLoad(stateMode, offset, data);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

static int MAPINT mapperSound (int cycles) {
	return pcm <<7;
}

uint16_t mapperNum =405;
} // namespace

MapperInfo MapperInfo_405 = {
	&mapperNum,
	_T("UMC UM6578"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	cpuCycle,
	ppuCycle,
	saveLoad,
	mapperSound,
	NULL
};

