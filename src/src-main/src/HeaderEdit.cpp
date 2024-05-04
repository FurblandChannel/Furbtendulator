#include "stdafx.h"
#include "Nintendulator.h"
#include "resource.h"
#include "HeaderEdit.h"
#include "MapperInterface.h"
#include <math.h>

namespace HeaderEdit {
const TCHAR *filename = NULL;	// name of ROM being edited
unsigned char header[16];	// ROM's header data
BOOL isExtended;		// whether or not the editor is in NES 2.0 mode
BOOL Mask = FALSE;		// whether we're in UpdateDialog() or not
BOOL PRGInvalid = FALSE;
BOOL CHRInvalid = FALSE;

const TCHAR *ConsoleTypes[16] = {_T("NES/Famicom"), _T("Vs. System"), _T("Playchoice 10"), _T("Famiclone with Decimal Mode"), _T("EPSM"), _T("VT01 Red/Cyan"), _T("VT02"), _T("VT03"), _T("VT09"), _T("VT32"), _T("VT369"), _T("UM6578"), _T("Famicom Network System"), _T("N/A"), _T("N/A"), _T("N/A") };
const TCHAR *ExpansionDevices[56] = {
	_T("Unspecified"),
	_T("Standard Controllers"),
	_T("NES Four Score/Satellite"),
	_T("Famicom Four Players Adapter"),
	_T("Vs. System (1P via $4016)"),
	_T("Vs. System (1P via $4017)"),
	_T("Reserved"),
	_T("Vs. Zapper"),
	_T("Zapper"),
	_T("Two Zappers"),
	_T("Bandai Hyper Shot"),
	_T("Power Pad Side A"),
	_T("Power Pad Side B"),
	_T("Family Trainer Side A"),
	_T("Family Trainer Side B"),
	_T("Arkanoid Paddle (NES)"),
	_T("Arkanoid Paddle (Famicom)"),
	_T("Two Arkanoid Paddles plus Data Recorder"),
	_T("Konami Hyper Shot"),
	_T("Coconuts Pachinko Controller"),
	_T("Exciting Boxing Punching Bag"),
	_T("Jissen Mahjong Controller"),
	_T("Party Tap"),
	_T("Oeka Kids Tablet"),
	_T("Sunsoft Barcode Battler"),
	_T("Miracle Piano Keyboard"),
	_T("Pokkun Moguraa"),
	_T("Top Rider"),
	_T("Double-Fisted"),
	_T("Famicom 3D System"),
	_T("Doremikko Keyboard"),
	_T("R.O.B. Gyro Set"),
	_T("Famicom Data Recorder"),
	_T("ASCII Turbo File"),
	_T("IGS Storage Battle Box"),
	_T("Family BASIC Keyboard"),
	_T("Dongda PEC-586 Keyboard"),
	_T("Bit Corp. Bit-79 Keyboard"),
	_T("Subor Keyboard"),
	_T("Subor Keyboard plus 3x8-bit mouse"),
	_T("Subor Keyboard plus 24-bit mouse (1P)"),
	_T("SNES Mouse"),
	_T("Multicart"),
	_T("Two SNES Controllers"),
	_T("RacerMate Bicycle"),
	_T("U-Force"),
	_T("R.O.B. Stack-Up"),
	_T("City Patrolman Lightgun"),
	_T("Sharp C1 Cassette Interface"),
	_T("Standard Controllers (inverted directions)"),
	_T("Sudoku (Excalibur)"),
	_T("ABL Pinball"),
	_T("Golden Nugget Casino"),
	_T("Golden Keyboard"),
	_T("Subor Keyboard plus 24-bit mouse (2P)"),
	_T("Port Test Controller")
};
const TCHAR *VSFlags[16] = {_T("Normal"), _T("RBI Baseball"), _T("TKO Boxing"), _T("Super Xevious"), _T("Vs. Ice Climber Japan"), _T("Vs. Dual"), _T("Raid on Bungeling Bay"), _T("N/A"), _T("N/A"), _T("N/A"), _T("N/A"), _T("N/A"), _T("N/A"), _T("N/A"), _T("N/A"), _T("N/A") };

int	ConsoleType(void) {
	return ((header[7] &3) ==3)? header[13]: (header[7] &3);
}

void	UpdateNum (HWND hDlg, int Control, int num)
{
	BOOL success;
	int oldnum = GetDlgItemInt(hDlg, Control, &success, FALSE);
	if ((num == oldnum) && success)
		return;
	SetDlgItemInt(hDlg, Control, num, FALSE);
}

void	UpdateDialog (HWND hDlg)
{
	const int extended[] = { IDC_INES_SUBMAP, IDC_INES_PRAM, IDC_INES_PSAV, IDC_INES_CRAM, IDC_INES_CSAV, IDC_INES_NTSC, IDC_INES_PAL, IDC_INES_DUAL, IDC_INES_DENDY, IDC_INES_VSPPU, IDC_INES_VSFLAG, IDC_INES_CONSOLE, IDC_INES_INPUT, IDC_INES_MISCROM, 0 };
	int i;
	Mask = TRUE;
	if ((header[7] & 0x0C) == 0x08)
		isExtended = TRUE;
	else	
		isExtended = FALSE;

	CheckRadioButton(hDlg, IDC_INES_VER1, IDC_INES_VER2, isExtended ? IDC_INES_VER2 : IDC_INES_VER1);

	for (i = 0; extended[i] != 0; i++)
		EnableWindow(GetDlgItem(hDlg, extended[i]), isExtended);

	if (isExtended) {
		UpdateNum(hDlg, IDC_INES_MAP, ((header[6] & 0xF0) >> 4) | (header[7] & 0xF0) | ((header[8] & 0x0F) << 8));
		
		int PRGSizeHigh =header[9] &0x0F;
		size_t PRGSize;
		if (PRGSizeHigh <0xF)
			PRGSize =(header[4] | ((header[9] & 0x0F) <<8)) *16384;
		else
			PRGSize =pow(2, header[4] >>2) *((header[4] &3)*2 +1);
		if (!PRGInvalid) UpdateNum(hDlg, IDC_INES_PRG, PRGSize /1024);

		int CHRSizeHigh =header[9] >>4;
		size_t CHRSize;
		if (CHRSizeHigh <0xF)
			CHRSize =(header[5] | ((header[9] & 0xF0) <<4)) *8192;
		else
			CHRSize =pow(2, header[5] >>2) *((header[5] &3)*2 +1);		
		if (!CHRInvalid) UpdateNum(hDlg, IDC_INES_CHR, CHRSize /1024);
		
		UpdateNum(hDlg, IDC_INES_MISCROM, header[14] &3);
	}
	else
	{
		UpdateNum(hDlg, IDC_INES_MAP, ((header[6] & 0xF0) >> 4) | (header[7] & 0xF0));
		if (!PRGInvalid) UpdateNum(hDlg, IDC_INES_PRG, header[4] *16384 /1024);
		if (!CHRInvalid) UpdateNum(hDlg, IDC_INES_CHR, header[5] *8192 /1024);
		UpdateNum(hDlg, IDC_INES_MISCROM, 0);
	}

	CheckDlgButton(hDlg, IDC_INES_BATT, (header[6] & 0x02) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hDlg, IDC_INES_TRAIN, (header[6] & 0x04) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hDlg, IDC_INES_4SCR, (header[6] & 0x08) ? BST_CHECKED : BST_UNCHECKED);

	if (header[6] & 0x01)
		CheckRadioButton(hDlg, IDC_INES_HORIZ, IDC_INES_VERT, IDC_INES_VERT);
	else	
		CheckRadioButton(hDlg, IDC_INES_HORIZ, IDC_INES_VERT, IDC_INES_HORIZ);

	if (IsDlgButtonChecked(hDlg, IDC_INES_4SCR)) {
		EnableWindow(GetDlgItem(hDlg, IDC_INES_HORIZ), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_INES_VERT), FALSE);
	} else {
		EnableWindow(GetDlgItem(hDlg, IDC_INES_HORIZ), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_INES_VERT), TRUE);
	}

	if (isExtended) {
		UpdateNum(hDlg, IDC_INES_SUBMAP, ((header[8] & 0xF0) >> 4));
		SendDlgItemMessage(hDlg, IDC_INES_PRAM, CB_SETCURSEL, header[10] & 0x0F, 0);
		SendDlgItemMessage(hDlg, IDC_INES_CRAM, CB_SETCURSEL, header[11] & 0x0F, 0);
		if (header[6] & 0x02) {
			EnableWindow(GetDlgItem(hDlg, IDC_INES_PSAV), TRUE);
			EnableWindow(GetDlgItem(hDlg, IDC_INES_CSAV), TRUE);
			SendDlgItemMessage(hDlg, IDC_INES_PSAV, CB_SETCURSEL, (header[10] & 0xF0) >> 4, 0);
			SendDlgItemMessage(hDlg, IDC_INES_CSAV, CB_SETCURSEL, (header[11] & 0xF0) >> 4, 0);
		} else {
			EnableWindow(GetDlgItem(hDlg, IDC_INES_PSAV), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_INES_CSAV), FALSE);
			SendDlgItemMessage(hDlg, IDC_INES_PSAV, CB_SETCURSEL, 0, 0);
			SendDlgItemMessage(hDlg, IDC_INES_CSAV, CB_SETCURSEL, 0, 0);
		}

		switch(header[12] &0x03) {
			case 0:	CheckRadioButton(hDlg, IDC_INES_NTSC, IDC_INES_DENDY, IDC_INES_NTSC); break;
			case 1:	CheckRadioButton(hDlg, IDC_INES_NTSC, IDC_INES_DENDY, IDC_INES_PAL); break;
			case 2:	CheckRadioButton(hDlg, IDC_INES_NTSC, IDC_INES_DENDY, IDC_INES_DUAL); break;
			case 3:	CheckRadioButton(hDlg, IDC_INES_NTSC, IDC_INES_DENDY, IDC_INES_DENDY); break;
		}

		SendDlgItemMessage(hDlg, IDC_INES_CONSOLE, CB_SETCURSEL, ConsoleType(), 0);
		SendDlgItemMessage(hDlg, IDC_INES_INPUT, CB_SETCURSEL, header[15], 0);
		if (ConsoleType() ==CONSOLE_VS) {
			SendDlgItemMessage(hDlg, IDC_INES_VSPPU, CB_SETCURSEL, header[13] & 0x0F, 0);
			SendDlgItemMessage(hDlg, IDC_INES_VSFLAG, CB_SETCURSEL, (header[13] & 0xF0) >> 4, 0);
		} else {
			EnableWindow(GetDlgItem(hDlg, IDC_INES_VSPPU), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_INES_VSFLAG), FALSE);
			SendDlgItemMessage(hDlg, IDC_INES_VSPPU, CB_SETCURSEL, 0xF, 0);
			SendDlgItemMessage(hDlg, IDC_INES_VSFLAG, CB_SETCURSEL, 0xF, 0);
		}
	} else {
		SetDlgItemText(hDlg, IDC_INES_SUBMAP, _T(""));
		SendDlgItemMessage(hDlg, IDC_INES_PRAM, CB_SETCURSEL, 0xF, 0);
		SendDlgItemMessage(hDlg, IDC_INES_PSAV, CB_SETCURSEL, 0xF, 0);
		SendDlgItemMessage(hDlg, IDC_INES_CRAM, CB_SETCURSEL, 0xF, 0);
		SendDlgItemMessage(hDlg, IDC_INES_CSAV, CB_SETCURSEL, 0xF, 0);
		CheckRadioButton(hDlg, IDC_INES_NTSC, IDC_INES_DENDY, IDC_INES_DUAL);
		SendDlgItemMessage(hDlg, IDC_INES_VSPPU, CB_SETCURSEL, 0xF, 0);
		SendDlgItemMessage(hDlg, IDC_INES_VSFLAG, CB_SETCURSEL, 0xF, 0);
	}
	EnableWindow(GetDlgItem(hDlg, IDOK), (PRGInvalid || CHRInvalid)? FALSE: TRUE);
	bool RequireNES20 =false;
	if (ConsoleType() >CONSOLE_PC10) RequireNES20 =true;
	if (GetDlgItemInt(hDlg, IDC_INES_MAP, NULL, FALSE) >=256) RequireNES20 =true;
	if (GetDlgItemInt(hDlg, IDC_INES_PRG, NULL, FALSE) >4096) RequireNES20 =true;
	if (GetDlgItemInt(hDlg, IDC_INES_PRG, NULL, FALSE) %16)   RequireNES20 =true;
	if (GetDlgItemInt(hDlg, IDC_INES_CHR, NULL, FALSE) >2048) RequireNES20 =true;
	if (GetDlgItemInt(hDlg, IDC_INES_CHR, NULL, FALSE) % 8)   RequireNES20 =true;
	if (GetDlgItemInt(hDlg, IDC_INES_CHR, NULL, FALSE) % 8)   RequireNES20 =true;
	EnableWindow(GetDlgItem(hDlg, IDC_INES_VER1), (PRGInvalid || CHRInvalid || RequireNES20)? FALSE: TRUE);
	Mask = FALSE;
}

