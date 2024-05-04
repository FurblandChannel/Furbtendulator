#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC1.h"
#include	"..\..\Hardware\h_EEPROM_93Cx6.h"

namespace {
#include	"LH5323M1.h"
MMC1Type	mmc1Type;	
EEPROM_93Cx6*	eeprom =NULL;

uint8_t		kanjiCount;
uint8_t		kanjiBank;
uint8_t		fcnsMirroring;
uint8_t		fcnsRAMControl;
uint8_t		fcnsRAMControl2;
uint8_t		fcnsWRAM [2][4096];
uint8_t		fcnsCHRRAM [2][8][1024];
uint8_t		fcnsCPU2 [4];
uint8_t		eepromData[128];

uint16_t	timerCounter;
uint16_t	timerLatch;
bool		timerEnabled;
bool		timerRepeat;
bool		pendingTimer;

void	sync (void) {
	if (ROM->INES2_SubMapper ==5)
		EMU->SetPRG_ROM32(0x8, 0); // SEROM/SHROM/SH1ROM
	else
		MMC1::syncPRG(0x0F, MMC1::getCHRBank(0) &0x10); // SUROM and SXROM, which are the only circuit boards supporting more than 256 KiB PRG ROM, take PRG A18 from CHR A16
	
	int wramBank =0;
	if (ROM->PRGRAMSize ==16384) {
		if (ROM->INES_CHRSize)
			wramBank =~MMC1::getCHRBank(0) >>4 &1; // SZROM has CHR ROM and takes WRAM A13 from CHR A16. Bank 1 is the battery-backed bank, so invert the bit to make the battery-backed bank appear first.
		else
			wramBank =~MMC1::getCHRBank(0) >>3 &1; // SOROM has CHR RAM and takes WRAM A13 from CHR A15. Bank 1 is the battery-backed bank, so invert the bit to make the battery-backed bank appear first.
	} else		
	if (ROM->PRGRAMSize ==32768)
		wramBank =MMC1::getCHRBank(0) >>2 &3; // SXROM has CHR RAM and takes WRAM A13-A14 from CHR A14-A15.
	else
	if (mmc1Type ==MMC1Type::MMC1A)
		wramBank =MMC1::getPRGBank(0) >>3 &1; // A hypothetical MMC1A circuit board with 16 KiB WRAM takes WRAM A13 from PRG A17.
	MMC1::syncWRAM(wramBank);
	
	MMC1::syncCHR(0x1F, 0x00);
	
	if (CART_VRAM)
		EMU->Mirror_4();
	else
		MMC1::syncMirror();
}

void	sync2ME (void) {
	MMC1::syncPRG(0x0F, 0x00);
	if (fcnsRAMControl & fcnsRAMControl2 &0x01)
		for (int bank =0; bank <2; bank++) EMU->SetPRG_Ptr4(0x6 +bank, fcnsWRAM[bank], TRUE);
	else
	if (MMC1::reg[1] &0x10)
		for (int bank =0; bank <2; bank++) EMU->SetPRG_OB4(0x6 +bank);
	else
		MMC1::syncWRAM(MMC1::getCHRBank(0) >>2 &3);
	
	for (int bank =0; bank <8; bank++) EMU->SetCHR_Ptr1(bank, fcnsCHRRAM[!!(fcnsRAMControl &0x08)][bank], TRUE);
	if (fcnsMirroring &0x80)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
	
	if (eeprom) eeprom->write((MMC1::reg[0] &0x03) ==0x01, !!(MMC1::reg[1] &0x02), !!(MMC1::reg[1] &0x01) && !!(MMC1::reg[0] &0x10));
}

int	MAPINT	readFCNS (int bank, int addr) {
	int result =0xFF;
	if (addr <0x20)
		return EMU->ReadAPU(bank, addr);
	else
	switch(addr) {		
		case 0xA2: result =pendingTimer *1; pendingTimer =false; break;
		case 0xAD: result =fcnsMirroring; break;
		case 0xB0: kanjiCount =0; break;
		case 0xC0: result =fcnsRAMControl |0x80; break;
		case 0xD0: case 0xD1: case 0xD2: case 0xD3:
			result =fcnsCPU2 [addr &3];
			break;
	}
	return result;
}


void	MAPINT	writeFCNS (int bank, int addr, int val) {
	if (addr <0x20)
		EMU->WriteAPU(bank, addr, val);
	else
	switch(addr) {
		case 0xA6: timerLatch =(timerLatch &0xFF00) |val; break;
		case 0xA7: timerLatch =(timerLatch &0x00FF) |(val <<8); break;
		case 0xA8: timerRepeat =!!(val &1);
		           timerEnabled =!!(val &2);
		           if (timerEnabled)
		           	timerCounter =timerLatch;
		           else
		           	pendingTimer =false;
		           break;   
		case 0xAD: fcnsMirroring =val; sync2ME(); break;
		case 0xAE: fcnsRAMControl2 =val; sync2ME(); break;
		case 0xB0: kanjiBank =val &1; break;
		case 0xC0: fcnsRAMControl =val; sync2ME(); break;
		case 0xD0: case 0xD1: case 0xD2: case 0xD3:
			fcnsCPU2 [addr &3] =val;
			break;
	}
}

int	MAPINT	readKanjiROM (int bank, int addr) {
	kanjiCount &=31;
	return kanjiROM[kanjiBank <<17 | addr <<5 | kanjiCount++];
}

int	MAPINT	readEEPROM (int bank, int addr) {
	return !(fcnsRAMControl & fcnsRAMControl2 &0x01) && MMC1::reg[1] &0x10? (eeprom->read()*0x01 + (*EMU->OpenBus &0xFE)): EMU->ReadPRG(bank, addr);
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	mmc1Type =ROM->INES_MapperNum ==155 || ROM->INES2_SubMapper ==3? MMC1Type::MMC1A: MMC1Type::MMC1B;
	TCHAR** Description =mmc1Type ==MMC1Type::MMC1A? &MapperInfo_155.Description: &MapperInfo_001.Description;

	if (ROM->INES2_SubMapper ==6) {
		*Description = L"Nintendo 2ME";
		MMC1::load(sync2ME, mmc1Type);
		eeprom =new EEPROM_93Cx6(eepromData, 128, 16);
		ROM->ChipRAMSize =128;
		ROM->ChipRAMData =eepromData;
	} else {
		// Determine circuit board name based on ROM and RAM sizes, but only for NES 2.0
		if (ROM->INES_Version <2)
			*Description =L"Nintendo MMC1-based";
		else
		if (ROM->CHRROMSize ==0) switch(ROM->PRGRAMSize) {
			case 32768: *Description =L"Nintendo SXROM"; break;
			case 16384: *Description =L"Nintendo SOROM"; break;
			case  8192: *Description =ROM->PRGROMSize ==524'288? L"Nintendo SUROM": L"Nintendo SNROM"; break;
			default:    *Description =L"Nintendo SGROM/SMROM"; break;
		} else
		if (ROM->PRGRAMSize) switch(ROM->PRGROMSize) {
			case 65536: *Description =L"Nintendo SAROM"; break;
			case 32768: *Description =L"Nintendo SIROM"; break;
			default:    *Description =ROM->CHRROMSize >=131'072? L"Nintendo SKROM": ROM->PRGRAMSize ==16384? L"Nintendo SZROM": L"Nintendo SJROM"; break;
		} else
		if (ROM->CHRROMSize <131'072) switch(ROM->PRGROMSize) {
			case 65536: *Description =L"Nintendo SBROM"; break;
			case 32768: *Description =L"Nintendo SBROM"; break;
			default:    *Description =L"Nintendo SFROM"; break;
		} else switch(ROM->PRGROMSize) {
			case 65536: *Description =L"Nintendo SCROM"; break;
			case 32768: *Description =L"Nintendo SHROM"; break;
			default:    *Description =L"Nintendo SLROM"; break;
		}
		MMC1::load(sync, mmc1Type);
	}	
	return TRUE;
}

int	MAPINT	readSwitch (int bank, int addr) {
	return ROM->dipValue;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	MMC1::reset(resetType);
	if (ROM->dipValueSet) EMU->SetCPUReadHandler(0x1, readSwitch); // For NES Test Station
	if (ROM->INES2_SubMapper ==6) {
		EMU->SetCPUReadHandler(0x4, readFCNS);
		EMU->SetCPUReadHandler(0x5, readKanjiROM);
		EMU->SetCPUReadHandler(0x6, readEEPROM);
		EMU->SetCPUReadHandler(0x7, readEEPROM);
		EMU->SetCPUWriteHandler(0x4, writeFCNS);
		fcnsMirroring =0;
		fcnsRAMControl =0;
		fcnsRAMControl2 =0;
		fcnsCPU2[0] =fcnsCPU2[1] =fcnsCPU2[2] =fcnsCPU2[3] =0;
		kanjiCount =0;
		kanjiBank =0;
		timerCounter =0;
		timerLatch =0;
		timerEnabled =false;
		timerRepeat =false;		
		pendingTimer =false;
		sync2ME();
	}
}

void	MAPINT	unload (void) {
	if (eeprom) {
		delete eeprom;
		eeprom =NULL;
	}
}

void	MAPINT	cpuCycle() {
	MMC1::cpuCycle();
	EMU->SetIRQ(pendingTimer? 0: 1);	
	if (ROM->INES2_SubMapper !=6) return;
	
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
	
	//fcnsCPU2[3] &=~0xC0;
}

uint16_t mapperNum001 =1;
uint16_t mapperNum155 =155;
} // namespace

MapperInfo MapperInfo_001 ={
	&mapperNum001,
	_T("Nintendo SxROM (MMC1B)"),
	COMPAT_FULL,
	load,
	reset,
	unload,
	cpuCycle,
	NULL,
	MMC1::saveLoad,
	NULL,
	NULL
};
MapperInfo MapperInfo_155 ={
	&mapperNum155,
	_T("Nintendo SxROM (MMC1A)"),
	COMPAT_FULL,
	load,
	MMC1::reset,
	unload,
	cpuCycle,
	NULL,
	MMC1::saveLoad,
	NULL,
	NULL
};
