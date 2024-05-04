#include "stdafx.h"
#include "Nintendulator.h"
#include "Settings.h"
#include "resource.h"
#include "Movie.h"
#include "Controllers.h"

namespace Controllers
{
#include <pshpack1.h>
struct ExpPort_Fami4Play_State
{
	unsigned char BitPtr1;
	unsigned char BitPtr2;
	unsigned char Strobe;
};
#include <poppack.h>
int	ExpPort_Fami4Play::Save (FILE *out)
{
	int clen = 0;
	unsigned short len = sizeof(*State);

	writeWord(len);
	writeArray(State, len);

	return clen;
}
int	ExpPort_Fami4Play::Load (FILE *in, int version_id)
{
	int clen = 0;
	unsigned short len;

	readWord(len);
	readArraySkip(State, len, sizeof(*State));

	return clen;
}
void	ExpPort_Fami4Play::Frame (unsigned char mode)
{
	int x, y = 0;
	if (mode & MOV_PLAY)
	{
		for (x = 0; x < FSPort3->MovLen; x++, y++)
			MovData[y] = FSPort3->MovData[x];
		for (x = 0; x < FSPort4->MovLen; x++, y++)
			MovData[y] = FSPort4->MovData[x];
	}
	FSPort3->Frame(mode);
	FSPort4->Frame(mode);
	if (mode & MOV_RECORD)
	{
		for (x = 0; x < FSPort3->MovLen; x++, y++)
			FSPort3->MovData[x] = MovData[y];
		for (x = 0; x < FSPort4->MovLen; x++, y++)
			FSPort4->MovData[x] = MovData[y];
	}
}
unsigned char	ExpPort_Fami4Play::Read1 (void)
{
	unsigned char result = 0;
	if (State->Strobe)
		State->BitPtr1 = 0;
	if (State->BitPtr1 >=8)
		result =0x02;
	else {
		result = FSPort3->Read() <<1;
		State->BitPtr1++;
	}
	return result;
}
unsigned char	ExpPort_Fami4Play::Read2 (void)
{
	unsigned char result = 0;
	if (State->Strobe)
		State->BitPtr2 = 0;
	if (State->BitPtr2 >=8)
		result =0x02;
	else {
		result = FSPort4->Read() <<1;
		State->BitPtr2++;
	}
	return result;
}

unsigned char	ExpPort_Fami4Play::ReadIOP (uint8_t) {
	return 0;
}

void	ExpPort_Fami4Play::Write (unsigned char Val)
{
	if ((State->Strobe) || (Val & 1))
	{
		State->Strobe = Val & 1;
		State->BitPtr1 = 0;
		State->BitPtr2 = 0;
		FSPort3->Write(Val);
		FSPort4->Write(Val);		
	}
}
INT_PTR	CALLBACK	ExpPort_Fami4Play_ConfigProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	switch (uMsg)
	{
	case WM_INITDIALOG:
		SendDlgItemMessage(hDlg, IDC_CONT_SPORT3, CB_RESETCONTENT, 0, 0);
		SendDlgItemMessage(hDlg, IDC_CONT_SPORT4, CB_RESETCONTENT, 0, 0);
		SendDlgItemMessage(hDlg, IDC_CONT_SPORT3, CB_ADDSTRING, 0, (LPARAM)StdPort_Mappings[STD_UNCONNECTED]);
		SendDlgItemMessage(hDlg, IDC_CONT_SPORT4, CB_ADDSTRING, 0, (LPARAM)StdPort_Mappings[STD_UNCONNECTED]);
		SendDlgItemMessage(hDlg, IDC_CONT_SPORT3, CB_ADDSTRING, 0, (LPARAM)StdPort_Mappings[STD_STDCONTROLLER]);
		SendDlgItemMessage(hDlg, IDC_CONT_SPORT4, CB_ADDSTRING, 0, (LPARAM)StdPort_Mappings[STD_STDCONTROLLER]);
		SendDlgItemMessage(hDlg, IDC_CONT_SPORT3, CB_SETCURSEL, FSPort3->Type, 0);
		SendDlgItemMessage(hDlg, IDC_CONT_SPORT4, CB_SETCURSEL, FSPort4->Type, 0);
		if (Movie::Mode)
		{
			EnableWindow(GetDlgItem(hDlg, IDC_CONT_SPORT3), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_CONT_SPORT4), FALSE);
		}
		else
		{
			EnableWindow(GetDlgItem(hDlg, IDC_CONT_SPORT3), TRUE);
			EnableWindow(GetDlgItem(hDlg, IDC_CONT_SPORT4), TRUE);
		}
		return TRUE;
	case WM_COMMAND:
		wmId    = LOWORD(wParam); 
		wmEvent = HIWORD(wParam); 
		switch (wmId)
		{
		case IDOK:
			EndDialog(hDlg, 1);
			return TRUE;
		case IDC_CONT_SPORT3:
			if (wmEvent == CBN_SELCHANGE)
			{
				SET_STDCONT(FSPort3, (STDCONT_TYPE)SendDlgItemMessage(hDlg, IDC_CONT_SPORT3, CB_GETCURSEL, 0, 0));
				return TRUE;
			}
			break;
		case IDC_CONT_SPORT4:
			if (wmEvent == CBN_SELCHANGE)
			{
				SET_STDCONT(FSPort4, (STDCONT_TYPE)SendDlgItemMessage(hDlg, IDC_CONT_SPORT4, CB_GETCURSEL, 0, 0));
				return TRUE;
			}
			break;
		case IDC_CONT_CPORT3:
			FSPort3->Config(hDlg);
			return TRUE;
		case IDC_CONT_CPORT4:
			FSPort4->Config(hDlg);
			return TRUE;
		}
		break;
	}
	return FALSE;
}
void	ExpPort_Fami4Play::Config (HWND hWnd)
{
	DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_EXPPORT_FAMI4PLAY), hWnd, ExpPort_Fami4Play_ConfigProc, (LPARAM)this);
}
void	ExpPort_Fami4Play::SetMasks (void) {
	FSPort3->SetMasks();
	FSPort4->SetMasks();	
}
ExpPort_Fami4Play::~ExpPort_Fami4Play (void)
{
	delete State;
	delete[] MovData;
}
ExpPort_Fami4Play::ExpPort_Fami4Play (DWORD *buttons)
{
	Type = EXP_FAMI4PLAY;
	NumButtons = 0;
	Buttons = buttons;
	State = new ExpPort_Fami4Play_State;
	MovLen = 2;
	MovData = new unsigned char[MovLen];
	ZeroMemory(MovData, MovLen);
	State->BitPtr1 = 0;
	State->BitPtr2 = 0;
	State->Strobe = 0;
}
void ExpPort_Fami4Play::CPUCycle (void) { }
} // namespace Controllers