bool	CheckHeader (bool fix)
{
	bool needfix = false;
	// Check for NES 2.0
	if ((header[7] & 0x0C) == 0x08)
	{
		// If the SRAM flag is cleared, make sure the extended battery-RAM sizes are zero
		if (!(header[6] & 0x02))
		{
			if ((header[10] & 0xF0) || (header[11] & 0xF0))
			{
				needfix = true;
				if (fix)
				{
					header[10] &= 0x0F;
					header[11] &= 0x0F;
				}
			}
		}
		// If the VS flag is cleared, make sure the Vs. PPU fields are zero
		if (ConsoleType() !=CONSOLE_VS && ConsoleType() <3)
		{
			if (header[13])
			{
				needfix = true;
				if (fix)
					header[13] = 0;
			}
		}
		// Make sure no reserved bits are set in the TV system byte
		if (header[12] & 0xFC) {
			needfix = true;
			if (fix)
				header[12] &= 0x03;
		}
	}
	else
	{
		if (header[7] & 0x0C)
		{
			needfix = true;
			if (fix)
				header[7] = 0;
		}
		for (int i = 8; i < 16; i++)
		{
			if (header[i])
			{
				needfix = true;
				if (fix)
					header[i] = 0;
			}
		}
	}
	return needfix;
}

