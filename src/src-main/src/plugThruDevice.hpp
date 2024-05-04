#pragma once

namespace PlugThruDevice {
// Read/write handlers of attached devices
extern	FCPURead	adCPURead[32];
extern	FCPURead	adCPUReadDebug[32];
extern	FCPUWrite	adCPUWrite[32];
extern	FPPURead	adPPURead[32];
extern	FPPURead	adPPUReadDebug[32];
extern	FPPUWrite	adPPUWrite[32];
extern	MapperInfo	*adMI;
extern	TCHAR		*Description;

extern	FCPURead	(MAPINT	*GetCPUReadHandler	) (int);
extern	FCPURead	(MAPINT	*GetCPUReadHandlerDebug) (int);
extern	FCPUWrite	(MAPINT	*GetCPUWriteHandler	) (int);
extern	FPPURead	(MAPINT	*GetPPUReadHandler	) (int);
extern	FPPURead	(MAPINT	*GetPPUReadHandlerDebug) (int);
extern	FPPUWrite	(MAPINT	*GetPPUWriteHandler	) (int);
extern	void	(MAPINT	*SetCPUReadHandler	) (int, FCPURead);
extern	void	(MAPINT	*SetCPUReadHandlerDebug) (int, FCPURead);
extern	void	(MAPINT	*SetCPUWriteHandler	) (int, FCPUWrite);
extern	void	(MAPINT	*SetPPUReadHandler	) (int, FPPURead);
extern	void	(MAPINT	*SetPPUReadHandlerDebug) (int, FPPURead);
extern	void	(MAPINT	*SetPPUWriteHandler	) (int, FPPUWrite);
extern	bool	button;
extern	int	buttonCount;

int	MAPINT	passCPURead     (int, int);
int	MAPINT	passCPUReadDebug(int, int);
void	MAPINT	passCPUWrite    (int, int, int);
void	MAPINT	ignoreWrite     (int, int, int);
int	MAPINT	passPPURead     (int, int);
int	MAPINT	passPPUReadDebug(int, int);
void	MAPINT	passPPUWrite    (int, int, int);
void	MAPINT	adSetIRQ	(int);
void	MAPINT	pdSetIRQ	(int);

void	initPlugThruDevice (void);
void	uninitPlugThruDevice (void);
void	initHandlers (void);
void	uninitHandlers (void);
bool	loadBIOS (TCHAR*, uint8_t*, size_t);
int	save (FILE *out);
int	load (FILE *in, int version_id);
void	pressButton (void);
void	releaseButton (void);
void	checkButton (void);
bool	needSecondCPU (int);

extern MapperInfo plugThruDevice_GameGenie;
extern MapperInfo plugThruDevice_ProActionReplay;
extern MapperInfo plugThruDevice_GameDoctor1M;
extern MapperInfo plugThruDevice_GameMaster19;
extern MapperInfo plugThruDevice_GameMaster20;
extern MapperInfo plugThruDevice_GameMasterKid;
extern MapperInfo plugThruDevice_SuperGameDoctor2M;
extern MapperInfo plugThruDevice_SuperGameDoctor4M;
extern MapperInfo plugThruDevice_MasterBoy;
extern MapperInfo plugThruDevice_GameActionReplay;
extern MapperInfo plugThruDevice_MagicardIITurbo;
extern MapperInfo plugThruDevice_SuperMagicCard;
extern MapperInfo plugThruDevice_GameConverter1M;
extern MapperInfo plugThruDevice_GameConverter2M;
extern MapperInfo plugThruDevice_MiniDoctor;
extern MapperInfo plugThruDevice_TurboGameDoctor4P;
extern MapperInfo plugThruDevice_TurboGameDoctor6P;
extern MapperInfo plugThruDevice_TurboGameDoctor6M;
extern MapperInfo plugThruDevice_SousekiFammy;
extern MapperInfo plugThruDevice_Bit79;
extern MapperInfo plugThruDevice_StudyBox;
extern MapperInfo plugThruDevice_DoctorPCJr;
}