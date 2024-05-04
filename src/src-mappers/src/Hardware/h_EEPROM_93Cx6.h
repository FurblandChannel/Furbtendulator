#pragma once
#include	"..\interface.h"

class EEPROM_93Cx6 {
	size_t		capacity;
	uint8_t*	storage;
	uint8_t		opcode;
	uint16_t	data;
	uint16_t 	address;
	uint8_t		state;
	bool		lastCLK;
	bool		writeEnabled;
	bool		output;
	uint8_t		wordSize;
	
	uint8_t		State_address;
	uint8_t		State_data;
public:	
	EEPROM_93Cx6	(uint8_t*, size_t, uint8_t);
	int		MAPINT	saveLoad (STATE_TYPE, int, uint8_t *);
	void		write (bool, bool, bool);
	bool		read (void);
};