bool	SaveROM (HWND hDlg)
{
	// Remove the read-only attribute if necessary
	SetFileAttributes(filename, 0);
	FILE *ROM = _tfopen(filename, _T("r+b"));
	if (ROM == NULL)
	{
		int result = MessageBox(hDlg, _T("Unable to reopen file! Discard changes?"), _T("iNES Editor"), MB_YESNO | MB_ICONQUESTION);
		return (result == IDYES);
	}
	// ensure the header is in a consistent state - zero out all unused bits
	CheckHeader(true);
	fseek(ROM, 0, SEEK_SET);
	fwrite(header, 1, 16, ROM);
	fclose(ROM);
	return true;
}

bool	OpenROM (void)
{
	FILE *rom = _tfopen(filename, _T("rb"));
	if (!rom)
	{
		MessageBox(hMainWnd, _T("Unable to open ROM!"), _T("Nintendulator"), MB_OK | MB_ICONERROR);
		return false;
	}
	fread(header, 16, 1, rom);
	fclose(rom);
	if (memcmp(header, "NES\x1A", 4))
	{
		MessageBox(hMainWnd, _T("Selected file is not an iNES ROM image!"), _T("Nintendulator"), MB_OK | MB_ICONERROR);
		return false;
	}
	if ((header[7] & 0x0C) == 0x04)
	{
		MessageBox(hMainWnd, _T("Selected ROM appears to have been corrupted by \"DiskDude!\" - cleaning..."), _T("Nintendulator"), MB_OK | MB_ICONWARNING);
		memset(header+7, 0, 9);
	}
	else if (CheckHeader(false) && 
		(MessageBox(hMainWnd, _T("Unrecognized or inconsistent data detected in ROM header! Do you wish to clean it?"), _T("Nintendulator"), MB_YESNO | MB_ICONQUESTION) == IDYES))
	{
		CheckHeader(true);
	}
	return true;
}

