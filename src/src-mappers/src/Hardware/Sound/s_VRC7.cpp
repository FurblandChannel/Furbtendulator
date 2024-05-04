#include	"..\..\interface.h"
#include	"s_vrc7.h"
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<math.h>

#define PI 3.14159265358979323846

#include	"emu2413/emu2413.cpp"

namespace VRC7sound {
OPLLsound::OPLL *OPL = NULL;
uint8_t regAddr;

void	Load (bool useVRC7)
{
	if (OPL != NULL)
	{
		MessageBox(hWnd, _T("YM2413 already created!"), _T("VRC7"), MSGBOX_FLAGS);
		return;
	}
	else
	{
		OPL = OPLLsound::OPLL_new(3579545, 1789773, useVRC7);
		if (OPL == NULL)
		{
			MessageBox(hWnd, _T("Unable to create YM2413!"), _T("VRC7"), MSGBOX_FLAGS);
			return;
		}
		OPLLsound::OPLL_set_quality(OPL, 1);
	}
}

void	Reset (RESET_TYPE ResetType) {
}

void	Unload (void) {
	if (OPL) {
		OPLLsound::OPLL_delete(OPL);
		OPL = NULL;
	}
}

void	Write (int Addr, int Val) {
	switch (Addr & 0xF031) {
		case 0:
		case 0x9010: regAddr =Val; break;
		case 1:
		case 0x9030: OPLLsound::OPLL_writeReg(OPL, regAddr, Val); break;
	}
}

int	MAPINT	Get (int numCycles)
{
	return OPLLsound::OPLL_calc(OPL);	// currently don't use numCycles
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(mode,offset,data,OPL->vrc7_mode);
	SAVELOAD_BYTE(mode,offset,data,OPL->adr);
	SAVELOAD_LONG(mode,offset,data,OPL->out);
	SAVELOAD_LONG(mode,offset,data,OPL->realstep);
	SAVELOAD_LONG(mode,offset,data,OPL->oplltime);
	SAVELOAD_LONG(mode,offset,data,OPL->opllstep);
	SAVELOAD_LONG(mode,offset,data,OPL->prev);
	SAVELOAD_LONG(mode,offset,data,OPL->next);
	SAVELOAD_LONG(mode,offset,data,OPL->sprev[0]);
	SAVELOAD_LONG(mode,offset,data,OPL->sprev[1]);
	SAVELOAD_LONG(mode,offset,data,OPL->snext[0]);
	SAVELOAD_LONG(mode,offset,data,OPL->snext[1]);
	for (int i =0; i<0x40; i++) SAVELOAD_BYTE(mode,offset,data,OPL->reg[i]);
	for (int i =0; i<18; i++) SAVELOAD_LONG(mode,offset,data,OPL->slot_on_flag[i]);
	SAVELOAD_LONG(mode,offset,data,OPL->pm_phase);
	SAVELOAD_LONG(mode,offset,data,OPL->lfo_pm);
	SAVELOAD_LONG(mode,offset,data,OPL->am_phase);
	SAVELOAD_LONG(mode,offset,data,OPL->lfo_am);
	SAVELOAD_LONG(mode,offset,data,OPL->quality);
	SAVELOAD_LONG(mode,offset,data,OPL->noisseed);
	for (int i =0; i<9; i++) {
		SAVELOAD_LONG(mode,offset,data,OPL->patch_number[i]);
		if (mode == STATE_LOAD) {
			MOD(OPL,i)->patch = &OPL->patch[OPL->patch_number[i] * 2 + 0];
			CAR(OPL,i)->patch = &OPL->patch[OPL->patch_number[i] * 2 + 1];
		}
	}
	for (int i =0; i<9; i++) SAVELOAD_LONG(mode,offset,data,OPL->key_status[i]);
	for (int i =0; i<18; i++) {
		SAVELOAD_LONG(mode,offset,data,OPL->slot[i].patch->TL);
		SAVELOAD_LONG(mode,offset,data,OPL->slot[i].patch->FB);
		SAVELOAD_LONG(mode,offset,data,OPL->slot[i].patch->EG);
		SAVELOAD_LONG(mode,offset,data,OPL->slot[i].patch->ML);
		SAVELOAD_LONG(mode,offset,data,OPL->slot[i].patch->AR);
		SAVELOAD_LONG(mode,offset,data,OPL->slot[i].patch->DR);
		SAVELOAD_LONG(mode,offset,data,OPL->slot[i].patch->SL);
		SAVELOAD_LONG(mode,offset,data,OPL->slot[i].patch->RR);
		SAVELOAD_LONG(mode,offset,data,OPL->slot[i].patch->KR);
		SAVELOAD_LONG(mode,offset,data,OPL->slot[i].patch->KL);
		SAVELOAD_LONG(mode,offset,data,OPL->slot[i].patch->AM);
		SAVELOAD_LONG(mode,offset,data,OPL->slot[i].patch->PM);
		SAVELOAD_LONG(mode,offset,data,OPL->slot[i].patch->WF);
		SAVELOAD_LONG(mode,offset,data,OPL->slot[i].type);
		SAVELOAD_LONG(mode,offset,data,OPL->slot[i].feedback);
		SAVELOAD_LONG(mode,offset,data,OPL->slot[i].output[0]);
		SAVELOAD_LONG(mode,offset,data,OPL->slot[i].output[1]);
		SAVELOAD_LONG(mode,offset,data,OPL->slot[i].phase);
		SAVELOAD_LONG(mode,offset,data,OPL->slot[i].dphase);
		SAVELOAD_LONG(mode,offset,data,OPL->slot[i].pgout);
		SAVELOAD_LONG(mode,offset,data,OPL->slot[i].fnum);
		SAVELOAD_LONG(mode,offset,data,OPL->slot[i].block);
		SAVELOAD_LONG(mode,offset,data,OPL->slot[i].volume);
		SAVELOAD_LONG(mode,offset,data,OPL->slot[i].sustine);
		SAVELOAD_LONG(mode,offset,data,OPL->slot[i].tll);
		SAVELOAD_LONG(mode,offset,data,OPL->slot[i].rks);
		SAVELOAD_LONG(mode,offset,data,OPL->slot[i].eg_mode);
		SAVELOAD_LONG(mode,offset,data,OPL->slot[i].eg_phase);
		SAVELOAD_LONG(mode,offset,data,OPL->slot[i].eg_dphase);
		SAVELOAD_LONG(mode,offset,data,OPL->slot[i].egout);
		if (mode == STATE_LOAD) OPL->slot[i].sintbl = OPLLsound::waveform[OPL->slot[i].patch->WF];
	}
	for (int i =0; i<19*2; i++) {
		SAVELOAD_LONG(mode,offset,data,OPL->patch[i].TL);
		SAVELOAD_LONG(mode,offset,data,OPL->patch[i].FB);
		SAVELOAD_LONG(mode,offset,data,OPL->patch[i].EG);
		SAVELOAD_LONG(mode,offset,data,OPL->patch[i].ML);
		SAVELOAD_LONG(mode,offset,data,OPL->patch[i].AR);
		SAVELOAD_LONG(mode,offset,data,OPL->patch[i].DR);
		SAVELOAD_LONG(mode,offset,data,OPL->patch[i].SL);
		SAVELOAD_LONG(mode,offset,data,OPL->patch[i].RR);
		SAVELOAD_LONG(mode,offset,data,OPL->patch[i].KR);
		SAVELOAD_LONG(mode,offset,data,OPL->patch[i].KL);
		SAVELOAD_LONG(mode,offset,data,OPL->patch[i].AM);
		SAVELOAD_LONG(mode,offset,data,OPL->patch[i].PM);
		SAVELOAD_LONG(mode,offset,data,OPL->patch[i].WF);
	}
	if (mode == STATE_LOAD) OPLLsound::UpdateAfterLoad(OPL);
	SAVELOAD_LONG(mode,offset,data,OPL->patch_update[0]);
	SAVELOAD_LONG(mode,offset,data,OPL->patch_update[1]);
	SAVELOAD_LONG(mode,offset,data,OPL->mask);
	SAVELOAD_BYTE(mode,offset,data,regAddr);
	return offset;
}
} // namespace VRC7sound