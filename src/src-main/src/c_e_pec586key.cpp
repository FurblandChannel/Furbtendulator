#include "stdafx.h"
#include "Nintendulator.h"
#include "Settings.h"
#include "resource.h"
#include "Movie.h"
#include "Controllers.h"
#include "MapperInterface.h"

namespace Controllers
{
#include <pshpack1.h>
struct ExpPort_PEC586Keyboard_State
{
	unsigned char Column;
	unsigned char Row;
	unsigned char Keys[13];
	unsigned char Strobe;
};
#include <poppack.h>
int	ExpPort_PEC586Keyboard::Save (FILE *out)
{
	int clen = 0;
	unsigned short len = sizeof(*State);

	writeWord(len);
	writeArray(State, len);

	return clen;
}
int	ExpPort_PEC586Keyboard::Load (FILE *in, int version_id)
{
	int clen = 0;
	unsigned short len;

	readWord(len);
	readArraySkip(State, len, sizeof(*State));

	return clen;
}

#define DIK_BREAK		0x63
#define DIK_GRETURN		0x65

void	ExpPort_PEC586Keyboard::Frame (unsigned char mode)
{
	int row, col;
	if (mode & MOV_PLAY)
	{
		for (row = 0; row < 13; row++)
			State->Keys[row] = MovData[row];
	}
	else
	{
		const int keymap[13][8] = {
			{ DIK_ESCAPE, DIK_SPACE, DIK_LALT, DIK_CAPITAL, DIK_LCONTROL, DIK_GRAVE, DIK_TAB, DIK_LSHIFT },
			{ DIK_F6, DIK_F7, DIK_F5, DIK_F4, DIK_F8, DIK_F2, DIK_F1, DIK_F3 },
			{ DIK_EQUALS,  DIK_NUMPAD0, DIK_PERIOD, DIK_A, DIK_RETURN, DIK_1, DIK_Q, DIK_Z },
			{ 0 /*=8 */, DIK_NUMPAD3, DIK_NUMPAD6, DIK_S, DIK_NUMPAD9, DIK_2, DIK_W, DIK_X },
			{ DIK_SLASH,  DIK_NUMPAD2, DIK_NUMPAD5, DIK_D, DIK_NUMPAD8, DIK_3, DIK_E, DIK_C },
			{ DIK_BREAK,  DIK_NUMPAD1, DIK_NUMPAD4, DIK_F, DIK_NUMPAD7, DIK_4, DIK_R, DIK_V },
			{ DIK_BACK, DIK_BACKSLASH, DIK_GRETURN, DIK_G, DIK_RBRACKET, DIK_5, DIK_T, DIK_B },
			{ DIK_9, DIK_PERIOD, DIK_L, DIK_K, DIK_O, DIK_8, DIK_I, DIK_COMMA },
			{ DIK_0, DIK_SLASH, DIK_SEMICOLON, DIK_J, DIK_P, DIK_7, DIK_U, DIK_M },
			{ DIK_MINUS, DIK_MINUS, DIK_APOSTROPHE, DIK_H, DIK_LBRACKET, DIK_6, DIK_Y, DIK_N },
			{ DIK_F11, DIK_F12, DIK_F10, 0,DIK_MINUS, DIK_F9, 0,0 },
			{ DIK_UP, DIK_RIGHT, DIK_DOWN, DIK_NEXT, DIK_LEFT, DIK_MULTIPLY, DIK_SUBTRACT, DIK_ADD },
			{ DIK_INSERT, DIK_NUMPAD1, DIK_PRIOR, DIK_HOME, DIK_DELETE, DIK_END, DIK_DIVIDE, DIK_NUMLOCK },
		};

		for (row = 0; row < 13; row++) {
			State->Keys[row] = 0;
			for (col = 0; col < 8; col++)
				if (KeyState[keymap[row][col]] & 0x80)
					State->Keys[row] |= 1 << col;
		}
		// special cases
		if (KeyState[DIK_RSHIFT] & 0x80)
			State->Keys[7] |= 0x80;
		if (KeyState[DIK_RCONTROL] & 0x80)
			State->Keys[5] |= 0x80;
	}
	if (mode & MOV_RECORD)
	{
		for (row = 0; row < 13; row++)
			MovData[row] = State->Keys[row];
	}
}
unsigned char	ExpPort_PEC586Keyboard::Read1 (void)
{
	return 0;	// tape, not yet implemented
}
unsigned char	ExpPort_PEC586Keyboard::Read2 (void)
{
	unsigned char result = 0;
	if (State->Row < 13) {
		//EI.DbgOut(L"Column: %d", State->Column);
		if (State->Keys[State->Row] & (1 <<(7-State->Column))) result |=2;
		State->Column =(State->Column +1) &7;
	}
	return result;
}

unsigned char	ExpPort_PEC586Keyboard::ReadIOP (uint8_t) {
	return 0;
}

void	ExpPort_PEC586Keyboard::Write (unsigned char Val) {
	if (~State->Strobe &2 && Val &2) State->Row =0;
	if ( State->Strobe &1 &&~Val &1) State->Column =0;
	if ( State->Strobe &4 &&~Val &4) State->Row =(State->Row +1) %13;
	//EI.DbgOut(L"Row: %d", State->Row);
	State->Strobe =Val;
}

void	ExpPort_PEC586Keyboard::Config (HWND hWnd) {
	MessageBox(hWnd, _T("No configuration necessary!"), _T("Nintendulator"), MB_OK);
}

void	ExpPort_PEC586Keyboard::SetMasks (void) {
	MaskKeyboard = TRUE;
}

ExpPort_PEC586Keyboard::~ExpPort_PEC586Keyboard (void) {
	delete State;
	delete[] MovData;
}

ExpPort_PEC586Keyboard::ExpPort_PEC586Keyboard (DWORD *buttons) {
	Type = EXP_PEC586KEYBOARD;
	NumButtons = 0;
	Buttons = buttons;
	State = new ExpPort_PEC586Keyboard_State;
	MovLen = 13;
	MovData = new unsigned char[MovLen];
	ZeroMemory(MovData, MovLen);
	State->Row = 0;
	State->Column = 0;
	ZeroMemory(State->Keys, sizeof(State->Keys));
}
void ExpPort_PEC586Keyboard::CPUCycle (void) { }
} // namespace Controllers