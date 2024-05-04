#ifndef VGMCapture_h
#define VGMCapture_h 1
#include <cstdio>
#include <cstdlib>
#include <stdint.h>
#include <memory>
#include <vector>

class VGMCapture {
private:
#pragma pack(push, 1)
        struct VGMHeader {
                char id[4];
                uint32_t rofsEOF;
                uint32_t version;
                uint32_t clockSN76489;
                uint32_t clockYM2413;
                uint32_t rofsGD3;
                uint32_t samplesInFile;
                uint32_t rofsLoop;
                uint32_t samplesInLoop;
                uint32_t videoRefreshRate;
                uint16_t SNfeedback;
                uint8_t SNshiftRegisterWidth;
                uint8_t SNflags;
                uint32_t clockYM2612;
                uint32_t clockYM2151;
                uint32_t rofsData;
                uint32_t clockSegaPCM;
                uint32_t interfaceRegisterSegaPCM;
                uint32_t clockRF5C68;
                uint32_t clockYM2203;
                uint32_t clockYM2608;
                uint32_t clockYM2610;
                uint32_t clockYM3812;
                uint32_t clockYM3526;
                uint32_t clockY8950;
                uint32_t clockYMF262;
                uint32_t clockYMF278B;
                uint32_t clockYMF271;
                uint32_t clockYMZ280B;
                uint32_t clockRF5C164;
                uint32_t clockPWM;
                uint32_t clockAY8910;
                uint8_t typeAY8910;
                uint8_t flagsAY8910;
                uint8_t flagsAY8910_YM2203;
                uint8_t flagsAY8910_YM2608;
                uint8_t volumeModifier;
                uint8_t reserved1;
                uint8_t loopBase;
                uint8_t loopModifier;
                uint32_t clockGBdmg;
                uint32_t clockNESapu;
                uint32_t clockMultiPCM;
                uint32_t clockUPD7759;
                uint32_t clockOKIM6258;
                uint8_t flagsOKIM6258;
                uint8_t flagsK054539;
                uint8_t typeC140;
                uint8_t reserved2;
                uint32_t clockOKIM6295;
                uint32_t clockK051649;
                uint32_t clockK054539;
                uint32_t clockHuC6280;
                uint32_t clockC140;
                uint32_t clockK053260;
                uint32_t clockPokey;
                uint32_t clockQSound;
                uint32_t clockSCSP;
                uint32_t rofsExtraHeader;
                uint32_t clockWonderSwan;
                uint32_t clockVBvsu;
                uint32_t clockSAA1099;
                uint32_t clockES5503;
                uint32_t clockES5505;
                uint8_t channelsES5503;
                uint8_t channelsES5506;
                uint8_t clockDividerC352;
                uint8_t reserved3;
                uint32_t clockX1_010;
                uint32_t clockC352;
                uint32_t clockGA20;
                uint32_t reserved4[7];
        } header;
	struct VGMExtraHeader {
		uint32_t theSize;
		uint32_t rofsChpClock;
		uint32_t rofsChpVol;
		uint8_t  entryCount;
		uint8_t  chipID;
		uint8_t  flags;
		uint16_t volume;
	} extraHeader;
#pragma pack(pop)
        bool YM2413_used, APU1_used, APU2_used;

        FILE *handle;
        std::vector<uint8_t> buffer;

        void logTimeDifference(void);
public:
        VGMCapture(FILE *theHandle);
        ~VGMCapture();
	void nextTick();
        void ioWrite_YM2413(uint8_t index, uint8_t value);
        void ioWrite_APU1(uint8_t index, uint8_t value);
	void ioWrite_APU2(uint8_t index, uint8_t value);

        uint32_t totalSamples, tickCount;
	float samplesPassedFraction;
};
#endif
