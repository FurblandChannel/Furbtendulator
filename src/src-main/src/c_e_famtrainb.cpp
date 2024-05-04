#include "stdafx.h"
#include "Nintendulator.h"
#include "Settings.h"
#include "resource.h"
#include "Movie.h"
#include "Controllers.h"

namespace Controllers
{
#include <pshpack1.h>
struct ExpPort_FamTrainerB_State
{
	unsigned short Bits;
	unsigned char Sel;
	unsigned short NewBits;
};
#include <poppack.h>
int	ExpPort_FamTrainerB::Save (FILE *out)
{
	int clen = 0;
	unsigned short len = sizeof(*State);

	writeWord(len);
	writeArray(State, len);

	return clen;
}
int	ExpPort_FamTrainerB::Load (FILE *in, int version_id)
{
	int clen = 0;
	unsigned short len;

	readWord(len);
	readArraySkip(State, len, sizeof(*State));

	return clen;
}
void	ExpPort_FamTrainerB::Frame (unsigned char mode)
{
	int i;
	if (mode & MOV_PLAY)
		State->NewBits = MovData[0] | (MovData[1] << 8);
	else
	{
		State->NewBits = 0;
		for (i = 0; i < 12; i++)
		{
			if (IsPressed(Buttons[i]))
				State->NewBits |= 1 << i;
		}
	}
	if (mode & MOV_RECORD)
	{
		MovData[0] = (unsigned char)(State->NewBits & 0xFF);
		MovData[1] = (unsigned char)(State->NewBits >> 8);
	}
}
unsigned char	ExpPort_FamTrainerB::Read1 (void)
{
	return 0;
}
unsigned char	ExpPort_FamTrainerB::Read2 (void)
{
	unsigned char result = 0;
	if (State->Sel & 0x1)
		result = (unsigned char)(State->Bits >> 8) & 0xF;
	else if (State->Sel & 0x2)
		result = (unsigned char)(State->Bits >> 4) & 0xF;
	else if (State->Sel & 0x4)
		result = (unsigned char)(State->Bits >> 0) & 0xF;
	return (result ^ 0xF) << 1;
}

unsigned char	ExpPort_FamTrainerB::ReadIOP (uint8_t) {
	return 0;
}

void	ExpPort_FamTrainerB::Write (unsigned char Val)
{
	State->Bits = State->NewBits;
	State->Sel = ~Val & 7;
}
INT_PTR	CALLBACK	ExpPort_FamTrainerB_ConfigProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	const int dlgLists[12] = {IDC_CONT_D0,IDC_CONT_D1,IDC_CONT_D2,IDC_CONT_D3,IDC_CONT_D4,IDC_CONT_D5,IDC_CONT_D6,IDC_CONT_D7,IDC_CONT_D8,IDC_CONT_D9,IDC_CONT_D10,IDC_CONT_D11};
	const int dlgButtons[12] = {IDC_CONT_K0,IDC_CONT_K1,IDC_CONT_K2,IDC_CONT_K3,IDC_CONT_K4,IDC_CONT_K5,IDC_CONT_K6,IDC_CONT_K7,IDC_CONT_K8,IDC_CONT_K9,IDC_CONT_K10,IDC_CONT_K11};
	ExpPort *Cont;
	if (uMsg == WM_INITDIALOG)
	{
		SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
		Cont = (ExpPort *)lParam;
	}
	else	Cont = (ExpPort *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
	return ParseConfigMessages(hDlg, uMsg, wParam, lParam, 12, 0, dlgLists, dlgButtons, Cont ? Cont->Buttons : NULL);
}
void	ExpPort_FamTrainerB::Config (HWND hWnd)
{
	DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_FAMILYTRAINER_B), hWnd, ExpPort_FamTrainerB_ConfigProc, (LPARAM)this);
}
void	ExpPort_FamTrainerB::SetMasks (void)
{
}
ExpPort_FamTrainerB::~ExpPort_FamTrainerB (void)
{
	delete State;
	delete[] MovData;
}
ExpPort_FamTrainerB::ExpPort_FamTrainerB (DWORD *buttons)
{
	Type = EXP_FAMTRAINERB;
	NumButtons = 12;
	Buttons = buttons;
	State = new ExpPort_FamTrainerB_State;
	MovLen = 2;
	MovData = new unsigned char[MovLen];
	ZeroMemory(MovData, MovLen);
	State->Bits = 0;
	State->Sel = 0;
	State->NewBits = 0;
}
void ExpPort_FamTrainerB::CPUCycle (void) { }
} // namespace Controllers