INT_PTR CALLBACK	dlgProc (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	const TCHAR *RAMsizes[16] = {_T("None"), _T("128"), _T("256"), _T("512"), _T("1K"), _T("2K"), _T("4K"), _T("8K"), _T("16K"), _T("32K"), _T("64K"), _T("128K"), _T("256K"), _T("512K"), _T("1MB"), _T("N/A") };
	const TCHAR *VSPPUs[16] = {_T("RP2C03B"), _T("RP2C03G"), _T("RP2C04-0001"), _T("RP2C04-0002"), _T("RP2C04-0003"), _T("RP2C04-0004"), _T("RC2C03B"), _T("RC2C03C"), _T("RC2C05-01"), _T("RC2C05-02"), _T("RC2C05-03"), _T("RC2C05-04"), _T("RC2C05-05"), _T("N/A"), _T("N/A"), _T("N/A") };

	int i, n, validDivisor;
	switch (message)
	{
	case WM_INITDIALOG:
		SetDlgItemText(hDlg, IDC_INES_NAME, filename);
		for (i = 0; i < 16; i++) {
			SendDlgItemMessage(hDlg, IDC_INES_PRAM, CB_ADDSTRING, 0, (LPARAM)RAMsizes[i]);
			SendDlgItemMessage(hDlg, IDC_INES_PSAV, CB_ADDSTRING, 0, (LPARAM)RAMsizes[i]);
			SendDlgItemMessage(hDlg, IDC_INES_CRAM, CB_ADDSTRING, 0, (LPARAM)RAMsizes[i]);
			SendDlgItemMessage(hDlg, IDC_INES_CSAV, CB_ADDSTRING, 0, (LPARAM)RAMsizes[i]);
			SendDlgItemMessage(hDlg, IDC_INES_VSPPU, CB_ADDSTRING, 0, (LPARAM)VSPPUs[i]);
			SendDlgItemMessage(hDlg, IDC_INES_VSFLAG, CB_ADDSTRING, 0, (LPARAM)VSFlags[i]);
			SendDlgItemMessage(hDlg, IDC_INES_CONSOLE, CB_ADDSTRING, 0, (LPARAM)ConsoleTypes[i]);
		}
		for (i =0; i <56; i++) SendDlgItemMessage(hDlg, IDC_INES_INPUT, CB_ADDSTRING, 0, (LPARAM)ExpansionDevices[i]);
		UpdateDialog(hDlg);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_INES_VER1:
			if (Mask)
				break;
			header[7] &= 0xF3;
			UpdateDialog(hDlg);
			return TRUE;

		case IDC_INES_VER2:
			if (Mask)
				break;
			header[7] &= 0xF3;
			header[7] |= 0x08;
			UpdateDialog(hDlg);
			return TRUE;

		case IDC_INES_MAP:
			if (Mask)
				break;
			n = GetDlgItemInt(hDlg, IDC_INES_MAP, NULL, FALSE);
			header[6] &= 0x0F;
			header[6] |= (n & 0x00F) << 4;
			header[7] &= 0x0F;
			header[7] |= (n & 0x0F0) << 0;
			if (isExtended)
			{
				header[8] &= 0xF0;
				header[8] |= (n & 0xF00) >> 8;
			}
			UpdateDialog(hDlg);
			return TRUE;

		case IDC_INES_PRG:
			if (Mask) break;
			PRGInvalid =FALSE;
			validDivisor =16384;
			n = GetDlgItemInt(hDlg, IDC_INES_PRG, NULL, FALSE) *1024;
			header[4] = (n /16384) & 0xFF;
			if (isExtended) {
				header[9] &= 0xF0;
				header[9] |= ((n /16384) & 0xF00) >> 8;
				
				if (n >= 64*1024*1024 || n %16384) {
					header[9] |=0xF;
					int multiplier =1;
					if ((n %3) ==0) multiplier =3;
					if ((n %5) ==0) multiplier =5;
					if ((n %7) ==0) multiplier =7;
					int exponent =log2 (n /multiplier);
					header[4] =(exponent <<2) | ((multiplier -1) >>1);
					validDivisor =pow(2, exponent) *multiplier;
				}
			}
			PRGInvalid =!!(n %validDivisor);
			UpdateDialog(hDlg);
			return TRUE;

		case IDC_INES_CHR:
			if (Mask) break;
			CHRInvalid =FALSE;
			validDivisor =8192;
			n = GetDlgItemInt(hDlg, IDC_INES_CHR, NULL, FALSE) *1024;
			header[5] = (n /8192) & 0xFF;
			if (isExtended) {
				header[9] &= 0x0F;
				header[9] |= ((n /8192) & 0xF00) >> 4;
				if (n >= 32*1024*1024 || n %8192) {
					header[9] |=0xF0;
					int multiplier =1;
					if ((n %3) ==0) multiplier =3;
					if ((n %5) ==0) multiplier =5;
					if ((n %7) ==0) multiplier =7;
					int exponent =log2 (n /multiplier);
					header[5] =(exponent <<2) | ((multiplier -1) >>1);
					validDivisor =pow(2, exponent) *multiplier;
				}
			}
			CHRInvalid =!!(n %validDivisor);
			UpdateDialog(hDlg);
			return TRUE;

		case IDC_INES_HORIZ:
			if (Mask)
				break;
			header[6] &= ~0x01;
			UpdateDialog(hDlg);
			return TRUE;

		case IDC_INES_VERT:
			if (Mask)
				break;
			header[6] |= 0x01;
			UpdateDialog(hDlg);
			return TRUE;

		case IDC_INES_4SCR:
			if (Mask)
				break;
			if (IsDlgButtonChecked(hDlg, IDC_INES_4SCR))
				header[6] |= 0x08;
			else	header[6] &= ~0x08;
			UpdateDialog(hDlg);
			return TRUE;

		case IDC_INES_BATT:
			if (Mask)
				break;
			if (IsDlgButtonChecked(hDlg, IDC_INES_BATT))
				header[6] |= 0x02;
			else	header[6] &= ~0x02;
			UpdateDialog(hDlg);
			return TRUE;

		case IDC_INES_TRAIN:
			if (Mask)
				break;
			if (IsDlgButtonChecked(hDlg, IDC_INES_TRAIN))
				header[6] |= 0x04;
			else	header[6] &= ~0x04;
			UpdateDialog(hDlg);
			return TRUE;

		case IDC_INES_MISCROM:
			if (Mask)
				break;
			n = GetDlgItemInt(hDlg, IDC_INES_MISCROM, NULL, FALSE) &3;
			header[14] =header[14] &~3 |n;
			UpdateDialog(hDlg);
			return TRUE;

		case IDC_INES_SUBMAP:
			if (Mask)
				break;
			n = GetDlgItemInt(hDlg, IDC_INES_SUBMAP, NULL, FALSE);
			header[8] &= 0x0F;
			header[8] |= (n & 0xF) << 4;
			UpdateDialog(hDlg);
			return TRUE;

		case IDC_INES_PRAM:
			if (Mask)
				break;
			n = (int)SendDlgItemMessage(hDlg, IDC_INES_PRAM, CB_GETCURSEL, 0, 0);
			header[10] &= 0xF0;
			header[10] |= n & 0xF;
			UpdateDialog(hDlg);
			return TRUE;

		case IDC_INES_PSAV:
			if (Mask)
				break;
			n = (int)SendDlgItemMessage(hDlg, IDC_INES_PSAV, CB_GETCURSEL, 0, 0);
			header[10] &= 0x0F;
			header[10] |= (n & 0xF) << 4;
			UpdateDialog(hDlg);
			return TRUE;

		case IDC_INES_CRAM:
			if (Mask)
				break;
			n = (int)SendDlgItemMessage(hDlg, IDC_INES_CRAM, CB_GETCURSEL, 0, 0);
			header[11] &= 0xF0;
			header[11] |= n & 0xF;
			UpdateDialog(hDlg);
			return TRUE;

		case IDC_INES_CSAV:
			if (Mask)
				break;
			n = (int)SendDlgItemMessage(hDlg, IDC_INES_CSAV, CB_GETCURSEL, 0, 0);
			header[11] &= 0x0F;
			header[11] |= (n & 0xF) << 4;
			UpdateDialog(hDlg);
			return TRUE;

		case IDC_INES_NTSC:
			if (Mask)
				break;
			header[12] &= ~0x03;
			UpdateDialog(hDlg);
			return TRUE;

		case IDC_INES_PAL:
			if (Mask)
				break;
			header[12] &= ~0x03;
			header[12] |= 0x01;
			UpdateDialog(hDlg);
			return TRUE;

		case IDC_INES_DUAL:
			if (Mask)
				break;
			header[12] &= ~0x03;
			header[12] |= 0x02;
			UpdateDialog(hDlg);
			return TRUE;

		case IDC_INES_DENDY:
			if (Mask)
				break;
			header[12] &= ~0x03;
			header[12] |= 0x03;
			UpdateDialog(hDlg);
			return TRUE;

		case IDC_INES_CONSOLE:
			if (Mask)
				break;
			n = (int)SendDlgItemMessage(hDlg, IDC_INES_CONSOLE, CB_GETCURSEL, 0, 0);
			if (n <3) {
				header[13] =0;
				header[7] =header[7] &~3 |n;
			} else {
				header[7] |=3;
				header[13] =n;
			}
			UpdateDialog(hDlg);
			return TRUE;

		case IDC_INES_INPUT:
			if (Mask)
				break;
			n = (int)SendDlgItemMessage(hDlg, IDC_INES_INPUT, CB_GETCURSEL, 0, 0);
			header[15] =n;
			UpdateDialog(hDlg);
			return TRUE;

		case IDC_INES_VSPPU:
			if (Mask)
				break;
			n = (int)SendDlgItemMessage(hDlg, IDC_INES_VSPPU, CB_GETCURSEL, 0, 0);
			header[13] &= 0xF0;
			header[13] |= n & 0xF;
			UpdateDialog(hDlg);
			return TRUE;

		case IDC_INES_VSFLAG:
			if (Mask)
				break;
			n = (int)SendDlgItemMessage(hDlg, IDC_INES_VSFLAG, CB_GETCURSEL, 0, 0);
			header[13] &= 0x0F;
			header[13] |= (n & 0xF) << 4;
			UpdateDialog(hDlg);
			return TRUE;

		case IDOK:
			if (!SaveROM(hDlg))
				break;
			// fall through
		case IDCANCEL:
			EndDialog(hDlg, 0);
			return TRUE;
		}
	}
	return FALSE;
}

void	Open (const TCHAR *file)
{
	// not re-entrant
	if (filename)
		return;
	filename = file;
	if (OpenROM())
		DialogBox(hInst, MAKEINTRESOURCE(IDD_INESHEADER), hMainWnd, dlgProc);
	filename = NULL;
}
}