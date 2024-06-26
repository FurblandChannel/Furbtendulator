#include "stdafx.h"
#include "Nintendulator.h"
#include "Settings.h"
#include "resource.h"
#include "Movie.h"
#include "Controllers.h"
#include "Tape.h"

namespace Controllers
{
#include <pshpack1.h>
struct ExpPort_SuborKeyboard_State
{
	unsigned char Column;
	unsigned char Row;
	unsigned char Keys[13];
};
#include <poppack.h>
int	ExpPort_SuborKeyboard::Save (FILE *out)
{
	int clen = 0;
	unsigned short len = sizeof(*State);

	writeWord(len);
	writeArray(State, len);

	return clen;
}
int	ExpPort_SuborKeyboard::Load (FILE *in, int version_id)
{
	int clen = 0;
	unsigned short len;

	readWord(len);
	readArraySkip(State, len, sizeof(*State));

	return clen;
}
void	ExpPort_SuborKeyboard::Frame (unsigned char mode)
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
			{DIK_4, DIK_G, DIK_F, DIK_C, DIK_F2, DIK_E, DIK_5, DIK_V},
			{DIK_2, DIK_D, DIK_S, DIK_END, DIK_F1, DIK_W, DIK_3, DIK_X},
			{DIK_INSERT, DIK_BACKSPACE, DIK_PGDN, DIK_RIGHT, DIK_F8, DIK_PGUP, DIK_DELETE, DIK_HOME},
			{DIK_9, DIK_I, DIK_L, DIK_COMMA, DIK_F5, DIK_O, DIK_0, DIK_PERIOD},
			{DIK_RBRACKET, DIK_RETURN, DIK_UP, DIK_LEFT, DIK_F7, DIK_LBRACKET, DIK_BACKSLASH, DIK_DOWN},
			{DIK_Q, DIK_CAPITAL, DIK_Z, DIK_TAB, DIK_ESCAPE, DIK_A, DIK_1, DIK_LCONTROL},
			{DIK_7, DIK_Y, DIK_K, DIK_M, DIK_F4, DIK_U, DIK_8, DIK_J},
			{DIK_MINUS, DIK_SEMICOLON, DIK_APOSTROPHE, DIK_SLASH, DIK_F6, DIK_P, DIK_EQUALS, DIK_LSHIFT},
			{DIK_T, DIK_H, DIK_N, DIK_SPACE, DIK_F3, DIK_R, DIK_6, DIK_B},
			// these only seem to be used in a further variant
			{DIK_GRAVE, DIK_F10, DIK_F11, DIK_F12, DIK_ADD, DIK_SUBTRACT, DIK_MULTIPLY, DIK_DIVIDE},
			{DIK_SCROLL, DIK_PAUSE, DIK_GRAVE, DIK_TAB, DIK_NUMPAD6, DIK_NUMPAD7, DIK_NUMPAD8, DIK_NUMPAD9},
			//{DIK_CAPITAL, DIK_SLASH, DIK_RSHIFT, DIK_LMENU, DIK_NUMPAD0, DIK_ADD, DIK_SUBTRACT, DIK_MULTIPLY},
			{DIK_CAPITAL, DIK_SLASH, DIK_RSHIFT, DIK_LMENU, DIK_F9, DIK_ADD, DIK_SUBTRACT, DIK_MULTIPLY},
			{DIK_RMENU, DIK_APPS, DIK_RCONTROL, DIK_NUMPAD1, DIK_DIVIDE, DIK_DECIMAL, DIK_NUMLOCK, DIK_NUMPADENTER}
		};

		for (row = 0; row < 13; row++)
		{
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
unsigned char	ExpPort_SuborKeyboard::Read1 (void)
{
	return (Tape::Input() &1) <<2;
}
unsigned char	ExpPort_SuborKeyboard::Read2 (void)
{
	unsigned char result = 0;
	if (State->Row < 13)
	{
		if (State->Column)
			result = (State->Keys[State->Row] & 0xF0) >> 3;
		else	
			result = (State->Keys[State->Row] & 0x0F) << 1;
		result ^= 0x1E;
	}
	return result;
}

unsigned char	ExpPort_SuborKeyboard::ReadIOP (uint8_t) {
	return 0;
}

void	ExpPort_SuborKeyboard::Write (unsigned char Val)
{
	Tape::Output((Val &2)? true: false);
	BOOL ResetKB = Val & 1;
	BOOL SelColumn = Val & 2;
	BOOL SelectKey = Val & 4;
	if (SelectKey)
	{
		if ((State->Column) && (!SelColumn))
			State->Row++;
		State->Column = SelColumn;
		if (ResetKB)
			State->Row = 0;
	}
	else
	{
		if (Val ==0) State->Row =13; // Bomberman on Study and Game 32-in-1
	}
}
void	ExpPort_SuborKeyboard::Config (HWND hWnd)
{
	MessageBox(hWnd, _T("No configuration necessary!"), _T("Nintendulator"), MB_OK);
}
void	ExpPort_SuborKeyboard::SetMasks (void)
{
	MaskKeyboard = TRUE;
}
ExpPort_SuborKeyboard::~ExpPort_SuborKeyboard (void)
{
	delete State;
	delete[] MovData;
}
ExpPort_SuborKeyboard::ExpPort_SuborKeyboard (DWORD *buttons)
{
	Type = EXP_SUBORKEYBOARD;
	NumButtons = 0;
	Buttons = buttons;
	State = new ExpPort_SuborKeyboard_State;
	MovLen = 13;
	MovData = new unsigned char[MovLen];
	ZeroMemory(MovData, MovLen);
	State->Row = 0;
	State->Column = 0;
	ZeroMemory(State->Keys, sizeof(State->Keys));
}
void ExpPort_SuborKeyboard::CPUCycle (void) { 
	Tape::CPUCycle();
}
} // namespace Controllers