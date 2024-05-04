#include "stdafx.h"
#include "Nintendulator.h"
#include "Settings.h"
#include "resource.h"
#include "Movie.h"
#include "Controllers.h"
#include "MapperInterface.h"
#include "PPU.h"
#include "GFX.h"

namespace Controllers {
#include <pshpack1.h>
struct ExpPort_CityPatrolman_State {
	uint8_t PosX;
	uint8_t PosY;
	uint8_t Trigger;
	uint8_t Reload;
	uint8_t Strobe;
	uint32_t timeOut;
	uint32_t shiftReg;
};
#include <poppack.h>
int	ExpPort_CityPatrolman::Save (FILE *out) {
	int clen = 0;
	unsigned short len = sizeof(*State);

	writeWord(len);
	writeArray(State, len);

	return clen;
}
int	ExpPort_CityPatrolman::Load (FILE *in, int version_id) {
	int clen = 0;
	unsigned short len;

	readWord(len);
	readArraySkip(State, len, sizeof(*State));

	return clen;
}
void	ExpPort_CityPatrolman::Frame (unsigned char mode) {
	POINT pos;
	if (mode & MOV_PLAY) {
		State->PosX = MovData[0];
		State->PosY = MovData[1];
		State->Trigger = MovData[2];
		State->Reload = MovData[3];
		GFX::SetCursorPos(State->PosX, State->PosY);
	} else {
		GFX::GetCursorPos(&pos);
		if ((pos.x >= 0) && (pos.x <= 255) && (pos.y >= 8) && (pos.y <= 239)) {
			State->PosX = pos.x -0;
			State->PosY = pos.y -8;
			CurrentCursor =&CursorCross;
		} else {
			State->PosX = State->PosY = 255;
			CurrentCursor =&CursorNormal;
		}
		State->Trigger = IsPressed(Buttons[0]);
		State->Reload = IsPressed(Buttons[1]);
	}
	if (mode & MOV_RECORD) {
		MovData[0] =State->PosX;
		MovData[1] =State->PosY;
		MovData[2] =State->Trigger;
		MovData[3] =State->Reload;
	}
}
unsigned char	ExpPort_CityPatrolman::Read1 (void) {
	unsigned char result =0;
	if (State->Strobe &2) {
		result =State->shiftReg &3;
		State->shiftReg >>=2;
	}	
	return result;
}

unsigned char	ExpPort_CityPatrolman::Read2 (void) {
	return 0;
}

unsigned char	ExpPort_CityPatrolman::ReadIOP (uint8_t) {
	return 0;
}

void	ExpPort_CityPatrolman::Write (unsigned char val) {
	if (State->timeOut >=8192) {
		State->timeOut =0;
		State->shiftReg =(State->Trigger? 0x10000: 0x00000) |
		                 (State->Reload? 0x20000: 0x00000) |
		                 (State->PosX !=255 && State->PosY !=255? 0x40000: 0x00000) |
		                 (State->PosX <<0 &0x0001) |
		                 (State->PosY <<1 &0x0002) |
		                 (State->PosX <<1 &0x0004) |
		                 (State->PosY <<2 &0x0008) |
		                 (State->PosX <<2 &0x0010) |
		                 (State->PosY <<3 &0x0020) |
		                 (State->PosX <<3 &0x0040) |
		                 (State->PosY <<4 &0x0080) |
		                 (State->PosX <<4 &0x0100) |
		                 (State->PosY <<5 &0x0200) |
		                 (State->PosX <<5 &0x0400) |
		                 (State->PosY <<6 &0x0800) |
		                 (State->PosX <<6 &0x1000) |
		                 (State->PosY <<7 &0x2000) |
		                 (State->PosX <<7 &0x4000) |
		                 (State->PosY <<8 &0x8000);
	}
	State->Strobe =val &2;
}

INT_PTR	CALLBACK	ExpPort_CityPatrolman_ConfigProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	const int dlgLists[2] = {IDC_CONT_D0, IDC_CONT_D1};
	const int dlgButtons[2] = {IDC_CONT_K0, IDC_CONT_K1};
	ExpPort *Cont;
	if (uMsg == WM_INITDIALOG) 	{
		SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
		Cont = (ExpPort *)lParam;
	} else	
		Cont = (ExpPort *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
	return ParseConfigMessages(hDlg, uMsg, wParam, lParam, 2, 0, dlgLists, dlgButtons, Cont ? Cont->Buttons : NULL);
}

void	ExpPort_CityPatrolman::Config (HWND hWnd) {
	DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_STDPORT_ZAPPER), hWnd, ExpPort_CityPatrolman_ConfigProc, (LPARAM)this);
}

void	ExpPort_CityPatrolman::SetMasks (void) {
}

ExpPort_CityPatrolman::~ExpPort_CityPatrolman (void) {
	delete State;
	delete[] MovData;
	CurrentCursor =&CursorNormal;
}

ExpPort_CityPatrolman::ExpPort_CityPatrolman (DWORD *buttons) {
	Type = EXP_CITYPATROLMAN;
	NumButtons = 2;
	Buttons = buttons;
	State = new ExpPort_CityPatrolman_State;
	MovLen = 4;
	MovData = new unsigned char[MovLen];
	ZeroMemory(MovData, MovLen);
	State->PosX = 0;
	State->PosY = 0;
	State->Trigger = 0;
	State->Reload = 0;
	State->Strobe = 0;
	State->timeOut = 0;
	State->shiftReg = 0;
	GFX::SetFrameskip(-2);	
}
void ExpPort_CityPatrolman::CPUCycle (void) { 
	State->timeOut++;
}
} // namespace Controllers