#include "stdafx.h"
#include "Nintendulator.h"
#include "Settings.h"
#include "MapperInterface.h"
#include "NES.h"
#include "Sound.h"
#include "APU.h"
#include "CPU.h"
#include "PPU.h"
#include "AVI.h"
#include "GFX.h"
#include "Controllers.h"
#include "Filter.h"
#include "Settings.h"
#include "OneBus.h"

# pragma comment(lib, "dsound.lib")
# pragma comment(lib, "dxguid.lib")

namespace Sound {
unsigned long		MHz =1;
int			Cycles;
int			BufPos;
unsigned long		next_pos;
BOOL			isEnabled;
double			lastFiltered =0, lastUnfiltered =0;

Filter::LPF_RC		LPFBandlimit(15392.0 / 1789773.0); // Mandatory low-pass filter for band-imited synthesis
Filter::LPF_RC		LPFOptional ( 4800.0 / 1789773.0); // Optional low-pass filter
Filter::LPF_RC		LPF_RF      ( 3183.0 / 1789773.0); // RF low-pass filter
Filter::HPF_RC		HPFRemoveDC (   37.0 / 1789773.0); // Mandatory high-pass filter to remove DC offset
#if	SUPPORT_STEREO
Filter::LPF_RC		LLPFBandlimit(15392.0 / 1789773.0); // Left: Mandatory low-pass filter for band-imited synthesis
Filter::LPF_RC		LLPFOptional ( 4800.0 / 1789773.0); // Left: Optional low-pass filter
Filter::LPF_RC		LLPF_RF      ( 3183.0 / 1789773.0); // Left: RF low-pass filter
Filter::HPF_RC		LHPFRemoveDC (   37.0 / 1789773.0); // Left: Mandatory high-pass filter to remove DC offset
Filter::LPF_RC		RLPFBandlimit(15392.0 / 1789773.0); // Right: Mandatory low-pass filter for band-imited synthesis
Filter::LPF_RC		RLPFOptional ( 4800.0 / 1789773.0); // Right: Optional low-pass filter
Filter::LPF_RC		RLPF_RF      ( 3183.0 / 1789773.0); // Right: RF low-pass filter
Filter::HPF_RC		RHPFRemoveDC (   37.0 / 1789773.0); // Right: Mandatory high-pass filter to remove DC offset
#endif

LPDIRECTSOUND		DirectSound;
LPDIRECTSOUNDBUFFER	PrimaryBuffer;
LPDIRECTSOUNDBUFFER	Buffer;

short			*buffer;
int			buflen;

unsigned long		LockSize;

#if SUPPORT_STEREO
#define CHANNELS	2
#else
#define CHANNELS	1
#endif	
#define	FREQ		SAMPLING_RATE
#define	BITS		16
#define	FRAMEBUF	4
const	unsigned int	LOCK_SIZE = CHANNELS * FREQ * (BITS / 8);

#define	Try(action, errormsg) do {\
	if (FAILED(action))\
	{\
		Stop();\
		Start();\
		if (FAILED(action))\
		{\
			SoundOFF();\
			MessageBox(hMainWnd, errormsg, _T("Nintendulator"), MB_OK | MB_ICONERROR);\
			return;\
		}\
	}\
} while (false)

void	SetRegion (void) {
	BOOL Enabled = isEnabled;
	BOOL Started = (Buffer != NULL);
	if (Enabled) SoundOFF();
	if (Started) Stop();
	int WantFPS = 60;
	switch (NES::CurRegion) {
	case Settings::REGION_NTSC:
		MHz = 1789773;
		WantFPS = 60;
		break;
	case Settings::REGION_PAL:
		MHz = 1662607;
		WantFPS = 50;
		break;
	case Settings::REGION_DENDY:
		if (Settings::Dendy60Hz) {
			MHz = 1773447 *60/50;
			WantFPS =50;
		} else {
			MHz = 1773447;
			WantFPS =50;
		}
		break;
	default:
		break;
	}
	LockSize = LOCK_SIZE / WantFPS;
	buflen = LockSize / (BITS / 8);
	if (buffer) delete buffer;
	buffer = new short[buflen];
	if (Started) Start();
	if (Enabled) SoundON();
}

void	Init (void) {
	DirectSound	= NULL;
	PrimaryBuffer	= NULL;
	Buffer		= NULL;
	buffer		= new short[1];
	isEnabled	= FALSE;
	LockSize	= 1;
	buflen		= 0;
	BufPos		= 0;
	next_pos	= 0;


	if (FAILED(DirectSoundCreate(&DSDEVID_DefaultPlayback, &DirectSound, NULL))) {
		Destroy();
		MessageBox(hMainWnd, _T("Failed to create DirectSound interface!"), _T("Nintendulator"), MB_OK);
		return;
	}
	if (FAILED(DirectSound->SetCooperativeLevel(hMainWnd, DSSCL_PRIORITY))) {
		Destroy();
		MessageBox(hMainWnd, _T("Failed to set cooperative level!"), _T("Nintendulator"), MB_OK);
		return;
	}
}

