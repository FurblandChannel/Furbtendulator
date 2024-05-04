#include "stdafx.h"
#include "Nintendulator.h"
#include "Settings.h"
#include "resource.h"
#include "Movie.h"
#include "Controllers.h"
#include "MapperInterface.h"
#include "GFX.h"

namespace Controllers {
	
//#include <pshpack1.h>
struct StdPort_SuborMouse_State {
	unsigned long Bits;
	unsigned char BitPtr;
	unsigned char Strobe;
	int Xmov, Ymov;
	int Xdelta, Ydelta;
	unsigned char Buttons;
	unsigned char ByteNum;
};
//#include <poppack.h>

int	StdPort_SuborMouse::Save (FILE *out) {
	int clen = 0;
	unsigned short len = sizeof(*State);

	writeWord(len);
	writeArray(State, len);

	return clen;
}

int	StdPort_SuborMouse::Load (FILE *in, int version_id) {
	int clen = 0;
	unsigned short len;

	readWord(len);
	readArraySkip(State, len, sizeof(*State));

	return clen;
}

void	StdPort_SuborMouse::Frame (unsigned char mode) { 
	if (mode & MOV_PLAY) {
		State->Xdelta = (signed char)MovData[0];
		State->Ydelta = (signed char)MovData[1];
		State->Buttons = MovData[2];
	} else {
		State->Buttons = 0;
		if (IsPressed(Buttons[0])) State->Buttons |= 0x2;
		if (IsPressed(Buttons[1])) State->Buttons |= 0x1;
		int Xdelta = GetDelta(Buttons[2]);
		int Ydelta = GetDelta(Buttons[3]);
		State->Xdelta =(Xdelta >>1) | (Xdelta &1);
		State->Ydelta =(Ydelta >>1) | (Ydelta &1);
		MaskMouse =CursorOnOutput;
	}
	if (mode & MOV_RECORD) {
		MovData[0] = (unsigned char)(State->Xdelta & 0xFF);
		MovData[1] = (unsigned char)(State->Ydelta & 0xFF);
		MovData[2] = (unsigned char)(State->Buttons & 0x03);
	}
	State->Xmov +=State->Xdelta;
	State->Ymov +=State->Ydelta;
}

unsigned char	StdPort_SuborMouse::Read (void) {
	unsigned char result;
	if (State->BitPtr <24)
		result = (unsigned char)(((State->Bits << State->BitPtr++) & 0x800000) >> 23);
	else	
		result = 0x00;
	return result;
}

void	StdPort_SuborMouse::Write (unsigned char Val) {
	if ((Val &7) ==1 && (RI.InputType ==INPUT_SUBOR_KBDMOUSE24_1P || RI.InputType ==INPUT_SUBOR_KBDMOUSE24_2P)) {
		State->Bits =0;
		int amountY = State->Ymov;
		int amountX = State->Xmov;
		if (amountY <0) amountY =-amountY;
		if (amountX <0) amountX =-amountX;
		if (amountY >127) amountY =127;
		if (amountX >127) amountX =127;
		if (State->Ymov <0) {
			State->Bits |=0x080000 | amountY;
			State->Ymov +=amountY;
		} else
		if (State->Ymov >0) {
			State->Bits |=0x040000 | amountY;
			State->Ymov -=amountY;
		}

		if (State->Xmov <0) {
			State->Bits |=0x020000 | (amountX <<8);
			State->Xmov +=amountX;
		} else
		if (State->Xmov >0) {
			State->Bits |=0x010000 | (amountX <<8);
			State->Xmov -=amountX;
		}
		if (amountY >1 || amountX >1) State->Bits |=0x200000;
		State->Bits |=State->Buttons <<22;
		State->BitPtr = 0;
	} else
	if (State->Strobe && ~Val &1 && RI.InputType ==INPUT_SUBOR_KBDMOUSE8) {
		int Ymov =State->Ymov;
		int Xmov =State->Xmov;
		if (Ymov <-31) Ymov =-31;
		if (Xmov <-31) Xmov =-31;
		if (Ymov > 31) Ymov = 31;
		if (Xmov > 31) Xmov = 31;
		//if (Xmov >1 || Xmov <-1 || Ymov >1 || Ymov <-1) {
		if (Xmov || Ymov) {
			if (++State->ByteNum >2) {
				State->ByteNum =0;
			}
			switch (State->ByteNum) {
				case 0:	State->Bits  =0x01;
					State->Bits |=State->Buttons <<6;
					State->Bits |=(Ymov <0)? 0x08: 0x00;
					State->Bits |=(((Ymov <0)? -Ymov: Ymov) >>4) <<2;
					State->Bits |=(Xmov <0)? 0x20: 0x00;
					State->Bits |=(((Xmov <0)? -Xmov: Xmov) >>4) <<4;
					State->Bits |=(State->Buttons &3) <<6;
					break;
				case 1: State->Bits  =0x02;
					State->Bits |=(((Xmov <0)? -Xmov: Xmov) &0xF) <<2;
					break;
				case 2: State->Bits  =0x03;
					State->Bits |=(((Ymov <0)? -Ymov: Ymov) &0xF) <<2;
					State->Xmov -=Xmov;
					State->Ymov -=Ymov;
					break;
			}
		} else {
			State->Bits  =0x00;
			State->Bits |=(State->Buttons <<6);
		}	
		State->Bits <<=16;
		State->BitPtr = 0;
	}
	State->Strobe =Val &1;
}

INT_PTR	CALLBACK	StdPort_SuborMouse_ConfigProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	const int dlgLists[4] = {IDC_CONT_D0,IDC_CONT_D1,IDC_CONT_D2,IDC_CONT_D3};
	const int dlgButtons[4] = {IDC_CONT_K0,IDC_CONT_K1,IDC_CONT_K2,IDC_CONT_K3};
	StdPort *Cont;
	if (uMsg == WM_INITDIALOG) {
		SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
		Cont = (StdPort *)lParam;
	} else	
		Cont = (StdPort *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
	return ParseConfigMessages(hDlg, uMsg, wParam, lParam, 2, 2, dlgLists, dlgButtons, Cont ? Cont->Buttons : NULL);
}

void	StdPort_SuborMouse::Config (HWND hWnd) {
	DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_STDPORT_SNESMOUSE), hWnd, StdPort_SuborMouse_ConfigProc, (LPARAM)this);
}

void	StdPort_SuborMouse::SetMasks (void) {
	MaskMouse = ((Buttons[2] >> 16) == 1) || ((Buttons[3] >> 16) == 1);
}

StdPort_SuborMouse::~StdPort_SuborMouse (void) {
	delete State;
	delete[] MovData;
}

StdPort_SuborMouse::StdPort_SuborMouse (DWORD *buttons) {
	Type =STD_SUBORMOUSE;
	NumButtons =4;
	Buttons =buttons;
	State =new StdPort_SuborMouse_State;
	MovLen =3;
	MovData =new unsigned char[MovLen];
	ZeroMemory(MovData, MovLen);
	State->Bits = 0;
	State->BitPtr = 0;
	State->Strobe = 0;
	State->Xmov = 0;
	State->Ymov = 0;
	State->Xdelta = 0;
	State->Ydelta = 0;
	State->Buttons = 0;
	State->ByteNum = 0;
}
} // namespace Controllers