#include	"..\interface.h"
#include	"Sound\s_VRC7.h"

namespace VRC7 {
void	MAPINT	syncPRG(int, int);
void	MAPINT	syncCHR(int, int);
void	MAPINT	syncCHR_ROM(int, int);
void	MAPINT	syncCHR_RAM(int, int);
void	MAPINT	syncMirror(void);
void	MAPINT	writePRGSound (int, int, int);
void	MAPINT  writeCHR (int, int, int);
void	MAPINT	writeMisc (int, int, int);
void	MAPINT	writeIRQ (int, int, int);
void	MAPINT	load (FSync, uint8_t, uint8_t);
void	MAPINT	reset (RESET_TYPE);
void	MAPINT	unload (void);
void	MAPINT	cpuCycle (void);
int	MAPINT	saveLoad (STATE_TYPE, int, unsigned char *);
int	MAPINT	mapperSnd (int);
}