void	Destroy (void) {
	Stop();
	if (DirectSound) {
		DirectSound->Release();
		DirectSound = NULL;
	}
}

void	Start (void) {
	WAVEFORMATEX WFX;
	DSBUFFERDESC DSBD;
	if (!DirectSound) return;

	ZeroMemory(&DSBD, sizeof(DSBUFFERDESC));
	DSBD.dwSize = sizeof(DSBUFFERDESC);
	DSBD.dwFlags = DSBCAPS_PRIMARYBUFFER;
	DSBD.dwBufferBytes = 0;
	DSBD.lpwfxFormat = NULL;
	if (FAILED(DirectSound->CreateSoundBuffer(&DSBD, &PrimaryBuffer, NULL))) {
		Stop();
		MessageBox(hMainWnd, _T("Failed to create primary buffer!"), _T("Nintendulator"), MB_OK);
		return;
	}

	ZeroMemory(&WFX, sizeof(WAVEFORMATEX));
	WFX.wFormatTag = WAVE_FORMAT_PCM;
	WFX.nChannels = CHANNELS;
	WFX.nSamplesPerSec = FREQ;
	WFX.wBitsPerSample = BITS;
	WFX.nBlockAlign = WFX.wBitsPerSample / 8 * WFX.nChannels;
	WFX.nAvgBytesPerSec = WFX.nSamplesPerSec * WFX.nBlockAlign;
	if (FAILED(PrimaryBuffer->SetFormat(&WFX))) {
		Stop();
		MessageBox(hMainWnd, _T("Failed to set output format!"), _T("Nintendulator"), MB_OK);
		return;
	}
	if (FAILED(PrimaryBuffer->Play(0, 0, DSBPLAY_LOOPING))) {
		Stop();
		MessageBox(hMainWnd, _T("Failed to start playing primary buffer!"), _T("Nintendulator"), MB_OK);
		return;
	}

	DSBD.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_LOCSOFTWARE | DSBCAPS_GETCURRENTPOSITION2;
	DSBD.dwBufferBytes = LockSize * FRAMEBUF;
	DSBD.lpwfxFormat = &WFX;

	if (FAILED(DirectSound->CreateSoundBuffer(&DSBD, &Buffer, NULL))) {
		Stop();
		MessageBox(hMainWnd, _T("Failed to create secondary buffer!"), _T("Nintendulator"), MB_OK);
		return;
	}
	//EI.DbgOut(_T("Created %iHz %i bit audio buffer, %i frames (%i bytes per frame)"), WFX.nSamplesPerSec, WFX.wBitsPerSample, DSBD.dwBufferBytes / LockSize, LockSize);
}

void	Stop (void) {
	if (Buffer) {
		SoundOFF();
		Buffer->Release();
		Buffer = NULL;
	}
	if (PrimaryBuffer) {
		PrimaryBuffer->Stop();
		PrimaryBuffer->Release();
		PrimaryBuffer = NULL;
	}
	if (buffer) {
		delete buffer;
		buffer = NULL;
	}
}

void	SoundOFF (void) {
	if (!isEnabled) return;
	isEnabled = FALSE;
	if (Buffer) Buffer->Stop();
}

void	SoundON (void) {
	LPVOID bufPtr;
	DWORD bufBytes;
	if (isEnabled) 	return;
	if (!Buffer) {
		Start();
		if (!Buffer) return;
	}
	Try(Buffer->Lock(0, 0, &bufPtr, &bufBytes, NULL, 0, DSBLOCK_ENTIREBUFFER), _T("Error locking sound buffer (Clear)"));
	ZeroMemory(bufPtr, bufBytes);
	Try(Buffer->Unlock(bufPtr, bufBytes, NULL, 0), _T("Error unlocking sound buffer (Clear)"));
	isEnabled = TRUE;

	Try(Buffer->Play(0, 0, DSBPLAY_LOOPING), _T("Unable to start playing buffer"));
	next_pos = 0;
}

void	Config (HWND hWnd) {
}

#if DISABLE_ALL_FILTERS
double	cumOutput =.0;
#endif

