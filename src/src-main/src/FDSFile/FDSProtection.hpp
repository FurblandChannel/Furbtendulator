#ifndef INCLUDED_FDSPROTECTION_HPP
#define INCLUDED_FDSPROTECTION_HPP 1

// ---- FDSProtection
// Protection detector and handler.
// Returns the true size of every block based on the detected protection,
// and the presence, type and size of an "epilogue" --- a block that follows the files according to the file count and hidden files.
// Any encountered block must be provided to "notify", and the actual file size of any following block queried with getFileSize().

class FDSProtection {
	bool     autodetectProtection;
	bool     oujiProtection;
	bool     substituteFileSizes;
	bool     expectMagicCardTrainer;
	bool     kgkProtection;
	bool     quickHunterProtection;
	uint8_t  nextFileType;
	uint16_t nextFileAddress;
	uint16_t nextFileSize;
	uint16_t cpuFileSize;
	uint16_t ppuFileSize;
	uint8_t  filesLeft;
	int      count2000;
public:
	// Constructor telling the procedure whether to consider protections
	FDSProtection(bool _autodetect):
		autodetectProtection(_autodetect),
		substituteFileSizes(false),
		expectMagicCardTrainer(false),
		kgkProtection(false),
		quickHunterProtection(false),
		nextFileType(0),
		nextFileAddress(0),
		nextFileSize(0),
		cpuFileSize(0),
		ppuFileSize(0),
		filesLeft(0) {
	}
	
	// Show every block that is encountered to the protection handler ...
	void     notify(const FDSBlock&);
	
	// then retrieve the actual size of a type 4 block based on that information.
	uint16_t getFileSize() const;
	
	// After all files denoted in the type 2 file count have been processed, get information on whether a final block should be expected, and which type of size it will have.
	bool     epiloguePresent() const;
	uint8_t  epilogueType() const;
	size_t   epilogueSize() const;
};
#endif