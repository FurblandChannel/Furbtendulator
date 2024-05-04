#include <vector>
#include <queue>

struct Drive {
	bool	inserted;
	bool	running;
	bool	writeProtected;
	int	cylinder;
	int	cylinders;
	int	sides;
	int	sectors;
	int	dataRate;
	uint8_t *data;
};

class FDC;
struct CommandInfo {
	size_t length;
	void (FDC::*handler)(void);
};

class FDC {
	Drive drives[4], *drive;
	uint8_t DOR, MSR, ST0, ST1, ST2, SRA, latch;
	uint8_t cylinder, head, sectorNumber, *data;
	int	dataRate, sectorSize, bytesLeft;
	void	(FDC::*phaseHandler)(void);
	std::vector<uint8_t> command;
	std::queue<uint8_t> resultQueue;
	static const CommandInfo commandInfo[0x20];
	
	void	phCommand (void);
	void	phResult (void);
	void	endCommand (bool, int, ...);
	void	nextSector (void);
	void	nextSectorFormat (void);
	void	cmReadID (void);
	void	cmReadData (void);
	void	cmWriteData (void);
	void	cmFormatTrack (void);
	void	cmSeek (void);
	void	cmRecalibrate (void);
	void	cmSenseInterruptStatus (void);
	void	cmSpecify (void);
	void	cmSenseDriveStatus (void);
	void	cmInvalid (void);	
public:
	FDC();
	void	insertDisk (uint8_t, uint8_t*, size_t, bool);
	void	ejectDisk (uint8_t);
	void	reset (void);
	void	run (void);
	int	readIO (int);
	int	readIODebug (int);
	void	writeIO (int, int);
	bool	irqRaised (void);
};