#pragma once

namespace PlugThruDevice {
namespace GameMaster {
union BIOS_GM {
	uint8_t	k8 [4][2][4096];
	uint8_t	b  [32*1024];
};
union BIOS_ML {
	uint8_t	k4 [2][4096];
	uint8_t	b  [8*1024];
};
union PRGRAM {
	uint8_t k8 [64][2][4096];
	uint8_t k16[32][4][4096];
	uint8_t k32[16][8][4096];
	uint8_t	b  [512*1024];
};
union CHRRAM {
	uint8_t k1 [256][1024];
	uint8_t k2 [128][2][1024];
	uint8_t k4 [64][4][1024];
	uint8_t k8 [32][8][1024];
	uint8_t b  [256*1024];
};
union StateRAM {
	uint8_t k4 [8][4096];
	uint8_t b  [32*1024];
};

extern bool	enableMasterLink;
extern bool	v20;
extern BIOS_GM  bios;
extern PRGRAM   prgram;
extern CHRRAM   chrram;
extern StateRAM stateRAM;
extern uint8_t	workRAM[2048];

extern bool	initialized;
extern bool	map6toD;
extern bool	mapBIOS;
extern bool	mapMode;
extern bool	mapAuxiliary;
extern bool	ciramCHR;
extern bool	enableGD;

extern uint8_t	pageRead;
extern uint8_t	pageWrite;

extern uint8_t	val4185;
extern uint8_t	parallelDataGM;
extern uint8_t	parallelStateGM;
extern uint8_t	parallelDataML;
extern uint8_t	parallelStateML;
extern uint8_t	transferByte;
extern bool	frameTimerEnabled;
extern uint8_t	frameTimerCount;
extern bool	hvToggle;
extern uint8_t	cache[0x50];
extern uint8_t	controllerCount;
extern uint8_t	controllerData;

extern bool	intercept;
extern bool	selectKey;
extern uint8_t	nmiConfig;
extern uint8_t	inputState;

extern bool	logEnabled;
extern uint16_t	logAddr;
extern uint16_t	logAddr2;

extern uint8_t	fusemapData[4096];
extern uint8_t	fusemapLetter;

void	ml_load (void);
void	ml_reset (void);
void	ml_cpuCycle (void);
int	ml_saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data);

int	MAPINT	cpuRead_sgd (int, int);
int	MAPINT	cpuReadDebug_sgd (int, int);
void	MAPINT	cpuWrite_sgd (int, int, int);
int	MAPINT	ppuRead_sgd (int, int);
int	MAPINT	ppuReadDebug_sgd (int, int);
void	MAPINT	ppuWrite_sgd (int, int, int);
void	reset_sgd (RESET_TYPE);
void	cpuCycle_sgd (void);
void	ppuCycle_sgd (int, int, int, int);
int	saveLoad_sgd (STATE_TYPE, int, unsigned char *);
void	parseFusemap (void);
void	loadFusemap (uint32_t);
void	loadFusemap (uint8_t);

extern	FCPURead	cpuRead_mapper;
extern	FCPURead	cpuReadDebug_mapper;
extern	FCPUWrite	cpuWrite_mapper;
extern	FPPURead	ppuRead_mapper;
extern	FPPURead	ppuReadDebug_mapper;
extern	FPPUWrite	ppuWrite_mapper;
extern	void		(*reset_mapper)(RESET_TYPE);
extern	void		(*cpuCycle_mapper)(void);
extern	void		(*ppuCycle_mapper)(int, int, int, int);
extern	int		(*saveLoad_mapper)(STATE_TYPE, int, unsigned char *);

} // namespace GameMaster
} // namespace PlugThruDevice