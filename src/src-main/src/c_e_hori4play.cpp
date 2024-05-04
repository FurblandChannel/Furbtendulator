#include "stdafx.h"
#include "Nintendulator.h"
#include "Settings.h"
#include "resource.h"
#include "Movie.h"
#include "Controllers.h"

namespace Controllers
{
#include <pshpack1.h>
struct ExpPort_Hori4Play_State
{
	unsigned char BitPtr1;
	unsigned char BitPtr2;
	unsigned char Strobe;
};
#include <poppack.h>
int	ExpPort_Hori4Play::Save (FILE *out)
{
	int clen = 0;
	unsigned short len = sizeof(*State);

	writeWord(len);
	writeArray(State, len);

	return clen;
}
int	ExpPort_Hori4Play::Load (FILE *in, int version_id)
{
	int clen = 0;
	unsigned short len;

	readWord(len);
	readArraySkip(State, len, sizeof(*State));

	return clen;
}
void	ExpPort_Hori4Play::Frame (unsigned char mode)
{
	int x, y = 0;
	if (mode & MOV_PLAY)
	{
		for (x = 0; x < FSPort1->MovLen; x++, y++)
			MovData[y] = FSPort1->MovData[x];
		for (x = 0; x < FSPort2->MovLen; x++, y++)
			MovData[y] = FSPort2->MovData[x];
		for (x = 0; x < FSPort3->MovLen; x++, y++)
			MovData[y] = FSPort3->MovData[x];
		for (x = 0; x < FSPort4->MovLen; x++, y++)
			MovData[y] = FSPort4->MovData[x];
	}
	FSPort1->Frame(mode);
	FSPort2->Frame(mode);
	FSPort3->Frame(mode);
	FSPort4->Frame(mode);
	if (mode & MOV_RECORD)
	{
		for (x = 0; x < FSPort1->MovLen; x++, y++)
			FSPort1->MovData[x] = MovData[y];
		for (x = 0; x < FSPort2->MovLen; x++, y++)
			FSPort2->MovData[x] = MovData[y];
		for (x = 0; x < FSPort3->MovLen; x++, y++)
			FSPort3->MovData[x] = MovData[y];
		for (x = 0; x < FSPort4->MovLen; x++, y++)
			FSPort4->MovData[x] = MovData[y];
	}
}
unsigned char	ExpPort_Hori4Play::Read1 (void)
{
	unsigned char result = 0;
	if (State->Strobe)
		State->BitPtr1 = 0;
	switch (State->BitPtr1)
	{
	case  0:case  1:case  2:case  3:case  4:case  5:case  6:case  7:
		result = FSPort1->Read() <<1;
		break;
	case  8:case  9:case 10:case 11:case 12:case 13:case 14:case 15:
		result = FSPort3->Read() <<1;
		break;
	case 18:
		result = 2;
		break;
	}
	if (State->BitPtr1 == 24)
		result = 2;
	else	State->BitPtr1++;
	return result;
}
unsigned char	ExpPort_Hori4Play::Read2 (void)
{
	unsigned char result = 0;
	if (State->Strobe)
		State->BitPtr2 = 0;
	switch (State->BitPtr2)
	{
	case  0:case  1:case  2:case  3:case  4:case  5:case  6:case  7:
		result = FSPort2->Read() <<1;
		break;
	case  8:case  9:case 10:case 11:case 12:case 13:case 14:case 15:
		result = FSPort4->Read() <<1;
		break;
	case 19:
		result = 2;
		break;
	}
	if (State->BitPtr2 == 24)
		result = 2;
	else	State->BitPtr2++;
	return result;
}

unsigned char	ExpPort_Hori4Play::ReadIOP (uint8_t) {
	return 0;
}

void	ExpPort_Hori4Play::Write (unsigned char Val)
{
	if ((State->Strobe) || (Val & 1))
	{
		State->Strobe = Val & 1;
		State->BitPtr1 = 0;
		State->BitPtr2 = 0;
		FSPort1->Write(Val);
		FSPort2->Write(Val);
		FSPort3->Write(Val);
		FSPort4->Write(Val);		
	}
}
INT_PTR	CALLBACK	ExpPort_Hori4Play_ConfigProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	switch (uMsg)
	{
	case WM_INITDIALOG:
		SendDlgItemMessage(hDlg, IDC_CONT_SPORT1, CB_RESETCONTENT, 0, 0);
		SendDlgItemMessage(hDlg, IDC_CONT_SPORT2, CB_RESETCONTENT, 0, 0);
		SendDlgItemMessage(hDlg, IDC_CONT_SPORT3, CB_RESETCONTENT, 0, 0);
		SendDlgItemMessage(hDlg, IDC_CONT_SPORT4, CB_RESETCONTENT, 0, 0);
		SendDlgItemMessage(hDlg, IDC_CONT_SPORT1, CB_ADDSTRING, 0, (LPARAM)StdPort_Mappings[STD_UNCONNECTED]);
		SendDlgItemMessage(hDlg, IDC_CONT_SPORT2, CB_ADDSTRING, 0, (LPARAM)StdPort_Mappings[STD_UNCONNECTED]);
		SendDlgItemMessage(hDlg, IDC_CONT_SPORT3, CB_ADDSTRING, 0, (LPARAM)StdPort_Mappings[STD_UNCONNECTED]);
		SendDlgItemMessage(hDlg, IDC_CONT_SPORT4, CB_ADDSTRING, 0, (LPARAM)StdPort_Mappings[STD_UNCONNECTED]);
		SendDlgItemMessage(hDlg, IDC_CONT_SPORT1, CB_ADDSTRING, 0, (LPARAM)StdPort_Mappings[STD_STDCONTROLLER]);
		SendDlgItemMessage(hDlg, IDC_CONT_SPORT2, CB_ADDSTRING, 0, (LPARAM)StdPort_Mappings[STD_STDCONTROLLER]);
		SendDlgItemMessage(hDlg, IDC_CONT_SPORT3, CB_ADDSTRING, 0, (LPARAM)StdPort_Mappings[STD_STDCONTROLLER]);
		SendDlgItemMessage(hDlg, IDC_CONT_SPORT4, CB_ADDSTRING, 0, (LPARAM)StdPort_Mappings[STD_STDCONTROLLER]);
		SendDlgItemMessage(hDlg, IDC_CONT_SPORT1, CB_SETCURSEL, FSPort1->Type, 0);
		SendDlgItemMessage(hDlg, IDC_CONT_SPORT2, CB_SETCURSEL, FSPort2->Type, 0);
		SendDlgItemMessage(hDlg, IDC_CONT_SPORT3, CB_SETCURSEL, FSPort3->Type, 0);
		SendDlgItemMessage(hDlg, IDC_CONT_SPORT4, CB_SETCURSEL, FSPort4->Type, 0);
		if (Movie::Mode)
		{
			EnableWindow(GetDlgItem(hDlg, IDC_CONT_SPORT1), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_CONT_SPORT2), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_CONT_SPORT3), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_CONT_SPORT4), FALSE);
		}
		else
		{
			EnableWindow(GetDlgItem(hDlg, IDC_CONT_SPORT1), TRUE);
			EnableWindow(GetDlgItem(hDlg, IDC_CONT_SPORT2), TRUE);
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
		case IDC_CONT_SPORT1:
			if (wmEvent == CBN_SELCHANGE)
			{
				SET_STDCONT(FSPort1, (STDCONT_TYPE)SendDlgItemMessage(hDlg, IDC_CONT_SPORT1, CB_GETCURSEL, 0, 0));
				return TRUE;
			}
			break;
		case IDC_CONT_SPORT2:
			if (wmEvent == CBN_SELCHANGE)
			{
				SET_STDCONT(FSPort2, (STDCONT_TYPE)SendDlgItemMessage(hDlg, IDC_CONT_SPORT2, CB_GETCURSEL, 0, 0));
				return TRUE;
			}
			break;
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
		case IDC_CONT_CPORT1:
			FSPort1->Config(hDlg);
			return TRUE;
		case IDC_CONT_CPORT2:
			FSPort2->Config(hDlg);
			return TRUE;
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
void	ExpPort_Hori4Play::Config (HWND hWnd)
{
	DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_STDPORT_FOURSCORE), hWnd, ExpPort_Hori4Play_ConfigProc, (LPARAM)this);
}
void	ExpPort_Hori4Play::SetMasks (void) {
	FSPort1->SetMasks();
	FSPort2->SetMasks();
	FSPort3->SetMasks();
	FSPort4->SetMasks();	
}
ExpPort_Hori4Play::~ExpPort_Hori4Play (void)
{
	delete State;
	delete[] MovData;
}
ExpPort_Hori4Play::ExpPort_Hori4Play (DWORD *buttons)
{
	Type = EXP_HORI4PLAY;
	NumButtons = 0;
	Buttons = buttons;
	State = new ExpPort_Hori4Play_State;
	MovLen = 2;
	MovData = new unsigned char[MovLen];
	ZeroMemory(MovData, MovLen);
	State->BitPtr1 = 0;
	State->BitPtr2 = 0;
	State->Strobe = 0;
}
void ExpPort_Hori4Play::CPUCycle (void) { }
} // namespace Controllers