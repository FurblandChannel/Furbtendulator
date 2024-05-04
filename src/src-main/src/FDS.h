#pragma once

namespace FDS {
extern char	gameName[5];
void			sync		(void);
void		MAPINT	load		(bool);
void		MAPINT	reset		(RESET_TYPE);
int		MAPINT	saveLoad	(STATE_TYPE,int,unsigned char *);
void		MAPINT	cpuCycle	(void);
void			checkIRQ	(void);
int		MAPINT	readReg		(int, int);
int		MAPINT	readRegDebug	(int, int);
void		MAPINT	writeReg	(int, int, int);

extern bool		pendingData;
// FastLoad stuff
extern unsigned int	diskPosition;
extern uint8_t		lastErrorCode;

void			initTrap (void);
void			applyPatches (void);

std::vector<uint8_t> 	diskReadBlock (uint8_t, size_t);
uint8_t 		getCPUByte (uint16_t);
uint16_t		getCPUWord (uint16_t);
void			putCPUByte (uint16_t, uint8_t);
void			putCPUWord (uint16_t, uint16_t);
void			putPPUByte (uint16_t, uint8_t);
} // namespace FDS