void	Run (void) {
	if (!Buffer) return;
	int NewBufPos =(long long) FREQ * ++Cycles / MHz * CHANNELS;
	if (NewBufPos >= buflen) {
		LPVOID bufPtr;
		DWORD bufBytes;
		unsigned long rpos, wpos;

		Try(Buffer->GetCurrentPosition(&rpos, &wpos), _T("Error getting audio position"));
		int diff =rpos -wpos;
		if (diff <0) diff =-diff;
		
		Cycles = NewBufPos = 0;
		if (AVI::handle) AVI::AddAudio();
		/*if ((RI.ROMType ==ROM_NSF || GFX::WantFPS !=60 || !Settings::VSync) && !Controllers::capsLock && !GFX::Fullscreen)*/
		do {
			Sleep(1);
			if (!isEnabled) break;
			Try(Buffer->GetCurrentPosition(&rpos, &wpos), _T("Error getting audio position"));
			rpos /= LockSize;
			wpos /= LockSize;
			if (wpos < rpos) wpos += FRAMEBUF;
		} while ((!Controllers::capsLock || Controllers::scrollLock) && (rpos <= next_pos) && (next_pos <= wpos));		
		if (isEnabled) 	{
			Try(Buffer->Lock(next_pos * LockSize, LockSize, &bufPtr, &bufBytes, NULL, 0, 0), _T("Error locking sound buffer"));
			memcpy(bufPtr, buffer, bufBytes);
			Try(Buffer->Unlock(bufPtr, bufBytes, NULL, 0), _T("Error unlocking sound buffer"));
			next_pos = (next_pos + 1) % FRAMEBUF;
		}
	}
	
#if SUPPORT_STEREO
	double OutputL =0.0, OutputR =0.0, Output =0.0;
	if (isEnabled && (!Controllers::capsLock || Controllers::scrollLock || AVI::handle)) {
		if (Settings::VSDualScreens ==2 && APU::APU[1]) {
			OutputL =APU::APU[0]->Output;
			OutputR =APU::APU[1]->Output;
		} else {
			Output =APU::APU[NES::WhichScreenToShow]->Output;
			OutputL =APU::APU[NES::WhichScreenToShow]->OutputL;
			OutputR =APU::APU[NES::WhichScreenToShow]->OutputR;
		}
		if (MI && MI->GenSound) {
			double add =(Settings::ExpansionAudioVolume /100.0) *MI->GenSound(1) /32767.0;	
			OutputL +=add;
			OutputR +=add;
			Output  +=add;
		}
		#if DISABLE_ALL_FILTERS
		#else
		if (Settings::LowPassFilterAPU ==2) {
			OutputL =LLPFOptional.process(OutputL);
			OutputR =RLPFOptional.process(OutputR);
		} else
		if (Settings::LowPassFilterAPU ==3) {
			OutputL =LLPF_RF.process(OutputL);
			OutputR =RLPF_RF.process(OutputR);
		} else {
			OutputL =LLPFBandlimit.process(OutputL);
			OutputR =RLPFBandlimit.process(OutputR);
		}
		#endif
		OutputL =LHPFRemoveDC.process(OutputL);
		OutputR =RHPFRemoveDC.process(OutputR);
	}
	
	if (NewBufPos != BufPos) {
		BufPos = NewBufPos;
		int samppos = OutputL *32767.0;
		if (samppos <-0x8000) samppos = -0x8000;
		if (samppos > 0x7FFF) samppos = 0x7FFF;
		buffer[BufPos] = (short)samppos;

		samppos = OutputR *32767.0;
		if (samppos <-0x8000) samppos = -0x8000;
		if (samppos > 0x7FFF) samppos = 0x7FFF;
		buffer[BufPos+1] = (short)samppos;		
	}
#else
	double Output =0.0;
	if (isEnabled && (!Controllers::capsLock || Controllers::scrollLock || AVI::handle)) {
		if (Settings::VSDualScreens ==2 && APU::APU[1])
			Output =APU::APU[0]->Output + APU::APU[1]->Output;
		else 
			Output =APU::APU[NES::WhichScreenToShow]->Output;
		if (MI && MI->GenSound)
			Output +=(Settings::ExpansionAudioVolume /100.0) *MI->GenSound(1) /32767.0;
		#if DISABLE_ALL_FILTERS
		#else
		if (Settings::LowPassFilterAPU ==2)
			Output =LPFOptional.process(Output +1e-15);
		else
		if (Settings::LowPassFilterAPU ==3)
			Output =LPF_RF.process(Output +1e-15);
		else
			Output =LPFBandlimit.process(Output +1e-15);
		#endif
		Output =HPFRemoveDC.process(Output);
		#if DISABLE_ALL_FILTERS
		cumOutput +=Output;
		#endif
	}
	
	if (NewBufPos != BufPos) {
		BufPos = NewBufPos;
		#if DISABLE_ALL_FILTERS
		int samppos = cumOutput *(32767.0 /1789773.0*SAMPLING_RATE);
		cumOutput =0.0;
		#else
		int samppos = Output *32767.0;
		#endif
		if (samppos <-0x8000) samppos = -0x8000;
		if (samppos > 0x7FFF) samppos = 0x7FFF;
		buffer[BufPos] = (short)samppos;
	}
#endif
}

} // namespace Sound
