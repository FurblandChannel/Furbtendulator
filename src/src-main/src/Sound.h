#pragma once

#include <mmsystem.h>
#define DIRECTSOUND_VERSION 0x0800
#include <dsound.h>

namespace Sound {
extern short	*buffer;
extern unsigned long	LockSize;
extern unsigned long	MHz;
extern int	buflen;
extern int	Cycles;
extern BOOL	isEnabled;

void	Init		(void);
void	Destroy		(void);
void	Start		(void);
void	Stop		(void);
void	SoundOFF	(void);
void	SoundON		(void);
void	Config		(HWND);
void	Run		(void);
void	SetRegion	(void);
} // namespace Sound
