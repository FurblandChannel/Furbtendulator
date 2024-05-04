#include "stdafx.h"
#include "Nintendulator.h"
#include "Settings.h"
#include "resource.h"
#include "MapperInterface.h"
#include "NES.h"
#include "States.h"
#include "CPU.h"
#include "PPU.h"
#include "APU.h"
#include "Movie.h"
#include "Controllers.h"
#include "GFX.h"
#include "FDS.h"
#include "plugThruDevice.hpp"

namespace States
{
TCHAR BaseFilename[MAX_PATH];
int SelSlot;

void	Init (void)
{
	SelSlot = 0;
}

void	SetFilename (TCHAR *Filename)
{
	// all we need is the base filename
	_tsplitpath(Filename, NULL, NULL, BaseFilename, NULL);
}

void	SetSlot (int Slot)
{
	TCHAR tpchr[MAX_PATH];
	FILE *tmp;
	SelSlot = Slot;
	_stprintf(tpchr, _T("%s\\States\\%s.ns%i"), DataPath, BaseFilename, SelSlot);
	tmp = _tfopen(tpchr, _T("rb"));
	if (tmp)
	{
		fclose(tmp);
		PrintTitlebar(_T("State Selected: %i (occupied)"), Slot);
	}
	else	PrintTitlebar(_T("State Selected: %i (empty)"), Slot);
}

int	SaveData (FILE *out)
{
	int flen = 0, clen;
	{
		fwrite("PLUG", 1, 4, out);	flen += 4;
		fwrite(&clen, 4, 1, out);	flen += 4;
		clen = PlugThruDevice::save(out);
		fseek(out, -clen - 4, SEEK_CUR);
		fwrite(&clen, 4, 1, out);
		fseek(out, clen, SEEK_CUR);	flen += clen;
	}
	{
		fwrite("CPUS", 1, 4, out);	flen += 4;
		fwrite(&clen, 4, 1, out);	flen += 4;
		clen = CPU::CPU[0]->Save(out);
		if (CPU::CPU[1]) clen += CPU::CPU[1]->Save(out);
		fseek(out, -clen - 4, SEEK_CUR);
		fwrite(&clen, 4, 1, out);
		fseek(out, clen, SEEK_CUR);	flen += clen;
	}
	{
		fwrite("PPUS", 1, 4, out);	flen += 4;
		fwrite(&clen, 4, 1, out);	flen += 4;
		clen = PPU::PPU[0]->Save(out);
		if (PPU::PPU[1]) clen += PPU::PPU[1]->Save(out);
		fseek(out, -clen - 4, SEEK_CUR);
		fwrite(&clen, 4, 1, out);
		fseek(out, clen, SEEK_CUR);	flen += clen;
	}
	{
		fwrite("APUS", 1, 4, out);	flen += 4;
		fwrite(&clen, 4, 1, out);	flen += 4;
		clen = APU::APU[0]->Save(out);
		if (APU::APU[1]) clen +=APU::APU[1]->Save(out);
		fseek(out, -clen - 4, SEEK_CUR);
		fwrite(&clen, 4, 1, out);
		fseek(out, clen, SEEK_CUR);	flen += clen;
	}
	{
		fwrite("CONT", 1, 4, out);	flen += 4;
		fwrite(&clen, 4, 1, out);	flen += 4;
		clen = Controllers::Save(out);
		fseek(out, -clen - 4, SEEK_CUR);
		fwrite(&clen, 4, 1, out);
		fseek(out, clen, SEEK_CUR);	flen += clen;
	}
	{
		for (clen = RI.PRGRAMSize - 1; clen >= 0; clen--)
			if (NES::PRG_RAM[clen >> 12][clen & 0xFFF])
				break;
		if (clen >= 0)
		{
			clen++;
			fwrite("NPRA", 1, 4, out);		flen += 4;
			fwrite(&clen, 4, 1, out);		flen += 4;
			fwrite(NES::PRG_RAM, 1, clen, out);	flen += clen;	//	PRAM	uint8[...]	PRG_RAM data
		}
	}
	{
		for (clen = RI.CHRRAMSize - 1; clen >= 0; clen--)
			if (NES::CHR_RAM[clen >> 10][clen & 0x3FF])
				break;
		if (clen >= 0)
		{
			clen++;
			fwrite("NCRA", 1, 4, out);		flen += 4;
			fwrite(&clen, 4, 1, out);		flen += 4;
			fwrite(NES::CHR_RAM, 1, clen, out);	flen += clen;	//	CRAM	uint8[...]	CHR_RAM data
		}
	}
	if (RI.ROMType == ROM_FDS)
	{
		fwrite("DISK", 1, 4, out);	flen += 4;
		fwrite(&clen, 4, 1, out);	flen += 4;
		clen = NES::FDSSave(out);
		fseek(out, -clen - 4, SEEK_CUR);
		fwrite(&clen, 4, 1, out);
		fseek(out, clen, SEEK_CUR);	flen += clen;
		
		clen = FDS::saveLoad(STATE_SIZE, 0, NULL);
		unsigned char *tpmi = new unsigned char[clen];
		FDS::saveLoad(STATE_SAVE, 0, tpmi);
		fwrite("FDSR", 1, 4, out);	flen += 4;
		fwrite(&clen, 4, 1, out);	flen += 4;
		fwrite(tpmi, 1, clen, out);	flen += clen;
		delete[] tpmi;
	}
	{
		if ((MI) && (MI->SaveLoad))
			clen = MI->SaveLoad(STATE_SIZE, 0, NULL);
		else	clen = 0;
		if (clen)
		{
			unsigned char *tpmi = new unsigned char[clen];
			MI->SaveLoad(STATE_SAVE, 0, tpmi);
			fwrite("MAPR", 1, 4, out);	flen += 4;
			fwrite(&clen, 4, 1, out);	flen += 4;
			fwrite(tpmi, 1, clen, out);	flen += clen;	//	CUST	uint8[...]	Custom mapper data
			delete[] tpmi;
		}
	}
	if (Movie::Mode)	// save state when recording, reviewing, OR playing
	{
		fwrite("NMOV", 1, 4, out);	flen += 4;
		fwrite(&clen, 4, 1, out);	flen += 4;
		clen = Movie::Save(out);
		fseek(out, -clen - 4, SEEK_CUR);
		fwrite(&clen, 4, 1, out);
		fseek(out, clen, SEEK_CUR);	flen += clen;
	}
	return flen;
}

void	SaveNamedState (void) {
	OPENFILENAME ofn;
	TCHAR FileName[MAX_PATH];
	
	ZeroMemory(&ofn, sizeof(ofn));
	FileName[0]='\0';
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hMainWnd;
	ofn.hInstance = hInst;
	ofn.lpstrFilter =_T("Nintendulator Save File (*.NS?)\0") _T("*.ns?\0")                         // 1
	                 _T("\0");
	ofn.lpstrCustomFilter = NULL;
	ofn.nFilterIndex =1;
	ofn.lpstrFile = FileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = Settings::Path_NST;
	ofn.Flags = OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt = _T("nst");
	ofn.lCustData = 0;
	ofn.lpfnHook = NULL;
	ofn.lpTemplateName = NULL;
	
	if (GetSaveFileName(&ofn)) {
		SaveState(FileName, FileName +ofn.nFileOffset);
		_tcscpy(Settings::Path_NST, FileName);
		Settings::Path_NST[ofn.nFileOffset - 1] = 0;
	}
}

void	LoadNamedState (void) {
	OPENFILENAME ofn;
	TCHAR FileName[MAX_PATH];
	
	ZeroMemory(&ofn, sizeof(ofn));	
	FileName[0]='\0';
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hMainWnd;
	ofn.hInstance = hInst;
	ofn.lpstrFilter =_T("Nintendulator Save File (*.NS?)\0") _T("*.ns?\0")                         // 1
	                 _T("\0");
	ofn.lpstrCustomFilter = NULL;
	ofn.nFilterIndex =1;
	ofn.lpstrFile = FileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = Settings::Path_NST;
	ofn.Flags = OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt = _T("nst");
	ofn.lCustData = 0;
	ofn.lpfnHook = NULL;
	ofn.lpTemplateName = NULL;
	
	if (GetOpenFileName(&ofn)) {
		LoadState(FileName, FileName +ofn.nFileOffset);
		_tcscpy(Settings::Path_NST, FileName);
		Settings::Path_NST[ofn.nFileOffset - 1] = 0;
	}
}

void	SaveNumberedState (void)
{
	TCHAR tps[MAX_PATH];
	TCHAR shortName[MAX_PATH];

	_stprintf(tps, _T("%s\\States\\%s.ns%i"), DataPath, BaseFilename, SelSlot);
	_stprintf(shortName, _T("%s.ns%i"), BaseFilename, SelSlot);
	SaveState(tps, shortName);
}
void	LoadNumberedState (void)
{
	TCHAR tps[MAX_PATH];
	TCHAR shortName[MAX_PATH];

	_stprintf(tps, _T("%s\\States\\%s.ns%i"), DataPath, BaseFilename, SelSlot);
	_stprintf(shortName, _T("%s.ns%i"), BaseFilename, SelSlot);
	LoadState(tps, shortName);
}

void	SaveState (const TCHAR* tps, const TCHAR *shortName)
{
	int flen;
	FILE *out;

	out = _tfopen(tps, _T("w+b"));
	flen = 0;

	fwrite("NSS\x1A", 1, 4, out);
	SaveVersion(out, STATES_CUR_VERSION);
	fwrite(&flen, 4, 1, out);
	if (Movie::Mode)	// save NREC during playback as well
		fwrite("NREC", 1, 4, out);
	else	fwrite("NSAV", 1, 4, out);

	flen = SaveData(out);

	// Write final filesize
	fseek(out, 8, SEEK_SET);
	fwrite(&flen, 4, 1, out);
	fclose(out);

	PrintTitlebar(_T("State saved: %s"), shortName);
	Movie::ShowFrame();
}

BOOL	LoadData (FILE *in, int flen, int version_id)
{
	char csig[4];
	int clen;
	BOOL SSOK = TRUE;
	fread(&csig, 1, 4, in);	flen -= 4;
	fread(&clen, 4, 1, in);	flen -= 4;
	while (flen > 0)
	{
		flen -= clen;
		if (!memcmp(csig, "CPUS", 4)) {
			clen -= CPU::CPU[0]->Load(in, version_id);
			if (CPU::CPU[1]) clen -= CPU::CPU[1]->Load(in, version_id);
		} else if (!memcmp(csig, "PPUS", 4)) {
			clen -= PPU::PPU[0]->Load(in, version_id);
			if (PPU::PPU[1]) clen -= PPU::PPU[1]->Load(in, version_id);
		} else if (!memcmp(csig, "APUS", 4)) {
			clen -= APU::APU[0]->Load(in, version_id);
			if (APU::APU[1]) clen -= APU::APU[1]->Load(in, version_id);
		} else if (!memcmp(csig, "CONT", 4))
			clen -= Controllers::Load(in, version_id);
		else if (!memcmp(csig, "PLUG", 4))
			clen -= PlugThruDevice::load(in, version_id);
		else if (!memcmp(csig, "NPRA", 4))
		{
			memset(NES::PRG_RAM, 0, sizeof(NES::PRG_RAM));
			fread(NES::PRG_RAM, 1, clen, in);	clen = 0;
		}
		else if (!memcmp(csig, "NCRA", 4))
		{
			memset(NES::CHR_RAM, 0, sizeof(NES::CHR_RAM));
			fread(NES::CHR_RAM, 1, clen, in);	clen = 0;
		}
		else if (!memcmp(csig, "DISK", 4))
		{
			if (RI.ROMType == ROM_FDS)
				clen -= NES::FDSLoad(in, version_id);
			else	
				EI.DbgOut(_T("Error - DISK save block found for non-FDS game!"));
		}
		else if (!memcmp(csig, "FDSR", 4))
		{
			int len = FDS::saveLoad(STATE_SIZE, 0, NULL);
			unsigned char *tpmi = new unsigned char[len];
			fread(tpmi, 1, len, in);
			FDS::saveLoad(STATE_LOAD, 0, tpmi);
			delete[] tpmi;
			clen -= len;
		}
		else if (!memcmp(csig, "MAPR", 4))
		{
			if ((MI) && (MI->SaveLoad))
			{
				int len = MI->SaveLoad(STATE_SIZE, 0, NULL);
				unsigned char *tpmi = new unsigned char[len];
				fread(tpmi, 1, len, in);		//	CUST	uint8[...]	Custom mapper data
				MI->SaveLoad(STATE_LOAD, 0, tpmi);
				delete[] tpmi;
				clen -= len;
			}
		}
		else if (!memcmp(csig, "NMOV", 4))
			clen -= Movie::Load(in, version_id);
		if (clen != 0)
		{
			SSOK = FALSE;			// too much, or too little
			fseek(in, clen, SEEK_CUR);	// seek back to the block boundary
		}
		if (flen > 0)
		{
			fread(&csig, 1, 4, in);	flen -= 4;
			fread(&clen, 4, 1, in);	flen -= 4;
		}
	}
	if (RI.ROMType ==ROM_FDS && Settings::FastLoad) FDS::applyPatches();
	return SSOK;
}

void	LoadState (const TCHAR *tps, const TCHAR *shortName)
{
	char tpc[5];
	int version_id;
	FILE *in;
	int flen;

	in = _tfopen(tps, _T("rb"));
	if (!in)
	{
		PrintTitlebar(_T("No such save state: %s"), shortName);
		return;
	}

	fread(tpc, 1, 4, in);
	if (memcmp(tpc, "NSS\x1a", 4))
	{
		fclose(in);
		PrintTitlebar(_T("Not a valid savestate file: %s"), shortName);
		return;
	}
	version_id = LoadVersion(in);
	if ((version_id < STATES_MIN_VERSION) || (version_id > STATES_CUR_VERSION))
	{
		fclose(in);
		tpc[4] = 0;
		PrintTitlebar(_T("Invalid or unsupported savestate version (%i): %s"), version_id, shortName);
		return;
	}
	fread(&flen, 4, 1, in);
	fread(tpc, 1, 4, in);

	if (!memcmp(tpc, "NMOV", 4))
	{
		fclose(in);
		PrintTitlebar(_T("Selected savestate (%s) is a movie file - cannot load!"), shortName);
		return;
	}
	else if (!memcmp(tpc, "NREC", 4))
	{
		// Movie savestate, can always load these
	}
	else if (!memcmp(tpc, "NSAV", 4))
	{
		// Non-movie savestate, can NOT load these while a movie is open
		if (Movie::Mode)
		{
			fclose(in);
			PrintTitlebar(_T("Selected savestate (%s) does not contain movie data!"), shortName);
			return;
		}
	}
	else
	{
		fclose(in);
		tpc[4] = 0;
		PrintTitlebar(_T("Selected savestate (%s) has unknown type! (%hs)"), shortName, tpc);
		return;
	}

	fseek(in, 16, SEEK_SET);
	NES::Reset(RESET_SOFT);

	if (Movie::Mode & MOV_REVIEW)		// If the user is reviewing an existing movie
		Movie::Mode = MOV_RECORD;	// then resume recording once they LOAD state

	if (LoadData(in, flen, version_id))
		PrintTitlebar(_T("%s loaded: %s"), (version_id == STATES_CUR_VERSION) ? _T("State") : _T("Old state"), shortName);
	else	PrintTitlebar(_T("%s loaded with errors: %s"), (version_id == STATES_CUR_VERSION) ? _T("State") : _T("Old state"), shortName);
	Movie::ShowFrame();
	fclose(in);
}

void	SaveVersion (FILE *out, int version_id)
{
	BYTE buf[4];
	buf[0] = (version_id / 1000) % 10 + '0';
	buf[1] = (version_id /  100) % 10 + '0';
	buf[2] = (version_id /   10) % 10 + '0';
	buf[3] = (version_id /    1) % 10 + '0';
	fwrite(buf, 1, 4, out);
}

int	LoadVersion (FILE *in)
{
	BYTE buf[4];
	fread(buf, 1, 4, in);
	int version_id = (buf[0] - '0') * 1000 + (buf[1] - '0') * 100 + (buf[2] - '0') * 10 + (buf[3] - '0');
	return version_id;
}
} // namespace States