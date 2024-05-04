#include	"..\..\DLL\d_iNES.h"

namespace {
uint8_t		fpgaMode;
uint8_t		fpgaBank;
uint8_t		fdsDisk;
uint32_t	diskPointer;

unsigned int	dataCounter;
uint8_t		fdsControl;
bool		pendingData;
bool		inserted;
bool		going;
unsigned int	changeCount;
unsigned int	ejectCount;
int		changeState;

uint16_t	timerCounter;
uint16_t	timerLatch;
bool		timerEnabled;
bool		timerRepeat;
bool		pendingTimer;


void	sync (void) {
	uint8_t mapper =fpgaMode >>4;
	switch (mapper) {
		case 4:
			EMU->SetPRG_RAM32(0x6, 0);
			EMU->SetPRG_ROM8(0xE, fpgaBank <<5 |7);
			if (fdsControl &8)
				EMU->Mirror_H();
			else
				EMU->Mirror_V();
			break;
		default:
			EMU->SetPRG_ROM32(0x8, fpgaBank);
			EMU->Mirror_H();
			break;
	}
	EMU->SetCHR_RAM8(0x0, 0);
}

int	MAPINT	readReg (int bank, int addr) {
	int result =*EMU->OpenBus;
	int bank32KiB;
	switch(addr) {
		case 0x030:		
			result =(pendingTimer? 0x01: 0x00) | (!pendingTimer && pendingData? 0x02: 0x00);
			pendingData =false;
			pendingTimer =false;
			break;
		case 0x032:
			if (ejectCount ==0) switch(changeState) {
			case 0:
				if (changeCount <1789772 *3) break;
				changeState++;
				ejectCount =50000;
				inserted =true;
				break;
			case 1:	changeState++;
				ejectCount =50000;
				inserted =false;
				break;
			case 2:	changeState =0;
				inserted =true;
				fdsDisk ^=1;
				break;
			}		
			result =inserted? 0x00: 0x05;
			changeCount =0;
			break;
		case 0x103:
			bank32KiB =(fdsDisk -2) <<1 | diskPointer >>15 &1;
			result =ROM->PRGROMData[bank32KiB <<18 | 0x38000 | diskPointer &0x7FFF];
			break;
	}
	return result;
}
void	MAPINT	writeReg (int bank, int addr, int val) {
	switch(addr) {
		case 0x020:
			timerLatch =(timerLatch &0xFF00) |val;
			break;
		case 0x021:
			timerLatch =(timerLatch &0x00FF) |(val <<8);
			break;
		case 0x022:
			timerRepeat =!!(val &1);
			timerEnabled =!!(val &2);
			if (timerEnabled)
				timerCounter =timerLatch;
			else
				pendingTimer =false;
			break;
		case 0x025:
			fdsControl =val;
			dataCounter =0;
			break;
		case 0x100:
			diskPointer =0xFFFB;			
			changeCount =0;
			break;
		case 0x102:
			diskPointer++;
			pendingData =false;
			break;
		case 0x110:
			fdsDisk =val;
			changeCount =0;
			sync();
			break;
		case 0x700:
			fpgaMode =val;
			sync();
			break;
		case 0x701:
			fpgaBank =val;
			sync();
			break;
	};
}

void	MAPINT	reset (RESET_TYPE resetType) {
	fpgaMode =0x00;
	fpgaBank =0xFF;
	fdsDisk =0x00;
	
	fdsControl =0x00;
	dataCounter =0;
	pendingData =false;

	timerCounter =0;
	timerLatch =0;
	timerEnabled =false;
	timerRepeat =false;
	pendingTimer =false;
	
	inserted =true;
	changeCount =0;
	ejectCount =0;
	changeState =0;
	sync();

	EMU->SetCPUReadHandler(0x5, readReg);
	EMU->SetCPUWriteHandler(0x5, writeReg);	
}

void	MAPINT	cpuCycle (void) {
	if (timerEnabled) {
		if (timerCounter ==0) {
			pendingTimer =true;
			if (timerRepeat)
				timerCounter =timerLatch;
			else
				timerEnabled =false;
		} else
			--timerCounter;
	}

	dataCounter +=3;
	while (dataCounter >=448) {
		dataCounter -=448;
		pendingData =true;
	}
	EMU->SetIRQ(fdsControl &0x80 && pendingData || pendingTimer? 0: 1);
	changeCount++;
	if (ejectCount) ejectCount--;
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, fpgaMode);
	SAVELOAD_BYTE(stateMode, offset, data, fpgaBank);
	SAVELOAD_BYTE(stateMode, offset, data, fdsDisk);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =469;
} // namespace

MapperInfo MapperInfo_469 ={
	&mapperNum,
	_T("BlazePro FDS"),
	COMPAT_FULL,
	NULL,
	reset,
	NULL,
	cpuCycle,
	NULL,
	saveLoad,
	NULL,
	NULL
};