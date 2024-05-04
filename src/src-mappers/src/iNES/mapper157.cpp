#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_LZ93D50.h"
#include	"resource.h"

namespace {
EEPROM_I2C*	InternalEPROM =NULL;
EEPROM_I2C*	ExternalEPROM =NULL;
uint8_t		InternalEPROMData[256]; // The Datach-internal EEPROM is not denoted in the NES 2.0 header, hence not stored by the emulator in PRG-RAM address space.
uint8_t		ExternalEPROMLatch =0;

HWND		ConfigWindow;
uint8_t		ConfigCmd;
uint8_t		BarcodeOut;
uint32_t	BarcodeReadPos;
uint32_t	BarcodeCycleCount;
TCHAR		Barcode[256];
uint8_t		BarcodeData[256];
int		NumberOfNamedBarcodes;
struct NamedBarcode {
	TCHAR* name;
	TCHAR* code;
};
NamedBarcode*	NamedBarcodes =NULL;

#include	"datach.hpp"

void	Sync (void) {
	LZ93D50::syncPRG(0x0F, 0x00);
	EMU->SetCHR_RAM8(0x00, 0);
	LZ93D50::syncMirror();
}

int	MAPINT	Read (int Bank, int Addr) {
	int Result =0x10;
	if (InternalEPROM) Result &=InternalEPROM->getData() *0x10;
	if (ExternalEPROM) Result &=ExternalEPROM->getData() *0x10;
	Result |=BarcodeOut &0x08;
	Result |=*EMU->OpenBus &~0x18;
	return Result;
}

void	MAPINT	Write (int Bank, int Addr, int Val) {
	switch (Addr &0xF) {
		case 0x0:	ExternalEPROMLatch &=~0x20;
				ExternalEPROMLatch |=Val <<2 &0x20; // Put the external EEPROM's CLK bit into the same bit position as it was for register 0xD
				if (ExternalEPROM) ExternalEPROM->setPins(false, !!(ExternalEPROMLatch &0x20),     !!(ExternalEPROMLatch &0x40));
				break;
		case 0xD:	ExternalEPROMLatch &=~0x40;	// DAT is shared between the two EEPROMs
				ExternalEPROMLatch |=Val &0x40;
				if (ExternalEPROM) ExternalEPROM->setPins(false, !!(ExternalEPROMLatch &0x20),     !!(ExternalEPROMLatch &0x40));
				if (InternalEPROM) InternalEPROM->setPins(false, Val &0x80? true: !!(Val &0x20), Val &0x80? true: !!(Val &0x40));
				break;
		default:	LZ93D50::writeASIC(Bank, Addr, Val);
				break;
	}
}

BOOL	MAPINT	Load (void) {
	memset(InternalEPROMData, 0, sizeof(InternalEPROMData));
	ROM->ChipRAMData =InternalEPROMData;
	ROM->ChipRAMSize =256;
	iNES_SetSRAM();
	InternalEPROM =new EEPROM_24C02(0, InternalEPROMData);
	if (ROM->INES_Version <2 || (ROM->INES2_PRGRAM &0xF0) >0)
		ExternalEPROM =new EEPROM_24C01(0, ROM->PRGRAMData);
	else
		ExternalEPROM =NULL;
	LZ93D50::load(Sync, NULL, false);
	ConfigWindow =NULL;
	return TRUE;
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	if (ResetType ==RESET_HARD) {
		switch(ROM->PRGROMCRC32) {
		case 0x19E81461: NumberOfNamedBarcodes =36; NamedBarcodes =cardsDragonBallZ;  break;
		case 0x5B457641: NumberOfNamedBarcodes =39; NamedBarcodes =cardsUltramanClub; break;
		case 0x0BE0A328: NumberOfNamedBarcodes =77; NamedBarcodes =cardsSDGundamWars; break;
		default:         NumberOfNamedBarcodes = 1; NamedBarcodes =cardsDefault;      break;
		}
		MapperInfo_157.Config(CFG_WINDOW, TRUE);
	}
	LZ93D50::reset(ResetType);
	for (int i =0x6; i<=0x7; i++) EMU->SetCPUReadHandler(i, Read);
	for (int i =0x8; i<=0xF; i++) EMU->SetCPUWriteHandler(i, Write);
	Barcode[0] =0;
	BarcodeData[0] =0xFF;
	BarcodeReadPos =0;
	BarcodeOut =0;
	BarcodeCycleCount =0;
}

void	MAPINT	Unload (void) {
	if (InternalEPROM) {
		delete InternalEPROM;
		InternalEPROM =NULL;
	}
	if (ExternalEPROM) {
		delete ExternalEPROM;
		ExternalEPROM =NULL;
	}
	if (ConfigWindow) 	{
		DestroyWindow(ConfigWindow);
		ConfigWindow =NULL;
	}
}

void	MAPINT	CPUCycle (void) {
	LZ93D50::cpuCycle();
	if (++BarcodeCycleCount >=1000) {
		BarcodeCycleCount -=1000;
		if (BarcodeData[BarcodeReadPos] ==0xFF)
			BarcodeOut =0;
		else 
			BarcodeOut =(BarcodeData[BarcodeReadPos++] ^ 1) <<3;
	}
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	offset =LZ93D50::saveLoad(mode, offset, data);
	
	for (int i =0; i <256; i++) SAVELOAD_BYTE(mode, offset, data, BarcodeData[i]);
	SAVELOAD_LONG(mode, offset, data, BarcodeReadPos);
	SAVELOAD_LONG(mode, offset, data, BarcodeCycleCount);
	SAVELOAD_BYTE(mode, offset, data, BarcodeOut);
	
	if (InternalEPROM) offset =InternalEPROM->saveLoad(mode, offset, data);
	if (InternalEPROM) for (int i =0; i <256; i++) SAVELOAD_BYTE(mode, offset, data, InternalEPROMData[i]);
	if (ExternalEPROM) SAVELOAD_BYTE(mode, offset, data, ExternalEPROMLatch);
	if (InternalEPROM) offset =ExternalEPROM->saveLoad(mode, offset, data);
	return offset;
}

int	NewBarcode (const TCHAR *rcode) {
	int prefix_parity_type[10][6] ={
		{ 0, 0, 0, 0, 0, 0 }, { 0, 0, 1, 0, 1, 1 }, { 0, 0, 1, 1, 0, 1 }, { 0, 0, 1, 1, 1, 0 },
		{ 0, 1, 0, 0, 1, 1 }, { 0, 1, 1, 0, 0, 1 }, { 0, 1, 1, 1, 0, 0 }, { 0, 1, 0, 1, 0, 1 },
		{ 0, 1, 0, 1, 1, 0 }, { 0, 1, 1, 0, 1, 0 }
	};
	int data_left_odd[10][7] ={
		{ 0, 0, 0, 1, 1, 0, 1 }, { 0, 0, 1, 1, 0, 0, 1 }, { 0, 0, 1, 0, 0, 1, 1 }, { 0, 1, 1, 1, 1, 0, 1 },
		{ 0, 1, 0, 0, 0, 1, 1 }, { 0, 1, 1, 0, 0, 0, 1 }, { 0, 1, 0, 1, 1, 1, 1 }, { 0, 1, 1, 1, 0, 1, 1 },
		{ 0, 1, 1, 0, 1, 1, 1 }, { 0, 0, 0, 1, 0, 1, 1 }
	};
	int data_left_even[10][7] ={
		{ 0, 1, 0, 0, 1, 1, 1 }, { 0, 1, 1, 0, 0, 1, 1 }, { 0, 0, 1, 1, 0, 1, 1 }, { 0, 1, 0, 0, 0, 0, 1 },
		{ 0, 0, 1, 1, 1, 0, 1 }, { 0, 1, 1, 1, 0, 0, 1 }, { 0, 0, 0, 0, 1, 0, 1 }, { 0, 0, 1, 0, 0, 0, 1 },
		{ 0, 0, 0, 1, 0, 0, 1 }, { 0, 0, 1, 0, 1, 1, 1 }
	};
	int data_right[10][7] ={
		{ 1, 1, 1, 0, 0, 1, 0 }, { 1, 1, 0, 0, 1, 1, 0 }, { 1, 1, 0, 1, 1, 0, 0 }, { 1, 0, 0, 0, 0, 1, 0 },
		{ 1, 0, 1, 1, 1, 0, 0 }, { 1, 0, 0, 1, 1, 1, 0 }, { 1, 0, 1, 0, 0, 0, 0 }, { 1, 0, 0, 0, 1, 0, 0 },
		{ 1, 0, 0, 1, 0, 0, 0 }, { 1, 1, 1, 0, 1, 0, 0 }
	};
	uint8_t code[13 + 1];
	uint32_t tmp_p =0;
	int i, j;
	int len;

	for (i =len =0; i <13; i++) {
		if (!rcode[i]) break;
		if ((code[i] =rcode[i] - '0') > 9)
			return(0);
		len++;
	}
	if (len !=13 && len !=12 && len !=8 && len !=7) return(0);

	#define BS(x) BarcodeData[tmp_p++] =x;
	for (j =0; j <32; j++) BS(0x00);

	/* Left guard bars */
	BS(1); BS(0); BS(1);
	if (len ==13 || len ==12) {
		uint32_t csum;

		for (i =0; i <6; i++)
			if (prefix_parity_type[code[0]][i]) {
				for (j =0; j <7; j++) {
					BS(data_left_even[code[i + 1]][j]);
				}
			} else
				for (j =0; j <7; j++) {
					BS(data_left_odd[code[i + 1]][j]);
				}

		/* Center guard bars */
		BS(0); BS(1); BS(0); BS(1); BS(0);

		for (i =7; i <12; i++)
			for (j =0; j <7; j++) {
				BS(data_right[code[i]][j]);
			}
		csum =0;
		for (i =0; i <12; i++) csum +=code[i] * ((i & 1) ? 3 : 1);
		csum =(10 - (csum % 10)) % 10;
		for (j =0; j <7; j++) {
			BS(data_right[csum][j]);
		}
	} else if (len ==8 || len ==7) {
		uint32_t csum =0;

		for (i =0; i <7; i++) csum +=(i & 1) ? code[i] : (code[i] * 3);

		csum =(10 - (csum % 10)) % 10;

		for (i =0; i <4; i++)
			for (j =0; j <7; j++) {
				BS(data_left_odd[code[i]][j]);
			}


		/* Center guard bars */
		BS(0); BS(1); BS(0); BS(1); BS(0);

		for (i =4; i <7; i++)
			for (j =0; j <7; j++) {
				BS(data_right[code[i]][j]);
			}

		for (j =0; j <7; j++) {
			BS(data_right[csum][j]);
		}
	}
	/* Right guard bars */
	BS(1); BS(0); BS(1);
	for (j =0; j <32; j++) {
		BS(0x00);
	}
	BS(0xFF);
	#undef BS
	BarcodeReadPos =0;
	BarcodeOut =0x8;
	BarcodeCycleCount =0;
	return(1);
}

INT_PTR CALLBACK ConfigProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	int Selection;
	switch (message)
	{
	case WM_INITDIALOG:
		SendDlgItemMessage(hDlg, IDC_BARCODE, WM_SETTEXT, 0, (LPARAM) Barcode);
		Selection =0;
		for (int i =0; i <NumberOfNamedBarcodes; i++) {
			SendDlgItemMessage(hDlg, IDC_DATACH_CHARS, LB_ADDSTRING, 0, (LPARAM) NamedBarcodes[i].name);
			if (_tcscmp(NamedBarcodes[i].code, Barcode) ==0) Selection =i;
		}
		SendDlgItemMessage(hDlg, IDC_DATACH_CHARS, LB_SETCURSEL, Selection, 0);
		EnableWindow(GetDlgItem(hDlg, IDC_BARCODE), (Selection ==0)? TRUE: FALSE);
		return FALSE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_DATACH_CHARS:
			Selection =SendDlgItemMessage(hDlg, IDC_DATACH_CHARS, LB_GETCURSEL, 0, 0);
			EnableWindow(GetDlgItem(hDlg, IDC_BARCODE), (Selection ==0)? TRUE: FALSE);
			SendDlgItemMessage(hDlg, IDC_BARCODE, WM_SETTEXT, 0, (LPARAM) NamedBarcodes[Selection].code);
			if(HIWORD(wParam) ==LBN_DBLCLK) {
				ConfigCmd =0x80;
				SendDlgItemMessage(hDlg, IDC_BARCODE, WM_GETTEXT, sizeof(Barcode), (LPARAM) Barcode);
			}
			return TRUE;
		case IDOK:
			ConfigCmd =0x80;
			SendDlgItemMessage(hDlg, IDC_BARCODE, WM_GETTEXT, sizeof(Barcode), (LPARAM) Barcode);
			return TRUE;
		case IDCANCEL:
			ConfigWindow =NULL;
			DestroyWindow(hDlg);
			return TRUE;
		}
		break;
	}
	return FALSE;
}

unsigned char	MAPINT	Config (CFG_TYPE mode, unsigned char data) {
	switch (mode) {
	case CFG_WINDOW:
		if (data) {
			if (ConfigWindow) break;
			ConfigWindow =CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DATACH), hWnd, ConfigProc);
			SetWindowPos(ConfigWindow, hWnd, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE);
		}
		else	
			return TRUE;
		break;
	case CFG_QUERY:
		return ConfigCmd;
		break;
	case CFG_CMD:
		if (data &0x80) NewBarcode(Barcode);
		ConfigCmd =0;
		break;
	}
	return 0;
}

uint16_t MapperNum =157;
} // namespace

MapperInfo MapperInfo_157 ={
	&MapperNum,
	_T("Bandai Datach Joint ROM System"),
	COMPAT_FULL,
	Load,
	Reset,
	Unload,
	CPUCycle,
	NULL,
	SaveLoad,
	NULL,
	Config
};