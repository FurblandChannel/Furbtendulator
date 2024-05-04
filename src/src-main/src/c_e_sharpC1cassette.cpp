#include "stdafx.h"
#include "Nintendulator.h"
#include "Settings.h"
#include "resource.h"
#include "Movie.h"
#include "Controllers.h"
#include "States.h"
#include <stdint.h>
#include "Tape.h"

namespace Controllers {
#include <pshpack1.h>
struct ExpPort_SharpC1Cassette_State {
};
#include <poppack.h>
int	ExpPort_SharpC1Cassette::Save (FILE *out) {
	int clen = 0;
	unsigned short len = sizeof(*State);

	writeWord(len);
	writeArray(State, len);

	return clen;
}
int	ExpPort_SharpC1Cassette::Load (FILE *in, int version_id) {
	int clen = 0;
	unsigned short len;

	readWord(len);
	readArraySkip(State, len, sizeof(*State));

	return clen;
}

void	ExpPort_SharpC1Cassette::Frame (unsigned char mode) {
}

unsigned char	ExpPort_SharpC1Cassette::Read1 (void) {
	return 0;
}

unsigned char	ExpPort_SharpC1Cassette::Read2 (void) {
	return Tape::Input()? 0x00: 0x04;
}

unsigned char	ExpPort_SharpC1Cassette::ReadIOP (uint8_t) {
	return 0;
}

void	ExpPort_SharpC1Cassette::Write (unsigned char Val) {
	Tape::Output(Val &4? true: false);
}

void	ExpPort_SharpC1Cassette::Config (HWND hWnd) {
	MessageBox(hWnd, _T("No configuration necessary!"), _T("Nintendulator"), MB_OK);
}

void	ExpPort_SharpC1Cassette::SetMasks (void) {
}

ExpPort_SharpC1Cassette::~ExpPort_SharpC1Cassette (void) {
	delete State;
}

ExpPort_SharpC1Cassette::ExpPort_SharpC1Cassette (DWORD *buttons) {
	Type = EXP_SHARPC1CASSETTE;
	NumButtons = 0;
	Buttons = buttons;
	State = new ExpPort_SharpC1Cassette_State;
	MovLen = 0;
	MovData = NULL;
}
void ExpPort_SharpC1Cassette::CPUCycle (void) { 
	Tape::CPUCycle();
}
} // namespace Controllers