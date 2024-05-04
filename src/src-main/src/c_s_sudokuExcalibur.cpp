#include "stdafx.h"
#include <stdint.h>
#include "Nintendulator.h"
#include "Settings.h"
#include "resource.h"
#include "Movie.h"
#include "Controllers.h"
#include "MapperInterface.h"
#include "NES.h"
#include "GFX.h"

namespace Controllers {
#include <pshpack1.h>
struct StdPort_SudokuExcalibur_State {
	unsigned char Bits;
	unsigned char BitPtr;
	unsigned char Strobe;
	unsigned char NewBits;
};
struct StdPort_SudokuExcalibur2_State {
	unsigned char Bits;
	unsigned char BitPtr;
	unsigned char Strobe;
	unsigned char NewBits;
};
#include <poppack.h>

int	StdPort_SudokuExcalibur::Save (FILE *out) {
	int clen = 0;
	unsigned short len = sizeof(*State);

	writeWord(len);
	writeArray(State, len);

	return clen;
}
int	StdPort_SudokuExcalibur2::Save (FILE *out) {
	int clen = 0;
	unsigned short len = sizeof(*State);

	writeWord(len);
	writeArray(State, len);

	return clen;
}

int	StdPort_SudokuExcalibur::Load (FILE *in, int version_id) {
	int clen = 0;
	unsigned short len;

	readWord(len);
	readArraySkip(State, len, sizeof(*State));

	return clen;
}
int	StdPort_SudokuExcalibur2::Load (FILE *in, int version_id) {
	int clen = 0;
	unsigned short len;

	readWord(len);
	readArraySkip(State, len, sizeof(*State));

	return clen;
}

void	StdPort_SudokuExcalibur::Frame (unsigned char mode) {
	int i;
	if (mode & MOV_PLAY)
		State->NewBits = MovData[0];
	else {
		State->NewBits =0;
		for (i =0; i <8; i++) if (IsPressed(Buttons[i])) State->NewBits |= 1 << i;
	}
	if (mode & MOV_RECORD) MovData[0] = State->NewBits;
}
void	StdPort_SudokuExcalibur2::Frame (unsigned char mode) {
	int i;
	if (mode & MOV_PLAY)
		State->NewBits = MovData[0];
	else {
		State->NewBits =0;
		for (i =0; i <8; i++) if (IsPressed(Buttons[i +8])) State->NewBits |= 1 << i;
	}
	if (mode & MOV_RECORD) MovData[0] = State->NewBits;
}

unsigned char	StdPort_SudokuExcalibur::Read (void) {
	unsigned char result;
	if (State->Strobe) {
		State->Bits = State->NewBits;
		State->BitPtr = 0;
		result = (unsigned char)(State->Bits & 1);
	} else {
		if (State->BitPtr < 8)
			result = (unsigned char)(State->Bits >> State->BitPtr++) & 1;
		else	
			result = 1;
	}
	return result;
}
unsigned char	StdPort_SudokuExcalibur2::Read (void) {
	unsigned char result;
	if (State->Strobe) {
		State->Bits = State->NewBits;
		State->BitPtr = 0;
		result = (unsigned char)(State->Bits & 1);
	} else {
		if (State->BitPtr < 8)
			result = (unsigned char)(State->Bits >> State->BitPtr++) & 1;
		else	
			result = 1;
	}
	return result;
}

void	StdPort_SudokuExcalibur::Write (unsigned char Val) {
	if (State->Strobe || Val &1) {
		State->Strobe =Val &1;
		State->Bits =State->NewBits;
		State->BitPtr =0;
	}
}
void	StdPort_SudokuExcalibur2::Write (unsigned char Val) {
	if (State->Strobe || Val &1) {
		State->Strobe =Val &1;
		State->Bits =State->NewBits;
		State->BitPtr =0;
	}
}

INT_PTR	CALLBACK StdPort_SudokuExcalibur_ConfigProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam){
	const int dlgLists[16] = {IDC_CONT_D1,IDC_CONT_D2,IDC_CONT_D3,IDC_CONT_D4,IDC_CONT_D5,IDC_CONT_D6,IDC_CONT_D7,IDC_CONT_D8,IDC_CONT_D9,IDC_CONT_D10,IDC_CONT_D11,IDC_CONT_D12,IDC_CONT_D13,IDC_CONT_D14,IDC_CONT_D15,IDC_CONT_D16};
	const int dlgButtons[16] = {IDC_CONT_K1,IDC_CONT_K2,IDC_CONT_K3,IDC_CONT_K4,IDC_CONT_K5,IDC_CONT_K6,IDC_CONT_K7,IDC_CONT_K8,IDC_CONT_K9,IDC_CONT_K10,IDC_CONT_K11,IDC_CONT_K12,IDC_CONT_K13,IDC_CONT_K14,IDC_CONT_K15,IDC_CONT_K16};
	StdPort *Cont;
	if (uMsg == WM_INITDIALOG)
	{
		SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
		Cont = (StdPort *)lParam;
	}
	else	Cont = (StdPort *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
	return ParseConfigMessages(hDlg, uMsg, wParam, lParam, 16, 0, dlgLists, dlgButtons, Cont ? Cont->Buttons : NULL);
}
void	StdPort_SudokuExcalibur::Config (HWND hWnd) {
	DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SUDOKU_EXCALIBUR), hWnd, StdPort_SudokuExcalibur_ConfigProc, (LPARAM)this);
}
void	StdPort_SudokuExcalibur2::Config (HWND hWnd) {
	MessageBox(hMainWnd, _T("Select the Controller 1 config button to configure this two-port controller."), _T("Nintendulator"), MB_OK);
}
void	StdPort_SudokuExcalibur::SetMasks (void) {
}
void	StdPort_SudokuExcalibur2::SetMasks (void) {
}

StdPort_SudokuExcalibur::~StdPort_SudokuExcalibur (void) {
	delete State;
	delete[] MovData;
}
StdPort_SudokuExcalibur2::~StdPort_SudokuExcalibur2 (void) {
	delete State;
	delete[] MovData;
}
StdPort_SudokuExcalibur::StdPort_SudokuExcalibur (DWORD *buttons) {
	Type = STD_SUDOKU_EXCALIBUR;
	NumButtons =16;
	Buttons = buttons;
	State = new StdPort_SudokuExcalibur_State;
	MovLen =1;
	MovData =new unsigned char[MovLen];
	ZeroMemory(MovData, MovLen);
	State->Bits =0;
	State->BitPtr =0;
	State->Strobe =0;
	State->NewBits =0;
}
StdPort_SudokuExcalibur2::StdPort_SudokuExcalibur2 (DWORD *buttons) {
	Type = STD_SUDOKU_EXCALIBUR2;
	NumButtons =16;
	Buttons = buttons;
	State = new StdPort_SudokuExcalibur2_State;
	MovLen =1;
	MovData =new unsigned char[MovLen];
	ZeroMemory(MovData, MovLen);
	State->Bits =0;
	State->BitPtr =0;
	State->Strobe =0;
	State->NewBits =0;
}
} // namespace Controllers