#pragma once

#define	STATES_CUR_VERSION	1100	// current savestate version
#define	STATES_MIN_VERSION	950	// minimum supported savestate version

namespace States
{
extern TCHAR BaseFilename[MAX_PATH];
extern int SelSlot;

void	Init (void);
void	SetFilename (TCHAR *name);
void	SetSlot (int Slot);
int	SaveData (FILE *);
BOOL	LoadData (FILE *, int len, int ver);
void	SaveNumberedState (void);
void	LoadNumberedState (void);
void	SaveNamedState (void);
void	LoadNamedState (void);
void	SaveState (const TCHAR*, const TCHAR*);
void	LoadState (const TCHAR*, const TCHAR*);
void	SaveVersion (FILE *, int ver);
int	LoadVersion (FILE *);
} // namespace States
