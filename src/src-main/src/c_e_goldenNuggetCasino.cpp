#include "stdafx.h"
#include "Nintendulator.h"
#include "Settings.h"
#include "resource.h"
#include "Movie.h"
#include "Controllers.h"
#include "MapperInterface.h"

namespace Controllers {

#include <pshpack1.h>
struct ExpPort_GoldenNuggetCasino_State {
	unsigned char Buttons[11];
	unsigned char Strobe;
	unsigned char Shift;
};
#include <poppack.h>
int	ExpPort_GoldenNuggetCasino::Save (FILE *out)
{
	int clen = 0;
	unsigned short len = sizeof(*State);

	writeWord(len);
	writeArray(State, len);

	return clen;
}
int	ExpPort_GoldenNuggetCasino::Load (FILE *in, int version_id)
{
	int clen = 0;
	unsigned short len;

	readWord(len);
	readArraySkip(State, len, sizeof(*State));

	return clen;
}
void	ExpPort_GoldenNuggetCasino::Frame (unsigned char mode) {
	if (mode & MOV_PLAY)
		for (int i =0; i <11; i++) State->Buttons[i] =MovData[i];
	else
		for (int i =0; i <11; i++) State->Buttons[i] =IsPressed(Buttons[i]);

	if (mode & MOV_RECORD)
		for (int i =0; i <11; i++) MovData[i] =State->Buttons[i];
}
unsigned char	ExpPort_GoldenNuggetCasino::Read1 (void) {	
	int result =State->Shift &1;
	State->Shift >>=1;
	return result;
}

unsigned char	ExpPort_GoldenNuggetCasino::Read2 (void) {
	return 0;
}

unsigned char	ExpPort_GoldenNuggetCasino::ReadIOP (uint8_t port) {
	return port ==3? ~(State->Buttons[3] <<0 | State->Buttons[6] <<1 | State->Buttons[5] <<2 | State->Buttons[4] <<3): 0;
}

void	ExpPort_GoldenNuggetCasino::Write (unsigned char Val) {
	if (State->Strobe &1 && ~Val &1) {
		State->Shift =(!!State->Buttons[1]?  0x01: 0x00) // 1, normally A
		            | (!!State->Buttons[2]?  0x02: 0x00) // 2, normally B
			    | (!!State->Buttons[0]?  0x08: 0x00) // START
			    | (!!State->Buttons[7]?  0x10: 0x00) // UP
			    | (!!State->Buttons[8]?  0x20: 0x00) // DOWN
			    | (!!State->Buttons[9]?  0x40: 0x00) // LEFT
			    | (!!State->Buttons[10]? 0x80: 0x00);// RIGHT
	}
	State->Strobe =Val;
}

INT_PTR	CALLBACK	ExpPort_GoldenNuggetCasino_ConfigProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	const int dlgLists[11] = {IDC_CONT_D0, IDC_CONT_D1, IDC_CONT_D2, IDC_CONT_D3, IDC_CONT_D4, IDC_CONT_D5, IDC_CONT_D6, IDC_CONT_D7, IDC_CONT_D8, IDC_CONT_D9, IDC_CONT_D10};
	const int dlgButtons[11] = {IDC_CONT_K0, IDC_CONT_K1, IDC_CONT_K2, IDC_CONT_K3, IDC_CONT_K4, IDC_CONT_K5, IDC_CONT_K6, IDC_CONT_K7, IDC_CONT_K8, IDC_CONT_K9, IDC_CONT_K10};
	ExpPort *Cont;
	if (uMsg == WM_INITDIALOG)
	{
		SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
		Cont = (ExpPort *)lParam;
	}
	else	Cont = (ExpPort *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
	return ParseConfigMessages(hDlg, uMsg, wParam, lParam, 11, 0, dlgLists, dlgButtons, Cont ? Cont->Buttons : NULL);
}
void	ExpPort_GoldenNuggetCasino::Config (HWND hWnd)
{
	DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_GOLDEN_NUGGET_CASINO), hWnd, ExpPort_GoldenNuggetCasino_ConfigProc, (LPARAM)this);
}
void	ExpPort_GoldenNuggetCasino::SetMasks (void)
{
}
ExpPort_GoldenNuggetCasino::~ExpPort_GoldenNuggetCasino (void) {
	delete State;
	delete[] MovData;
	CurrentCursor =&CursorNormal;
}
ExpPort_GoldenNuggetCasino::ExpPort_GoldenNuggetCasino (DWORD *buttons)
{
	Type = EXP_GOLDENNUGGETCASINO;
	NumButtons = 11;
	Buttons = buttons;
	State = new ExpPort_GoldenNuggetCasino_State;
	MovLen = 11;
	MovData = new unsigned char[MovLen];
	ZeroMemory(MovData, MovLen);
	for (int i =0; i <11; i++) {
		Buttons[i] =0;
		State->Buttons[i] =0;
	}
	State->Strobe =0;
	State->Shift =0;
}
void ExpPort_GoldenNuggetCasino::CPUCycle (void) { }
} // namespace Controllers