#define	F_MIRROR_VERTICAL	0x01
#define F_BATTERY		0x02
#define F_TRAINER		0x04
#define F_FOUR_SCREEN		0x08
#define F_VS			0x10
#define F_PLAYCHOICE_10		0x20
#define TV_NTSC			0x00
#define TV_PAL			0x01
#define TV_DUAL			0x02
#define TV_DENDY		0x03

struct CorrectMapperInfo {
	unsigned long int PRGCRC;
	unsigned short int MapperNum;
	int SubMapper;
	int Flags;
	int PRG_RAM;
	int CHR_RAM;
	int TVMode;
	int VSData;
	int Special;
	unsigned long int AllCRC;
} CorrectMapperInfos [] = {
//	         CRC  Map Subm Fl  PRG CHR       TV   VS   Special
	// Missing Battery
	{ 0x1E0C7EA3,   1,  0,  2,0x70,  0,  TV_NTSC,   0, 0 }, // Advanced Dungeons & Dragons - Dragons of Flame
	{ 0xA70B34FB,   1,  0,  2,0x70,  0,  TV_NTSC,   0, 0 }, // Advanced Dungeons & Dragons - Dragons of Flame (DvD Translations v1.03)
	{ 0x7831B2FF,   1,  0,  2,0x70,  0,  TV_NTSC,   0, 0 }, // America Daitouryou Senkyo
	// Misc
	{ 0x560E44B9, 158,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Alien Syndrome
	{ 0xACD7C8F3, 158,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Alien Syndrome
	{ 0xD43325A7, 303,  0,  0,   7,  7,  TV_NTSC,   0, 0 }, // Almana no Kiseki
	{ 0x353E6277, 303,  0,  0,   7,  7,  TV_NTSC,   0, 0 }, // Almana no Kiseki
	{ 0x1D080886,  79,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // 葉山レイコ、桂木麻也子のAV花札倶楽部
	{ 0x6F0EFBF5,  79,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // AV麻雀倶楽部 (Hacker International)
	{ 0xB69780CE,  79,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // AV麻雀倶楽部 (Hacker International)
	{ 0x1394E1A2,  48,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // 爆笑!! 人生劇場 3
	{ 0x9CF17FAB,   3,  2,  1,   0,  0,  TV_NTSC,   0, 0 }, // Banana
	// Some incomplete or bad dumps
	{ 0xD74B6B33,  45,  0,  0,   0,  0,  TV_NTSC,   0,-1 }, // 2-in-1 (Super 300-in-1) (Pocket Monster)
	{ 0xA957E6CD,   0,  0,  1,   0,  0,  TV_NTSC,   0,-1 }, // 125-in-1 (K)
	{ 0x5AF1FA4C,   4,  0,  0,   0,  0,  TV_NTSC,   0,-1 }, // 3-in-1 Street Blaster II Pro
	{ 0xF470A14C,   4,  0,  0,   0,  0,  TV_NTSC,   0,-1 }, // 2-in-1 Fight 12Peoples
	{ 0x9237FCB3,   4,  0,  0,   0,  0,  TV_NTSC,   0,-1 }, // 4-in-1 (in Gold Saint)
	{ 0x814AAEB6,   4,  0,  0,   0,  0,  TV_NTSC,   0,-1 }, // 6-in-1 (from Pirate Ninja Cat)
	{ 0x071D605F,   4,  0,  0,   0,  0,  TV_NTSC,   0,-1 }, // 7-in-1 (223) (Snow Bros)
	{ 0x9065F46F,   4,  0,  0,   0,  0,  TV_NTSC,   0,-1 }, // 7-in-1 (Chip to Dale 2)
	{ 0xA7CBAE89,   4,  0,  0,   0,  0,  TV_NTSC,   0,-1 }, // Game 4-in-1 (in Gun-Nac)
	{ 0x4F9287FE,   4,  0,  0,   0,  0,  TV_NTSC,   0,-1 }, // Super 8-in-1 (in Pokemon Blue)
	{ 0x2C8C7FC7,   4,  0,  0,   0,  0,  TV_NTSC,   0,-1 }, // Super 8-in-1 (Pokemon Golden)
	{ 0xD54BF8BD,   4,  0,  0,   0,  0,  TV_NTSC,   0,-1 }, // Super 8-in-1 Super Mario Kart Rider
	{ 0xCD60753C,   2,  0,  0,   0,  7,  TV_NTSC,   0,-1 }, // Jumbo 8-in-1 (Green Beret)
	{ 0xCE69C722,   2,  0,  0,   0,  7,  TV_NTSC,   0,-1 }, // Well 7-in-1 (AB150) (from Pirate Mitsume ga Tooru)
	{ 0x3462AA7F,   4,  0,  0,   0,  0,  TV_NTSC,   0,-1 }, // 영재컴 꼬꼬마 Pack 2
	{ 0x64130AD8, 116,  0,  0,   0,  0,  TV_NTSC,   0,-1 }, // AV美少女戰士
	{ 0x6B751B85, 147,  0,  0,   0,  0,  TV_NTSC,   0,-1, 0x91440AAB }, // Chinese Kungfu: 少林武者
	{ 0x52C78553, 148,  0,  0,   0,  0,  TV_NTSC,   0,-1, 0xE8829001 }, // Soap Panic
	{ 0xB62CFE50,   6,  0,  4,   7,  9,  TV_NTSC,   0,-1 }, // Dragon Ball 3 (FFE)
	// Nesticle MMC3 hacks
	{ 0x274CB4C6, 100,  0,  3,0x70,  0,  TV_NTSC,   0, 0 }, // Digital Devil Story: 女神転生II
	// Games that require bus conflicts to be emulated
	{ 0x34D5FC6E,   3,  2,  1,   0,  0,  TV_NTSC,   0, 0 }, // Cybernoid: The Fighting Machine
	{ 0x7ED91F80, 144,  0,  1,   0,  0,  TV_DUAL,   0, 0 }, // Death Race
	// Namco 175
	{ 0x8E1E1181, 210,  1,  1,   0,  0,  TV_NTSC,   0, 0 }, // Famista '91
	{ 0x86ADC177, 210,  1,  1,   0,  0,  TV_NTSC,   0, 0 }, // Famista '91
	{ 0xB62A7B71, 210,  1,  3,0x50,  0,  TV_NTSC,   0, 0 }, // Family Circuit '91
	{ 0x0B4C2DDC, 210,  1,  1,   0,  0,  TV_NTSC,   0, 0 }, // Chibi Maruko-chan: Uki Uki Shopping
	{ 0xDA8B5993, 210,  1,  1,   0,  0,  TV_NTSC,   0, 0 }, // Heisei Tensai Bakabon
	{ 0x08124F85, 210,  1,  1,   0,  0,  TV_NTSC,   0, 0 }, // Heisei Tensai Bakabon
	{ 0x7F5DAE94, 210,  1,  1,   0,  0,  TV_NTSC,   0, 0 }, // Heisei Tensai Bakabon
	{ 0xABCE0B65, 210,  1,  1,   0,  0,  TV_NTSC,   0, 0 }, // Heisei Tensai Bakabon
	// Namco 340
	{ 0xD666560F, 210,  2,  0,   0,  0,  TV_NTSC,   0, 0 }, // Splatterhouse - Wanpaku Graffiti
	{ 0x4933C083, 210,  2,  0,   0,  0,  TV_NTSC,   0, 0 }, // Splatterhouse - Wanpaku Graffiti
	{ 0xF9965F43, 210,  2,  0,   0,  0,  TV_NTSC,   0, 0 }, // Splatterhouse - Wanpaku Graffiti
	{ 0x5D071A9B, 210,  2,  0,   0,  0,  TV_NTSC,   0, 0 }, // Splatterhouse - Wanpaku Graffiti
	{ 0x3D7A063A, 210,  2,  0,   0,  0,  TV_NTSC,   0, 0 }, // Splatterhouse - Wanpaku Graffiti
	{ 0x0481072F, 210,  2,  0,   0,  0,  TV_NTSC,   0, 0 }, // Splatterhouse - Wanpaku Graffiti
	{ 0xC3C53462, 210,  2,  0,   0,  0,  TV_NTSC,   0, 0 }, // Splatterhouse - Wanpaku Graffiti
	{ 0xD7F1EE7A, 210,  2,  0,   0,  0,  TV_NTSC,   0, 0 }, // Splatterhouse - Wanpaku Graffiti
	{ 0x1BD334E4, 210,  2,  0,   0,  0,  TV_NTSC,   0, 0 }, // Splatterhouse - Wanpaku Graffiti
	{ 0x71670B09, 210,  2,  0,   0,  0,  TV_NTSC,   0, 0 }, // Splatterhouse - Wanpaku Graffiti
	{ 0xCE681AC3, 210,  2,  0,   0,  0,  TV_NTSC,   0, 0 }, // Splatterhouse - Wanpaku Graffiti
	{ 0x02738C68, 210,  2,  0,   0,  0,  TV_NTSC,   0, 0 }, // Wagyan Land 2
	{ 0xA18D8596, 210,  2,  0,   0,  0,  TV_NTSC,   0, 0 }, // Wagyan Land 2
	{ 0x131FECD8, 210,  2,  0,   0,  0,  TV_NTSC,   0, 0 }, // Wagyan Land 2
	{ 0xFDBCA5D1, 210,  2,  0,   0,  0,  TV_NTSC,   0, 0 }, // Famista '92
	{ 0x507E6E5E, 210,  2,  0,   0,  0,  TV_NTSC,   0, 0 }, // Famista '92
	{ 0x650353F7, 210,  2,  0,   0,  0,  TV_NTSC,   0, 0 }, // Famista '93
	{ 0x3940B0F9, 210,  2,  0,   0,  0,  TV_NTSC,   0, 0 }, // Famista '94
	{ 0x8BC2EC0C, 210,  2,  0,   0,  0,  TV_NTSC,   0, 0 }, // Dream Master
	{ 0xD61A4D85, 210,  2,  0,   0,  0,  TV_NTSC,   0, 0 }, // Dream Master
	{ 0xFDBC0A6D, 210,  2,  0,   0,  0,  TV_NTSC,   0, 0 }, // Dream Master
	{ 0x0D2A2C55, 210,  2,  0,   0,  0,  TV_NTSC,   0, 0 }, // Dream Master
	{ 0x9227180C, 210,  2,  0,   0,  0,  TV_NTSC,   0, 0 }, // Top Striker
	{ 0x14942C06, 210,  2,  0,   0,  0,  TV_NTSC,   0, 0 }, // Wagyan Land 3
	{ 0x0346543D, 210,  2,  0,   0,  0,  TV_NTSC,   0, 0 }, // Wagyan Land 3
	// Ambiguous discrete logic
	{ 0x42392440,  78,  1,  0,   0,  0,  TV_NTSC,   0, 0 }, // Uchûsen Cosmo Carrier
	{ 0xCECE4CFC,  78,  1,  0,   0,  0,  TV_NTSC,   0, 0 }, // Uchûsen Cosmo Carrier
	{ 0xBC1197A4,  78,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Holy Diver
	{ 0xBFCAA5F4,  78,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Holy Diver
	{ 0x6B878F5A,  78,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Holy Diver
	{ 0x685CBD0A,  78,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Holy Diver
	{ 0x4A01106A,  78,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Holy Diver
	{ 0x2CB5B110,  78,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Holy Diver
	{ 0x10BB8F9A,  70,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // Family Trainer: Manhattan Police
	{ 0xA59CA2EF,  70,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // Kamen Rider Club
	{ 0x0CD00488,  70,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Space Shadow
	{ 0xAD3DF455,  70,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // Family Trainer: Meiro Daisakusen
	{ 0x48F8D55E, 152,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Gegege no Kitarô 2
	{ 0x63CE1749, 152,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Gegege no Kitarô 2
	{ 0x97D0A625, 152,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Gegege no Kitarô 2
	{ 0x6267FBD1, 152,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Arkanoid 2
	{ 0x26B1866F, 152,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Arkanoid 2
	{ 0x5347741E, 152,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Arkanoid 2
	{ 0xDA71D69E, 152,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Arkanoid 2
	{ 0x8D310BC7, 152,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Arkanoid 2
	{ 0x5E653533, 152,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Arkanoid 2
	{ 0x8CE99FE4, 152,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Pocket Zaurus
	{ 0x3BF15767, 152,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Saint Seiya: Ôgon Densetsu
	{ 0xE6835FC3, 152,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Saint Seiya: Ôgon Densetsu
	{ 0xFFFFFFFF, 152,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, //
	{ 0xFFFFFFFF, 152,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, //
	{ 0xFFFFFFFF, 152,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, //
	{ 0xFFFFFFFF, 152,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, //
	{ 0xFFFFFFFF, 152,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, //
	{ 0xFFFFFFFF, 152,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, //
	{ 0xFFFFFFFF, 152,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, //
	{ 0xFFFFFFFF, 152,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, //
	{ 0xFFFFFFFF, 152,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, //
	{ 0xFFFFFFFF, 152,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, //
	{ 0xFFFFFFFF, 152,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, //
	{ 0xFFFFFFFF, 152,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, //
	{ 0xFFFFFFFF, 152,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, //
	// Codemasters
	{ 0x6C93377C,  71,  0,  1,   0,  7,  TV_DUAL,   0, 0 }, // Bee 52
	{ 0x79705A91,  71,  0,  1,   0,  7,  TV_DUAL,   0, 0 }, // Bee 52
	{ 0x1BC686A8,  71,  1,  1,   0,  7,  TV_DUAL,   0, 0 }, // Fire Hawk
	{ 0xFDDBB3B3,  71,  1,  1,   0,  7,  TV_DUAL,   0, 0 }, // Fire Hawk
	{ 0xBD154C3E,  71,  0,  1,   0,  7,  TV_DUAL,   0, 0 }, // Bignose the Caveman
	{ 0x90D3210A,  71,  0,  1,   0,  7,  TV_DUAL,   0, 0 }, // Bignose the Caveman
	{ 0x227BFBFD,  71,  0,  1,   0,  7,  TV_DUAL,   0, 0 }, // Bignose the Caveman
	{ 0xEB498D70,  71,  0,  1,   0,  7,  TV_DUAL,   0, 0 }, // Bignose the Caveman
	{ 0xCCDCBFC6,  71,  0,  1,   0,  7,  TV_DUAL,   0, 0 }, // Bignose Freaks Out
	{ 0x5B2B72CB,  71,  0,  1,   0,  7,  TV_DUAL,   0, 0 }, // Bignose Freaks Out
	{ 0x87E91AD6,  71,  0,  1,   0,  7,  TV_DUAL,   0, 0 }, // Bignose Freaks Out
	{ 0xF62B0327,  71,  0,  1,   0,  7,  TV_DUAL,   0, 0 }, // Bignose Freaks Out
	{ 0x514770A0,  71,  0,  1,   0,  7,  TV_DUAL,   0, 0 }, // BMX Simulator
	{ 0x40E1F09E,  71,  0,  1,   0,  7,  TV_DUAL,   0, 0 }, // BMX Simulator
	{ 0xC469EC55,  71,  0,  1,   0,  7,  TV_DUAL,   0, 0 }, // BMX Simulator
	{ 0x2EAFD5A9,  71,  0,  1,   0,  7,  TV_DUAL,   0, 0 }, // Mystery World Dizzy
	{ 0x4F74E236,  71,  0,  1,   0,  7,  TV_DUAL,   0, 0 }, // Wonderland Dizzy
	{ 0xDB99D0CB,  71,  0,  1,   0,  7,  TV_DUAL,   0, 0 }, // Dizzy the Adventurer
	{ 0xD8EB3620,  71,  0,  1,   0,  7,  TV_DUAL,   0, 0 }, // Dizzy the Adventurer
	{ 0x7499DA57,  71,  0,  1,   0,  7,  TV_DUAL,   0, 0 }, // Dizzy the Adventurer
	{ 0x58609357,  71,  0,  1,   0,  7,  TV_DUAL,   0, 0 }, // Dizzy the Adventurer
	{ 0xA20B307D,  71,  0,  1,   0,  7,  TV_DUAL,   0, 0 }, // Dizzy the Adventurer
	{ 0xFA81E1C0,  71,  0,  1,   0,  7,  TV_DUAL,   0, 0 }, // Dizzy the Adventurer
	{ 0x38FBCC85,  71,  0,  1,   0,  7,  TV_DUAL,   0, 0 }, // The Fantastic Adventures of Dizzy
	{ 0x465F2363,  71,  0,  1,   0,  7,  TV_DUAL,   0, 0 }, // The Fantastic Adventures of Dizzy
	{ 0xC6EAEFA1,  71,  0,  1,   0,  7,  TV_DUAL,   0, 0 }, // The Fantastic Adventures of Dizzy
	{ 0xA7E32B78,  71,  0,  1,   0,  7,  TV_DUAL,   0, 0 }, // The Fantastic Adventures of Dizzy
	{ 0x11ADEB8E,  71,  0,  1,   0,  7,  TV_DUAL,   0, 0 }, // The Fantastic Adventures of Dizzy
	{ 0xF732C8FD,  71,  0,  1,   0,  7,  TV_DUAL,   0, 0 }, // The Fantastic Adventures of Dizzy
	{ 0x2F96D152,  71,  0,  1,   0,  7,  TV_DUAL,   0, 0 }, // The Fantastic Adventures of Dizzy
	{ 0x8C057E35,  71,  0,  1,   0,  7,  TV_DUAL,   0, 0 }, // The Fantastic Adventures of Dizzy
	{ 0x9429B2F6,  71,  0,  1,   0,  7,  TV_DUAL,   0, 0 }, // The Fantastic Adventures of Dizzy
	{ 0x63D38B86,  71,  0,  1,   0,  7,  TV_DUAL,   0, 0 }, // Dreamworld Pogie
	{ 0xE61C82E6,  71,  0,  1,   0,  7,  TV_DUAL,   0, 0 }, // Dreamworld Pogie
	{ 0x9E379698,  71,  0,  1,   0,  7,  TV_DUAL,   0, 0 }, // Linus Spacehead's Cosmic Crusade
	{ 0xCECBE5F6,  71,  0,  1,   0,  7,  TV_DUAL,   0, 0 }, // Linus Spacehead's Cosmic Crusade
	{ 0x4985E8C8,  71,  0,  1,   0,  7,  TV_DUAL,   0, 0 }, // Linus Spacehead's Cosmic Crusade
	{ 0x70F31D2C,  71,  0,  1,   0,  7,  TV_DUAL,   0, 0 }, // Linus Spacehead's Cosmic Crusade
	{ 0x24BA12DD,  71,  0,  1,   0,  7,  TV_DUAL,   0, 0 }, // Micro Machines
	{ 0x3C33ECC2,  71,  0,  1,   0,  7,  TV_DUAL,   0, 0 }, // Micro Machines
	{ 0xB7D37277,  71,  0,  1,   0,  7,  TV_DUAL,   0, 0 }, // Micro Machines
	{ 0x9235B57B,  71,  0,  1,   0,  7,  TV_DUAL,   0, 0 }, // Micro Machines
	{ 0x6B523BD7,  71,  0,  1,   0,  7,  TV_DUAL,   0, 0 }, // Micro Machines
	{ 0xDB1FD64E,  71,  0,  1,   0,  7,  TV_DUAL,   0, 0 }, // Micro Machines
	{ 0xE62E3382,  71,  0,  1,   0,  7,  TV_DUAL,   0, 0 }, // MiG 29 Soviet Fighter
	{ 0xF0D55256,  71,  0,  1,   0,  7,  TV_DUAL,   0, 0 }, // MiG 29 Soviet Fighter
	{ 0x3A990EE0,  71,  0,  1,   0,  7,  TV_DUAL,   0, 0 }, // Stunt Kids
	{ 0x3E81DD67,  71,  0,  1,   0,  7,  TV_DUAL,   0, 0 }, // Super Robin Hood
	{ 0xD793EB0A,  71,  0,  1,   0,  7,  TV_DUAL,   0, 0 }, // Super Robin Hood
	{ 0x3C97F7E4,  71,  0,  1,   0,  7,  TV_DUAL,   0, 0 }, // Super Robin Hood
	{ 0xA4D96251,  71,  0,  1,   0,  7,  TV_DUAL,   0, 0 }, // Super Robin Hood
	{ 0x892434DD,  71,  0,  1,   0,  7,  TV_DUAL,   0, 0 }, // Ultimate Stuntman
	{ 0x274008A9,  71,  0,  1,   0,  7,  TV_DUAL,   0, 0 }, // Ultimate Stuntman
	{ 0xB89888C9, 232,  0,  1,   0,  7,  TV_DUAL,   0, 0 }, // Quattro Adventure
	{ 0x6A7BF037, 232,  1,  1,   0,  7,  TV_DUAL,   0, 0 }, // Super Adventure Quests
	{ 0x792070A9, 232,  0,  1,   0,  7,  TV_DUAL,   0, 0 }, // Quattro Arcade
	{ 0xED58D1F4, 232,  0,  1,   0,  7,  TV_DUAL,   0, 0 }, // Quattro Arcade
	{ 0x69145253, 232,  0,  1,   0,  7,  TV_DUAL,   0, 0 }, // Quattro Arcade
	{ 0xCCCAF368, 232,  0,  1,   0,  7,  TV_DUAL,   0, 0 }, // Quattro Sports v1
	{ 0xA045FE1D, 232,  1,  1,   0,  7,  TV_DUAL,   0, 0 }, // Quattro Sports v2
	{ 0xB462718E, 232,  0,  1,   0,  7,  TV_DUAL,   0, 0 }, // Quattro Sports v3
	{ 0x62EF6C79, 232,  1,  1,   0,  7,  TV_DUAL,   0, 0 }, // Quattro Sports
	{ 0x4B40CBD9, 232,  1,  1,   0,  7,  TV_DUAL,   0, 0 }, // Pegasus 4-in-1
	{ 0x6096F84E, 104,  0,  1,   0,  7,  TV_DUAL,   0, 0 }, // Pegasus 5-in-1
	// SunSoft 4
	{ 0xB938B7E9,  68,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // After Burner
	{ 0x39E8DCBE,  68,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // After Burner
	{ 0x88F202F0,  68,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // After Burner
	{ 0x72125764,  68,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // After Burner
	{ 0x88D7753C,  68,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // After Burner
	{ 0x99B5992F,  68,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // After Burner
	{ 0xFDE79681,  68,  0,  2,0x70,  0,  TV_NTSC,   0, 0 }, // Maharaja
	{ 0xFBB57D03,  68,  0,  2,0x70,  0,  TV_NTSC,   0, 0 }, // Maharaja
	{ 0xBDDBC3B0,  68,  1,  2,0x70,  0,  TV_NTSC,   0, 0 }, // Nantettatte!! Baseball
	{ 0x82C96242,  68,  1,  2,0x70,  0,  TV_NTSC,   0, 0 }, // Nantettatte!! Baseball
	{ 0x1F9A8904,  68,  1,  2,0x70,  0,  TV_NTSC,   0, 0 }, // Nantettatte!! Baseball
	{ 0x1A23F270,  68,  1,  2,0x70,  0,  TV_NTSC,   0, 0 }, // Nantettatte!! Baseball
	{ 0x3B9E30F1,  68,  1,  2,0x70,  0,  TV_NTSC,   0, 0 }, // Nantettatte!! Baseball
	// NINA-001
	{ 0x3A5CC3FA,  34,  1,  0,   7,  0,  TV_NTSC,   0, 0 }, // Impossible Mission II
	{ 0x2F2404C5,  34,  1,  0,   7,  0,  TV_NTSC,   0, 0 }, // Impossible Mission II
	// BNROM
	{ 0xC2730C30,  34,  2,  0,   0,  7,  TV_NTSC,   0, 0 }, // Deadly Towers
	{ 0xA21E675C,  34,  2,  0,   0,  7,  TV_NTSC,   0, 0 }, // Mashô
	{ 0x2D7BF9A3,  34,  2,  0,   0,  7,  TV_NTSC,   0, 0 }, // Mashô
	{ 0x5030BCA8,  34,  2,  0,   0,  7,  TV_NTSC,   0, 0 }, // Mashô
	{ 0x9EA7EDFE,  34,  2,  0,   0,  7,  TV_NTSC,   0, 0 }, // Mashô
	// IREM-G101
	{ 0x726EDB66,  32,  0,  0,   7,  0,  TV_NTSC,   0, 0 }, // Image Fight
	{ 0xD2C55CB1,  32,  0,  0,   7,  0,  TV_NTSC,   0, 0 }, // Image Fight
	{ 0xB8FD3059,  32,  0,  0,   7,  0,  TV_NTSC,   0, 0 }, // Image Fight
	{ 0x35E65A8C,  32,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Kaiketsu Yanchamaru 2
	{ 0xD6155371,  32,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Kaiketsu Yanchamaru 2
	{ 0x479084EF,  32,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Kaiketsu Yanchamaru 2
	{ 0x9BD80595,  32,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Kaiketsu Yanchamaru 2
	{ 0x4E890104,  32,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Kaiketsu Yanchamaru 2
	{ 0x4E890104,  32,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Kaiketsu Yanchamaru 2
	{ 0x222BD030,  32,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Kaiketsu Yanchamaru 2
	{ 0xC0FED437,  32,  1,  0,   0,  0,  TV_NTSC,   0, 0 }, // Major League
	{ 0x788BED9A,  32,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Meikyûjima
	{ 0x48DFA165,  32,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Meikyûjima
	{ 0xEB807320,  32,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Paaman
	{ 0x336A3CD5,  32,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Paaman
	{ 0xDBA3A02E,  32,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Paaman 2
	{ 0xD272B57D,  32,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Paaman 2
	{ 0x021FDD7D,  32,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Paaman 2
	{ 0x0BCEC82E,  32,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Paaman 2
	// Namco 109 instead of MMC3
	{ 0x899A0067, 206,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Babel no Tô
	{ 0xFAE3E03C, 206,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Babel no Tô
	{ 0x55ACB1E0, 206,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Babel no Tô
	{ 0x33BD809C, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // Dragon Buster II
	{ 0xD0E26539, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // Dragon Buster II
	{ 0x5A48921D, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // Dragon Buster II
	{ 0x306EBBDE, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // Dragon Slayer IV
	{ 0x1EE8BA0F, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // Dragon Slayer IV
	{ 0x21D16321, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // Dragon Slayer IV
	{ 0x123844D3, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // Dragon Slayer IV
	{ 0x3598238F, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // Family Boxing
	{ 0xDCEA8DC4, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // Family Boxing
	{ 0x4EBE8A31, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // Ring King
	{ 0xDC170570, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // Ring King
	{ 0x8D171E47, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // Ring King
	{ 0xA50C51FB, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // Family Circuit
	{ 0xAC75F8CD, 206,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Family Jockey
	{ 0xA941EEFA, 206,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Family Mahjong
	{ 0xB43BF74E, 206,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Family Mahjong
	{ 0x01656B0A, 206,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Family Mahjong
	{ 0x3AE06D19, 206,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Family Mahjong II
	{ 0x7A8AE523, 206,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Family Pinball
	{ 0x481608A9, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // Family Tennis
	{ 0xBDBAE3E0, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // Family Stadium '89
	{ 0x75B1336B, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // Family Stadium '89
	{ 0x5DD90C5F, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // Family Stadium '89
	{ 0x3D96A1D8, 206,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Fantasy Zone (Tengen)
	{ 0x834D1924, 206,  0,  8,   0,  0,  TV_NTSC,   0, 0 }, // Gauntlet
	{ 0xA6A2EC56, 206,  0,  8,   0,  0,  TV_NTSC,   0, 0 }, // Gauntlet
	{ 0xCA2C4E7F, 206,  0,  8,   0,  0,  TV_NTSC,   0, 0 }, // Gauntlet
	{ 0xF8FD1FFE, 206,  0,  8,   0,  0,  TV_NTSC,   0, 0 }, // Gauntlet
	{ 0x52C851FB, 206,  0,  8,   0,  0,  TV_NTSC,   0, 0 }, // Gauntlet
	{ 0xB19C48A5, 206,  0,  8,   0,  0,  TV_NTSC,   0, 0 }, // Gauntlet
	{ 0x6529E911, 206,  0,  8,   0,  0,  TV_NTSC,   0, 0 }, // Gauntlet
	{ 0x392B98ED, 206,  0,  8,   0,  0,  TV_NTSC,   0, 0 }, // Gauntlet
	{ 0x13EC1C90, 206,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Genpei Tômaden
	{ 0x002C9072, 206,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Genpei Tômaden
	{ 0x651700A5, 206,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Indiana Jones and the Temple of Doom
	{ 0xAEB294CA, 206,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Indiana Jones and the Temple of Doom
	{ 0x627E2BE4, 206,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Indiana Jones and the Temple of Doom
	{ 0x5CEB1256, 206,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Indiana Jones and the Temple of Doom
	{ 0x4970105E, 206,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Indiana Jones and the Temple of Doom
	{ 0xF9B4C729, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // Debias
	{ 0x6869FE66, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // Debias
	{ 0x2FA32583, 206,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Karnov
	{ 0x198237C5, 206,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Karnov
	{ 0x3208B3A2, 206,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Karnov
	{ 0x6026FE2C, 206,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Karnov
	{ 0x7E3B5B41, 206,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Karnov
	{ 0x2274F0EB, 206,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Karnov
	{ 0x1455E2AD, 206,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Karnov
	{ 0x152C567F, 206,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Karnov
	{ 0xA37B0EE3, 206,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Karnov
	{ 0x359C3631, 206,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Karnov
	{ 0x986BB2D4, 206,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Karnov
	{ 0xE0B5B4E2, 206,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Karnov
	{ 0x569BCB5D, 206,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Karnov
	{ 0x15249A59, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // Ki no Bôken
	{ 0x70BA378C, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // Ki no Bôken
	{ 0xD6BB171E, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // Ki no Bôken
	{ 0x11B038FE, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // Lupin Sensei
	{ 0xED2E02B5, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // Lupin Sensei
	{ 0xD6B8199B, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // Lupin Sensei
	{ 0xDF7FFD81, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // Lupin Sensei
	{ 0x2652DE66, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // Lupin Sensei
	{ 0x15BE6E58, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // Mappy-Land
	{ 0xBDABC4A6, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // Metro-Cross
	{ 0xC7D56EC0, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // Metro-Cross
	{ 0x02ED6298, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // Pac-Mania
	{ 0xC77D8E36, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // Pac-Mania
	{ 0xBA0363ED, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // Pac-Mania
	{ 0x18C0623B, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // Family Stadium
	{ 0x203D32B5, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0x4DA98F98, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0x42607A97, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0xDFB33C48, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0x8CAE7DD6, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0x68F1EB38, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0xA63FEC79, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0x0AA97C6D, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0xC4677B2C, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0xEABA684F, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0x24746F0E, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0x2235D834, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0xECFBDF75, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0x02BC97AE, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0xCC7290EF, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0x46C8B03C, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0x8806B77D, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0x06C97BF4, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0xC8077CB5, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0x65085E33, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0xBA46F9C3, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0x3C4DA9B0, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0xD13DB0DB, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0x0DB043AB, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0xD009B923, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0xF90DC782, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0xFF105CC4, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0xB0F7BBBB, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0xE3C392A2, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0x8D47C314, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0x8E7A6D90, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0xE3B47FC8, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0xC9FD20FE, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0x72E231B9, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0x8A781BBA, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0xDEEB8C02, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0xB4AC878D, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0xF9136A65, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0xD63AD5E1, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0xB2E70500, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0x19AF7208, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0x733E5038, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0x17145971, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0x31575EBD, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0x4E46362D, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0xD26B721A, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0x6C770EAF, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0x0A93708C, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0x38671522, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0x743D7592, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0xA24AAA93, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0x81DBD1FF, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0x7E06469F, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0x9D14BC5B, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0x74F9D3A5, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0x27BA7D96, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0xC7A8EA4A, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0x00E900F5, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0x130485A1, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0x64B3E4CD, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0x2CFDB374, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0x96AB5D85, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0xD003AAA2, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0xD85BD21D, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0x0D879305, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0x1C27F936, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0x67AEDEF2, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0x3F2239F0, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0xF8B50E54, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0xBC7B16FA, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0x8D39D5E2, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0xD0852334, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0xA8BD32AA, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0x33436770, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0xE3508FE1, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0x2D9E88A0, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0x20F8FC55, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0x6F839FD0, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0x4589F47E, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0x8E51E1B1, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0x093DD767, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0x788EBDEA, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0x74BABC81, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0xED92D53D, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0x43FB14A7, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0xF0DD1B89, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0xA2690C71, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0x7A956449, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0x19DAFB0E, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0xB6B47EF2, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0x336CE630, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0xFDA2E171, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0xC949C81E, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0x0787CF5F, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball
	{ 0xE8B7C767, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // Family Stadium '87
	{ 0xC993A386, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // Family Stadium '87
	{ 0x7E4966E4, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // Family Stadium '88
	{ 0x093311AA, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball 2
	{ 0x18B9669E, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball 2
	{ 0x760409CD, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball 2
	{ 0xBAB897FE, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball 3
	{ 0x5F30FCD8, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball 3
	{ 0x5123E0F0, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball 3
	{ 0x5F9368EB, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball 3
	{ 0xF7EA3BDA, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // R.B.I. Baseball 3
	{ 0x58581770, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // LaSalle Ishii no Child's Quest
	{ 0x61F96440, 206,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Sanma no Meitantei
	{ 0xB3DBC08F, 206,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Sanma no Meitantei
	{ 0x2959C0EE, 206,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Sanma no Meitantei
	{ 0x8A94F41C, 206,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Side Pocket
	{ 0x607DB27A, 206,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Side Pocket
	{ 0x816E127D, 206,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Side Pocket
	{ 0x90FAA618, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // Sky Kid
	{ 0xAABD9624, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // Sky Kid
	{ 0x51C7C66A, 206,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Super Chinese
	{ 0x314790C9, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // Super Sprint
	{ 0x88E6754D, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // Super Sprint
	{ 0xD5883D6B, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // Super Sprint
	{ 0x4D637285, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // Super Sprint
	{ 0xAA19534D, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // Super Sprint
	{ 0x5C163FDE, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // Super Sprint
	{ 0x4D21172C, 206,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Super Xevious
	{ 0x7C854039, 206,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Super Xevious
	{ 0x47F1FD34, 206,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Super Xevious
	{ 0x910CC30F, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // Keru Naguru
	{ 0x9B9AAE4D, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // Keru Naguru
	{ 0x05D70600, 206,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Toobin'
	{ 0x028912BA, 206,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Toobin'
	{ 0x7C7AB58E, 206,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Valkyrie no Densetsu
	{ 0x374448C5, 206,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Valkyrie no Densetsu
	{ 0x071F8421, 206,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Valkyrie no Densetsu
	{ 0x4F3B8767, 206,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Valkyrie no Densetsu
	{ 0x4F3B8767, 206,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Valkyrie no Densetsu
	{ 0xAB19FFAE, 206,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Valkyrie no Densetsu
	{ 0x6E72B8FF, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // Vindicators
	{ 0xA459E9E9, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // Vindicators
	{ 0x4D5E870C, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // Vindicators
	{ 0x72D98595, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // Vindicators
	{ 0xB097F651, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // Wagyan Land
	{ 0x1C10527C, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // Wagyan Land
	{ 0x27100217, 206,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // Wagyan Land
	// Namco 109 with single-screen mirroring
	{ 0xC1B6B2A6, 154,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Devil Man
	{ 0xB13A5D9D, 154,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Devil Man
	// Wrong Mirroring or Mapper
	{ 0x78F00865, 190,  0,  1,   7,  0,  TV_NTSC,   0, 0 }, // Magic Kid Goo Goo
	{ 0x22A0641D,   6,  1,  5,   8,  9,  TV_NTSC,   0, 0 }, // Fantasy Zone II (FFE)
	{ 0xC9ACEC1D,   2,  0,  1,   0,  7,  TV_NTSC,   0, 0 }, // Momotarou Densetsu
	{ 0x7778ABA8,   2,  0,  1,   0,  7,  TV_NTSC,   0, 0 }, // Pachio-Kun (hM02)
	{ 0xC346B2CC,   2,  0,  1,   0,  7,  TV_NTSC,   0, 0 }, // Pachio-Kun 2 (hM02)
	{ 0x5012D5D0,   2,  0,  1,   0,  7,  TV_NTSC,   0, 0 }, // Ripple Island
	{ 0x42F98D61,   1,  0,  2,0x70,  7,  TV_NTSC,   0, 0 }, // Deep Dungeon III (not FFE as claimed)
	{ 0x6C30AE88,   6,  1,  1,   8,  9,  TV_NTSC,   0, 0 }, // Contra (FFE)
	{ 0xCCA24CAF,   6,  1,  1,   8,  9,  TV_NTSC,   0, 0 }, // Dragon Scroll (FFE)
	{ 0xDF5CA492,   6,  1,  1,   8,  9,  TV_NTSC,   0, 0 }, // Dragon Scroll (FFE)
	{ 0xC92AA406,   6,  1,  1,   8,  9,  TV_NTSC,   0, 0 }, // Dragon Scroll (FFE)
	// FFE dumps
	{ 0x380FA9AB,   6,  0,  4,   8,  9,  TV_NTSC,   0, 10}, // Jovial Race
	{ 0x793F59C6,   6,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Mi Hun Che
	{ 0xFE820BAF,   6,  6,  0,   0,  0,  TV_NTSC,   0, 0 }, // Mi Hun Che
	{ 0xE47FC4A3,   6,  0,  6,0x76,  9,  TV_NTSC,   0, 12}, // がんばれゴエモンが外伝
	{ 0x94E29FB8,   6,  4,  4,   8,  7,  TV_NTSC,   0, 0 }, // Bio Senshi Dan
	{ 0xA0DDF884,   6,  0,  1,   8,  7,  TV_NTSC,   0, 0 }, // Bomberman II
	{ 0xB382AEA4,   6,  0,  1,   8,  7,  TV_NTSC,   0, 0 }, // Bomberman II
	{ 0xDF776483,   6,  0,  4,   8,  9,  TV_NTSC,   0, 11}, // Dragon Ball: Daimao Fukkatsu
	{ 0x8EBD788F,   6,  0,  4,   8,  9,  TV_NTSC,   0, 0 }, // Super Chinese 2
	/*{ 0x0EA3AF89,   6,  8,  5,   8,  0,  TV_NTSC,   0, 6 }, // Mahjong (Kaiser)	
	{ 0xF2594374,   4,  0,  4,   8,  0,  TV_NTSC,   0, 7 }, // Matendouji
	{ 0x22D6D5BD, 206,  0,  5,   8,  0,  TV_NTSC,   0, 7 }, // Debias
	{ 0x9D21FE96, 206,  0,  5,   8,  0,  TV_NTSC,   0, 7 }, // Lupin Sansei
	{ 0x07589B66,   1,  0,  4,   8,  0,  TV_NTSC,   0, 8 }, // Sky Shark
	{ 0x25519E6E, 148,  0,  5,   8,  0,  TV_NTSC,   0, 9 }, // Pyramid
	{ 0x27F77729,   6,  2,  1,   8,  7,  TV_NTSC,   0, 0 }, // Salamander
	{ 0x072CD567,   6,  2,  1,   8,  7,  TV_NTSC,   0, 0 }, // Salamander
	{ 0xA9E30826,   6,  2,  1,   8,  7,  TV_NTSC,   0, 0 }, // Salamander
	{ 0xCDEF1C0D,   6,  2,  0,   8,  7,  TV_NTSC,   0, 0 }, // 100 Man $ Kid
	{ 0x1D9D0BD4,   6,  2,  4,   8,  7,  TV_NTSC,   0, 0 }, // Arctic
	{ 0x86FCE96E,   6,  2,  4,   8,  7,  TV_NTSC,   0, 0 }, // Arctic
	{ 0xE66E59A0,   6,  2,  2,0x76,  7,  TV_NTSC,   0, 0 }, // Bikkuriman World
	{ 0xA65D3207,   6,  2,  2,0x76,  7,  TV_NTSC,   0, 0 }, // Deep Dungeon III
	{ 0x256441EB,   6,  2,  2,0x76,  7,  TV_NTSC,   0, 0 }, // Bloody Warriors
	{ 0xD34A15D9,   6,  2,  4,   8,  9,  TV_NTSC,   0, 0 }, // Cadillac
	{ 0x70DB619A,   6,  0,  4,   8,  9,  TV_NTSC,   0, 0 }, // Space Boy
	{ 0xA4D64275,   6,  5,  5,   8,  9,  TV_NTSC,   0, 0 }, // Atlantis no Nazo
	{ 0x8FAAEF86,   6,  0,  4,   8,  7,  TV_NTSC,   0, 0 }, // Densetsu no Kishi Elrond
	{ 0x7090B851,   6,  2,  2,0x76,  7,  TV_NTSC,   0, 0 }, // Dragon Quest III
	{ 0x7132CAF3,   6,  2,  1,   8,  7,  TV_NTSC,   0, 0 }, // Dragon Unit
	{ 0xAB07C7F3,   6,  2,  2,0x76,  7,  TV_NTSC,   0, 0 }, // Final Fantasy
	{ 0x62F3D761,   6,  2,  2,0x76,  7,  TV_NTSC,   0, 0 }, // Final Fantasy
	{ 0xEC04F0DA,   6,  2,  1,   8,  7,  TV_NTSC,   0, 0 }, // Final Fantasy II
	{ 0x053C4699,   6,  2,  0,   8,  7,  TV_NTSC,   0, 0 }, // Gambler Jiko Chuushin Ha
	{ 0x0689965D,   6,  2,  0,   8,  7,  TV_NTSC,   0, 0 }, // Garfield
	{ 0x5D853F02,   6,  2,  0,   8,  7,  TV_NTSC,   0, 0 }, // Gekitotsu Yonku Battle
	{ 0x9C015583,   6,  2,  1,   8,  7,  TV_NTSC,   0, 0 }, // Hanjuku Hero
	{ 0xA657C2A2,   6,  2,  1,   8,  7,  TV_NTSC,   0, 0 }, // Hanjuku Hero
	{ 0x24A0AA1A,   6,  2,  0,   8,  7,  TV_NTSC,   0, 0 }, // Highway Star
	{ 0x3BCEBB61,   6,  2,  4,   8,  7,  TV_NTSC,   0, 0 }, // Ide Yousuke no Jissen Mahjong
	{ 0x0446282F,   6,  2,  1,   8,  7,  TV_NTSC,   0, 0 }, // Highway Star (FMG)
	{ 0x694C5297,   6,  2,  2,0x76,  7,  TV_NTSC,   0, 0 }, // Hokuto no Ken 4
	{ 0x6DE472F4,   6,  2,  2,0x76,  7,  TV_NTSC,   0, 0 }, // Hototogisu
	{ 0xA875C5E9,   6,  2,  0,   8,  7,  TV_NTSC,   0, 0 }, // Ikari II
	{ 0xE8973B45,   6,  2,  2,0x76,  7,  TV_NTSC,   0, 0 }, // Indora no Hikari
	{ 0x32979126,   6,  2,  0,   8,  7,  TV_NTSC,   0, 0 }, // Jesus (!!!)
	{ 0xD0C6E757,   6,  2,  2,0x76,  7,  TV_NTSC,   0, 0 }, // Kaguya Hime Densetsu
	{ 0x1D5583FD,   6,  0,  4,   8,  7,  TV_NTSC,   0, 0 }, // Mashou
	{ 0x39AA20F0,   6,  2,  2,0x76,  7,  TV_NTSC,   0, 0 }, // Maison Ikkoku
	{ 0xD294B2E9,   6,  2,  2,0x76,  7,  TV_NTSC,   0, 0 }, // Maison Ikkoku
	{ 0xF2679D5A,   6,  2,  0,   8,  7,  TV_NTSC,   0, 0 }, // Meitantei Holmes
	{ 0x2DFFCA66,   6,  2,  4,   8,  7,  TV_NTSC,   0, 0 }, // Mizushima Shinji no Dai Koushien
	{ 0x3DAE3335,   6,  2,  2,0x76,  7,  TV_NTSC,   0, 0 }, // Nobunaga no Yabou
	{ 0xCDCA4513,   6,  2,  2,0x76,  7,  TV_NTSC,   0, 0 }, // Pro Yakyuu Satsujin Jiken!
	{ 0xC5990806,   6,  2,  2,0x76,  7,  TV_NTSC,   0, 0 }, // Pro Yakyuu Satsujin Jiken!
	{ 0x09A6FF02,   6,  2,  2,0x76,  7,  TV_NTSC,   0, 0 }, // Pro Yakyuu Satsujin Jiken!
	{ 0x7D32ACB9,   6,  2,  0,   8,  7,  TV_NTSC,   0, 0 }, // Reigen Doushi
	{ 0xC4966242,   6,  2,  0,   8,  7,  TV_NTSC,   0, 0 }, // Salad no Kunio no Tomato Hime
	{ 0x0D52E2E6,   6,  2,  2,0x76,  7,  TV_NTSC,   0, 0 }, // Satsui no Kaisou - Power Soft Satsujin Jiken
	{ 0x69587E49,   6,  2,  5,   8,  7,  TV_NTSC,   0, 0 }, // Seirei Gari
	{ 0x818CE8AA,   6,  2,  2,0x76,  7,  TV_NTSC,   0, 0 }, // Shogun (!!!)
	{ 0xEA2B9D3F,   6,  2,  2,0x76,  7,  TV_NTSC,   0, 0 }, // 小公子Ceddie
	{ 0x654D7A3E,   6,  2,  2,0x76,  7,  TV_NTSC,   0, 0 }, // Super Black Onyx
	{ 0x705E13CA,   6,  2,  3,0x76,  7,  TV_NTSC,   0, 0 }, // Super 桃太郎電鉄
	{ 0xFAFF4177,   6,  2,  0,   8,  7,  TV_NTSC,   0, 0 }, // Super Pinball
	{ 0x3D623C6E,   6,  2,  5,   8,  7,  TV_NTSC,   0, 0 }, // Takeshi no Chosenjo
	{ 0xF982CDF5,   6,  2,  2,0x76,  7,  TV_NTSC,   0, 0 }, // Sweet Home
	{ 0x7AEE2161,   6,  2,  0,   8,  7,  TV_NTSC,   0, 0 }, // 太陽の神殿: Aztec 2
	{ 0x21EBE345,   6,  2,  2,0x76,  7,  TV_NTSC,   0, 0 }, // Ultima Exodus
	{ 0xBDE10416,   6,  2,  3,0x76,  7,  TV_NTSC,   0, 0 }, // Zoids 2*/
	// Boards with unusual WRAM
	{ 0x18F5BFDC,   3,  2,  0,   5,  0,  TV_NTSC,   0, 0 }, // Hayauchi Super Igo
	{ 0xE086E68E,   3,  2,  0,   5,  0,  TV_NTSC,   0, 0 }, // Hayauchi Super Igo
	{ 0x860DE1C4,   3,  2,  0,   5,  0,  TV_NTSC,   0, 0 }, // Hayauchi Super Igo
	{ 0xE43075EB,   3,  2,  0,   5,  0,  TV_NTSC,   0, 0 }, // Hayauchi Super Igo
	{ 0x5AF9FD6A,   3,  2,  0,   5,  0,  TV_NTSC,   0, 0 }, // Hayauchi Super Igo
	{ 0x70B5B1F1,   3,  1,  0,   5,  0,  TV_NTSC,   0, 0 }, // Hayauchi Super Igo
	{ 0x54113F0B,   3,  1,  0,   5,  0,  TV_NTSC,   0, 0 }, // Hayauchi Super Igo
	{ 0x41243492,   4,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Low G Man
	{ 0xC0FDDD23,   4,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Low G Man
	{ 0x41AC5A4E,   4,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Low G Man
	{ 0x8238D843,   4,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Low G Man
	{ 0x7C41FAC4,   4,  0,  0,   0,  0,  TV_PAL,    0, 0 }, // Low G Man
	{ 0x98CCD385,   4,  0,  0,   0,  0,  TV_PAL,    0, 0 }, // Low G Man
	// Namco 129/163 with odd WRAM sizes and different mixing resistors
	{ 0x3296FF7A,  19,  2,  2,0x10,  0,  TV_NTSC,   0, 0 }, // Battle Fleet
	{ 0x1294AB5A,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Megami Tensei 2
	{ 0x761CCFB5,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Megami Tensei 2
	{ 0x55611E61,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Megami Tensei 2
	{ 0xB39FCD86,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Megami Tensei 2
	{ 0x96533999,  19,  2,  2,0x10,  0,  TV_NTSC,   0, 0 }, // Dokuganryû Masamune
	{ 0x5F162195,  19,  5,  0,0x01,  0,  TV_NTSC,   0, 0 }, // Erika to Satoru no Yume Bôken
	{ 0x563AA29D,  19,  5,  0,0x01,  0,  TV_NTSC,   0, 0 }, // Erika to Satoru no Yume Bôken
	{ 0x429FD177,  19,  2,  2,0x10,  0,  TV_NTSC,   0, 0 }, // Famista '90
	{ 0x5D1146A6,  19,  2,  2,0x10,  0,  TV_NTSC,   0, 0 }, // Famista '90
	{ 0x968DCF09,  19,  3,  0,0x01,  0,  TV_NTSC,   0, 0 }, // Final Lap
	{ 0xAF895332,  19,  3,  0,0x01,  0,  TV_NTSC,   0, 0 }, // Final Lap
	{ 0x8E287B2F,  19,  3,  0,0x01,  0,  TV_NTSC,   0, 0 }, // Final Lap
	{ 0x29C61B41,  19,  2,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Juvei Quest
	{ 0x36B93D27,  19,  2,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Juvei Quest
	{ 0x6682A59E,  19,  2,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Juvei Quest
	{ 0xDD454208,  19,  2,  2,0x10,  0,  TV_NTSC,   0, 0 }, // Hydlide 3
	{ 0xB1B9E187,  19,  2,  2,0x10,  0,  TV_NTSC,   0, 0 }, // Kaijû Monogatari
	{ 0x5BDBCF6E,  19,  2,  2,0x10,  0,  TV_NTSC,   0, 0 }, // Kaijû Monogatari
	{ 0xCFB40C3C,  19,  2,  2,0x10,  0,  TV_NTSC,   0, 0 }, // Kaijû Monogatari
	{ 0x912A0C3C,  19,  2,  2,0x10,  0,  TV_NTSC,   0, 0 }, // Kaijû Monogatari
	{ 0xB3F3A662,  19,  2,  2,0x10,  0,  TV_NTSC,   0, 0 }, // Kaijû Monogatari
	{ 0x0545CF6E,  19,  2,  2,0x10,  0,  TV_NTSC,   0, 0 }, // Kaijû Monogatari
	{ 0x1DD6619B,  19,  5,  2,0x71,  0,  TV_NTSC,   0, 0 }, // King of Kings
	{ 0x598FB9F5,  19,  4,  0,0x01,  0,  TV_NTSC,   0, 0 }, // Mappy Kids
	{ 0xB7B6800C,  19,  4,  0,0x01,  0,  TV_NTSC,   0, 0 }, // Mappy Kids
	{ 0xAF15338F,  19,  2,  2,0x10,  0,  TV_NTSC,   0, 0 }, // Mindseeker
	{ 0x2B825CE1,  19,  0,  0,0x01,  0,  TV_NTSC,   0, 0 }, // Namco Classic
	{ 0x9A2B0641,  19,  4,  0,0x01,  0,  TV_NTSC,   0, 0 }, // Namco Classic II
	{ 0x3DEAC303,  19,  4,  0,0x01,  0,  TV_NTSC,   0, 0 }, // Rolling Thunder
	{ 0xB78970D2,  19,  4,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi
	{ 0xBC387346,  19,  4,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi
	{ 0x6901346E,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0xD8F5291A,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0x5C2EBF72,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0xDB443200,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0x0467B363,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0xE57B3740,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0xA9F1D0EE,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0x24994C97,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0xAEBB4B14,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0x8526C9E7,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0x60254C19,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0x42200894,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0x3AD76222,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0x2D50308C,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0x43E53A76,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0xE6BD913E,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0x1CDCF32A,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0x90A2C036,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0x73D58441,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0xC7FE9B9C,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0x08E268B4,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0x46CE25D0,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0x759DF76B,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0xEE1A0659,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0x7457D11C,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0x150142CA,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0x6FFCC397,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0x0CC4BB06,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0x9EDA5C31,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0xC52C28F3,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0x4B4B916D,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0x93B37783,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0xAEF73BA0,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0x4004C548,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0x805E775A,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0xCD96D83C,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0x6901346E,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0xFBC297F5,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0x71F40FAC,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0xC5809320,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0xC6851284,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0x1F797509,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0x4DFE1A45,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0x3E881EDF,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0x85716E3E,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0x41817932,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0x1EAD7B0B,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0x430B4A4F,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0xEDAC76A2,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0x936C3380,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0x0D4E1E34,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0x52548B56,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0x1FCABE9B,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0x1D4A0D11,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0x4A5128E7,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0x0D82383E,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0x390C61DE,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0x2B0FE9AD,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0xCC0EF76D,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0xEE5F392E,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0xE0302224,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0x6E98C9B6,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0xF525D152,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0xE8698AE9,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0xD0A70A05,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0x54444BF7,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0x2244D9E5,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0x8D7913B3,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0x97496ED7,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0x48619EB2,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0x880D3888,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0xCFD4D6FE,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0xBC1A0DFF,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0x6F132BC8,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0xAF57D8A9,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0x264A66B0,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0xCE22AFCA,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0xA31B51FC,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0xCE22B6B8,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0x5252E7CD,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0x65066482,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0x6C63A17D,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0x60B8E823,  19,  3,  2,0x71,  0,  TV_NTSC,   0, 0 }, // Sangokushi II
	{ 0x8A2824BB,  19,  2,  0,0x01,  0,  TV_NTSC,   0, 0 }, // Star Wars
	{ 0xAE82C753,  19,  2,  0,0x01,  0,  TV_NTSC,   0, 0 }, // Star Wars
	{ 0xDEEA2A19,  19,  2,  0,0x01,  0,  TV_NTSC,   0, 0 }, // Star Wars
	{ 0x088A9D58,  19,  2,  0,0x01,  0,  TV_NTSC,   0, 0 }, // Star Wars
	{ 0x7EAFE3A8,  19,  2,  0,0x01,  0,  TV_NTSC,   0, 0 }, // Star Wars
	{ 0x068B049C,  19,  2,  0,0x01,  0,  TV_NTSC,   0, 0 }, // Star Wars
	{ 0x4E979D8B,  19,  2,  0,0x01,  0,  TV_NTSC,   0, 0 }, // Star Wars
	{ 0x2DD784A8,  19,  2,  0,0x01,  0,  TV_NTSC,   0, 0 }, // Star Wars
	{ 0x17421900,  19,  4,  0,0x01,  0,  TV_NTSC,   0, 0 }, // Yôkai Dôchûki
	{ 0xE6CF29F9,  19,  4,  0,0x01,  0,  TV_NTSC,   0, 0 }, // Yôkai Dôchûki
	// MMC1A without WRAM disable bit
	{ 0x15D53E78, 155,  0,  0,   7,  0,  TV_NTSC,   0, 0 }, // Tatakae!! Ramen Man
	{ 0x81DF3D70, 155,  0,  0,   7,  0,  TV_NTSC,   0, 0 }, // Tatakae!! Ramen Man
	{ 0x8CE9C87B, 155,  0,  2,0x70,  0,  TV_NTSC,   0, 0 }, // The Money Game
	// MMC1 with non-bankable 32 KiB of PRG-ROM
	{ 0x75CC0549,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Adventures of Lolo
	{ 0xE5682E52,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Adventures of Lolo
	{ 0x7F7156A7,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Adventures of Lolo
	{ 0x6030218C,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Adventures of Lolo
	{ 0x6C9C02EB,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Adventures of Lolo
	{ 0x06967ABB,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Adventures of Lolo
	{ 0x1CE81916,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Adventures of Lolo
	{ 0x4210F75E,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Adventures of Lolo
	{ 0x0724FD1A,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Adventures of Lolo
	{ 0x5DCA93EF,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Adventures of Lolo
	{ 0xFC0E71D3,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Adventures of Lolo
	{ 0xEA6A1596,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Adventures of Lolo
	{ 0x3F839C22,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Adventures of Lolo
	{ 0xA59AE4D7,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Adventures of Lolo
	{ 0x08C79373,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Adventures of Lolo
	{ 0x3A4C4179,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Adventures of Lolo
	{ 0x6051578E,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Adventures of Lolo
	{ 0x945BF8A4,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Adventures of Lolo
	{ 0xC0025BF3,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Adventures of Lolo
	{ 0xE25EE789,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Adventures of Lolo
	{ 0xD03D21D6,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Adventures of Lolo
	{ 0x79E62E23,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Adventures of Lolo
	{ 0x54BA9958,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Adventures of Lolo
	{ 0x722F3DB3,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Adventures of Lolo
	{ 0xB43FCD05,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Adventures of Lolo
	{ 0x1ED3151F,   1,  5,  0,   0,  0,   TV_PAL,   0, 0 }, // Adventures of Lolo
	{ 0x8E773E04,   1,  5,  0,   0,  0,   TV_PAL,   0, 0 }, // Adventures of Lolo
	{ 0x79D8C39D,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Anticipation
	{ 0x2FC528E0,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Anticipation
	{ 0xB391A86D,   1,  5,  0,   0,  0,   TV_PAL,   0, 0 }, // Anticipation
	{ 0xC6475C2A,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Boulder Dash
	{ 0x62DFC064,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Boulder Dash
	{ 0x3B86CCB7,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Boulder Dash
	{ 0x66D6176F,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Boulder Dash
	{ 0x414220B1,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Boulder Dash
	{ 0x73EBA224,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Boulder Dash
	{ 0xE69353F7,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Boulder Dash
	{ 0x123DB3BF,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Boulder Dash
	{ 0x945AA127,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Boulder Dash
	{ 0x54A0C2F0,   1,  5,  0,   0,  0,   TV_PAL,   0, 0 }, // Boulder Dash
	{ 0x06CE1BAF,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Boulder Dash
	{ 0xCB02A930,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Dr. Mario
	{ 0xCD2AC92C,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Dr. Mario
	{ 0x36EF4C7E,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Dr. Mario
	{ 0xF6AD1545,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Dr. Mario
	{ 0x1C614ADB,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Dr. Mario
	{ 0x346E5418,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Dr. Mario
	{ 0xF6C951A2,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Dr. Mario
	{ 0x70046938,   1,  5,  0,   0,  0,   TV_PAL,   0, 0 }, // Dr. Mario
	{ 0xDD0B2E7B,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Dr. Mario
	{ 0x4D72A3E4,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Dr. Mario
	{ 0x503B5AE7,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Dr. Mario
	{ 0x99205BD0,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Dr. Mario
	{ 0xC1BB2A5F,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Dr. Mario
	{ 0xA5239A77,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Dr. Mario
	{ 0xB31C6ADD,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Dr. Mario
	{ 0xCAC362CA,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Dr. Mario
	{ 0x33A2753C,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Dr. Mario
	{ 0x6A80DE01,   1,  5,  0,   0,  0,   TV_PAL,   0, 0 }, // Dr. Mario
	{ 0x65609DF1,   1,  5,  0,   0,  0,   TV_PAL,   0, 0 }, // Dr. Mario
	{ 0xE3F2E1D6,   1,  5,  0,   0,  0,   TV_PAL,   0, 0 }, // Dr. Mario
	{ 0x772513F4,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Hirake! Ponkikki
	{ 0xA99BDB1B,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Hirake! Ponkikki
	{ 0xD1E331CB,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Hirake! Ponkikki
	{ 0x7EBFDFE4,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Hirake! Ponkikki
	{ 0x35E3EEA2,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Palamedes
	{ 0x17C111E0,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Palamedes
	{ 0x1E9EBB00,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Palamedes II
	{ 0x82CFDE25,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // R.C. Pro-Am (rev 1)
	{ 0x42AE70CB,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // R.C. Pro-Am (rev 1)
	{ 0x2DBDDD11,   1,  5,  0,   0,  0,   TV_PAL,   0, 0 }, // R.C. Pro-Am (rev 1)
	{ 0x0847C623,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Sesame Street 123
	{ 0x2E6C3CA9,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Sesame Street ABC
	{ 0xAC38CF63,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Sesame Street ABC
	{ 0xBE6E3385,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Sesame Street ABC
	{ 0x798B491D,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Sesame Street ABC
	{ 0x8E9C4F74,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Snake Rattle'n Roll
	{ 0xC1E699AC,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Snake Rattle'n Roll
	{ 0xA6496E01,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Snake Rattle'n Roll
	{ 0x406FE900,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Snake Rattle'n Roll
	{ 0xC076E61A,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Snake Rattle'n Roll
	{ 0x8BF554F3,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Snake Rattle'n Roll
	{ 0xCF7C5FCD,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Snake Rattle'n Roll
	{ 0x533F5707,   1,  5,  0,   0,  0,   TV_PAL,   0, 0 }, // Snake Rattle'n Roll
	{ 0xAA174BC6,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Airwolf
	{ 0xA8357A62,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Airwolf
	{ 0x19ED0810,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Airwolf
	{ 0xF71A9931,   1,  5,  0,   0,  0,   TV_PAL,   0, 0 }, // Airwolf
	{ 0x26F2B268,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Family Feud
	{ 0xAD7F9480,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Sky Kid
	{ 0xC1918EC3,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Sky Kid
	{ 0xD3DEA841,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Tetris
	{ 0x943DFBBE,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Tetris
	{ 0x7C6F9AC8,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Tetris
	{ 0x9609528E,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Tetris
	{ 0xA6F9A456,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Tetris
	{ 0xBB565E44,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Tetris
	{ 0xA95E7903,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Tetris
	{ 0xD7B0B0D1,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Tetris
	{ 0x62696B41,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Tetris
	{ 0x0B3AF855,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Tetris
	{ 0xA95E7903,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Tetris
	{ 0x05FE773B,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Taboo (PRG0)
	{ 0x4366545C,   1,  5,  0,   0,  0,  TV_NTSC,   0, 0 }, // Taboo (PRG1)
	// Vs. Unisystem
	{ 0xD5D7EAC4,   1,  0, 24,   5,  0,  TV_NTSC,   4, 0 }, // Dr. Mario
	{ 0x07138C06,  99,  0, 24,   5,  0,  TV_NTSC,   5, 0 }, // Clu Clu Land
	{ 0xED588F00,  99,  0, 24,   5,  0,  TV_NTSC,   0, 0 }, // Duck Hunt
	{ 0x29155E0C,  99,  0, 24,   5,  0,  TV_NTSC,   5, 0 }, // ExciteBike (rev 0)
	{ 0xCBE85490,  99,  0, 24,   5,  0,  TV_NTSC,   4, 0 }, // ExciteBike (rev 1)
	{ 0xFE446787,  99,  0, 24,   5,  0,  TV_NTSC,  10, 0 }, // Gumshoe
	{ 0xE2A5A4CC,  99,  0, 24,   5,  0,  TV_NTSC,  10, 0 }, // Gumshoe
	{ 0xBBCB8236,  99,  0, 24,   5,  0,  TV_NTSC,  10, 0 }, // Gumshoe
	{ 0xFF5135A3,  99,  0, 24,   5,  0,  TV_NTSC,   2, 0 }, // Hogan's Alley
	{ 0x43A357EF,  99,  0, 24,   5,  0,  TV_NTSC,   5, 0 }, // Ice Climber
	{ 0x0B65A917,  99,  0, 24,   5,  0,  TV_NTSC,   3, 0 }, // Mach Rider (Endurance Course version)
	{ 0x8A6A9848,  99,  0, 24,   5,  0,  TV_NTSC,   3, 0 }, // Mach Rider (Endurance Course version)
	{ 0xE528F651,  99,  0, 24,   5,  0,  TV_NTSC,   0, 0 }, // Pinball (rev 0)
	{ 0xEC461DB9,  99,  0, 24,   5,  0,  TV_NTSC,   2, 0 }, // Pinball (rev 1)
	{ 0x70901B25,   0,  0, 24,   5,  0,  TV_NTSC,   3, 0 }, // Slalom
	{ 0x46914E3E,  99,  0, 24,   5,  0,  TV_NTSC,   4, 0 }, // Soccer (rev 1)
	{ 0xA93A5AEE,  99,  0, 24,   5,  0,  TV_NTSC,   0, 0 }, // Stroke & Match Golf (Men's, rev 0?)
	{ 0xCC2C4B5D,  99,  0, 24,   5,  0,  TV_NTSC,   3, 0 }, // Stroke & Match Golf (Men's, rev 1?)
	{ 0x86167220,  99,  0, 24,   5,  0,  TV_NTSC,   3, 0 }, // Stroke & Match Golf (Ladies')
	{ 0x8192C804,  99,  0, 24,   5,  0,  TV_NTSC,   5, 0 }, // Super Mario Brothers FAIL
	{ 0x8B60CC58,  99,  0, 24,   5,  0,  TV_NTSC,   5, 0 }, // Super Mario Brothers
	{ 0x3296B9EF,  99,  0, 24,   5,  0,  TV_NTSC,   0, 0 }, // Super Mario Brothers
	{ 0x737DD1BF,  99,  0, 24,   5,  0,  TV_NTSC,   5, 0 }, // Super Mario Brothers
	{ 0x0D184485,  99,  0, 24,   5,  0,  TV_NTSC,   5, 0 }, // Super Mario Brothers
	{ 0x135ADF7C, 206,  0, 24,   5,  0,  TV_NTSC,0x12, 0 }, // Atari R.B.I. Baseball
	{ 0xEB2DBA63, 206,  0, 24,   5,  0,  TV_NTSC,0x24, 0 }, // T.K.O. Boxing
	{ 0x98CFE016, 206,  0, 24,   5,  0,  TV_NTSC,0x24, 0 }, // T.K.O. Boxing
	{ 0x9818F656, 206,  0, 24,   5,  0,  TV_NTSC,0x24, 0 }, // T.K.O. Boxing
	{ 0x70433F2C,  99,  0, 24,   5,  0,  TV_NTSC,   2, 0 }, // Battle City
	{ 0x8D15A6E6,  99,  0, 24,   5,  0,  TV_NTSC,   2, 0 }, // Battle City
	{ 0xCF36261E, 206,  0, 24,   5,  0,  TV_NTSC,   2, 0 }, // Super Sky Kid
	{ 0xE1AA8214,  99,  0, 24,   5,  0,  TV_NTSC,   0, 0 }, // Star Luster
	{ 0xF9D3B0A3, 206,  0, 24,   5,  0,  TV_NTSC,0x32, 0 }, // Super Xevious: GAMP no Nazo
	{ 0x16D3F469,  99,  0, 24,   5,  0,  TV_NTSC,   8, 0 }, // Ninja Jajamaru-kun
	{ 0xCA85E56D,  99,  0, 24,   5,  0,  TV_NTSC,   9, 0 }, // Mighty Bomb Jack
	{ 0xFFBEF374,   2,  0, 24,   5,  7,  TV_NTSC,   3, 0 }, // Castlevania
	{ 0x1E438D52,  75,  0, 24,   5,  0,  TV_NTSC,   4, 0 }, // The Goonies
	{ 0xBF88FD7B,  75,  0, 24,   5,  0,  TV_NTSC,   4, 0 }, // The Goonies
	{ 0xD99A2087,  75,  0, 24,   5,  0,  TV_NTSC,   2, 0 }, // Gradius
	{ 0xF0632142,  75,  0, 24,   5,  0,  TV_NTSC,   2, 0 }, // Gradius
	{ 0x8C0C2DF5,   2,  0, 24,   5,  7,  TV_NTSC,  11, 0 }, // Top Gun
	{ 0x17AE56BE, 206,  0, 24,   5,  0,  TV_NTSC,   2, 0 }, // Freedom Force
	{ 0xE2C0A2BE,  67,  0, 24,   5,  0,  TV_NTSC,   2, 0 }, // Platoon
	{ 0x8850924B,  99,  0, 24,   5,  0,  TV_NTSC,   2, 0 }, // Tetris
	// Vs. Dual
	{ 0xAD407F52,  99,  0, 24,   5,  0,  TV_NTSC,   4, 0 }, // Balloon Fight
	{ 0x6AD67502,  99,  0, 24,   5,  0,  TV_NTSC,   4, 0 }, // Balloon Fight
	{ 0xF42DAB14,  99,  0, 24,   5,  0,  TV_NTSC,   5, 0 }, // Ice Climber P1
	{ 0x7D6B764F,  99,  0, 24,   5,  0,  TV_NTSC,   5, 0 }, // Ice Climber P2
	{ 0x18A93B7B,  99,  0, 24,   5,  0,  TV_NTSC,   0, 0 }, // Mahjong ????
	{ 0x008A9C16,  99,  0, 24,   5,  0,  TV_NTSC,   3, 0 }, // Wrecking Crew
	{ 0x30C42B1E,  99,  0, 24,   5,  0,  TV_NTSC,   3, 0 }, // Wrecking Crew
	{ 0xB90497AA,  99,  0, 24,   5,  0,  TV_NTSC,   0, 0 }, // Tennis
	{ 0xBC202DB6,  99,  0, 24,   5,  0,  TV_NTSC,   0, 0 }, // Tennis
	{ 0xF5DEBF88,  99,  0, 24,   5,  0,  TV_NTSC,   2, 0 }, // Baseball
	{ 0xB5853830,  99,  0, 24,   5,  0,  TV_NTSC,   2, 0 }, // Baseball
	{ 0xC4DD2523,  99,  0, 24,   5,  0,  TV_NTSC,   2, 0 }, // Baseball
	{ 0x13A91937,  99,  0, 24,   5,  0,  TV_NTSC,   2, 0 }, // Baseball
	{ 0xF64D7252,  99,  0, 24,   5,  0,  TV_NTSC,   2, 0 }, // Baseball
	{ 0x968A6E9D,  99,  0, 24,   5,  0,  TV_NTSC,   2, 0 }, // Baseball
	// MMC5, to specify PRG-RAM size
	{ 0x286613D8,   5,  0,  2,0x90,  0,  TV_NTSC,   0, 0 }, // Nobunaga no Yabô: Bushô Fûunroku
	{ 0xF4120E58,   5,  0,  2,0x90,  0,  TV_NTSC,   0, 0 }, // Aoki Ôkami to Shiroki Mejika: Genchô Hishi
	{ 0x11EAAD26,   5,  0,  2,0x90,  0,  TV_NTSC,   0, 0 }, // Romance of the Three Kingdoms II
	{ 0xFCFD72DC,   5,  0,  2,0x90,  0,  TV_NTSC,   0, 0 }, // Romance of the Three Kingdoms II
	{ 0x95BA5733,   5,  0,  2,0x90,  0,  TV_NTSC,   0, 0 }, // Romance of the Three Kingdoms II
	{ 0x135D21F9,   5,  0,  2,0x90,  0,  TV_NTSC,   0, 0 }, // Romance of the Three Kingdoms II

	{ 0x98C8E090,   5,  0,  2,0x77,  0,  TV_NTSC,   0, 0 }, // Nobunaga's Ambition II
	{ 0xE59071C4,   5,  0,  2,0x77,  0,  TV_NTSC,   0, 0 }, // Nobunaga's Ambition II
	{ 0xB56958D1,   5,  0,  2,0x77,  0,  TV_NTSC,   0, 0 }, // Nobunaga's Ambition II
	{ 0x7A6033F9,   5,  0,  2,0x77,  0,  TV_NTSC,   0, 0 }, // Nobunaga's Ambition II
	{ 0x2B548D75,   5,  0,  2,0x77,  0,  TV_NTSC,   0, 0 }, // Bandit Kings of Ancient China
	{ 0xF4024666,   5,  0,  2,0x77,  0,  TV_NTSC,   0, 0 }, // Bandit Kings of Ancient China
	{ 0xE6C28C5F,   5,  0,  2,0x77,  0,  TV_NTSC,   0, 0 }, // Bandit Kings of Ancient China
	{ 0x40DBC815,   5,  0,  0,0x00,  0,  TV_NTSC,   0, 0 }, // Castlevania III
	{ 0x50238DAA,   5,  0,  0,0x00,  0,  TV_NTSC,   0, 0 }, // Castlevania III
	{ 0x95CA9EC7,   5,  0,  0,0x00,  0,  TV_NTSC,   0, 0 }, // Castlevania III
	{ 0x184112C0,   5,  0,  0,0x00,  0,  TV_NTSC,   0, 0 }, // Castlevania III
	{ 0xF99E60C2,   5,  0,  0,0x00,  0,  TV_NTSC,   0, 0 }, // Castlevania III
	{ 0xC2AAE5AC,   5,  0,  0,0x00,  0,  TV_NTSC,   0, 0 }, // Castlevania III
	{ 0x5CD8E567,   5,  0,  0,0x00,  0,  TV_NTSC,   0, 0 }, // Castlevania III
	{ 0xA429E83F,   5,  0,  0,0x00,  0,  TV_NTSC,   0, 0 }, // Castlevania III
	{ 0x8CE26532,   5,  0,  0,0x00,  0,  TV_NTSC,   0, 0 }, // Castlevania III
	{ 0x70681F38,   5,  0,  0,0x00,  0,  TV_NTSC,   0, 0 }, // Castlevania III
	{ 0x677F8617,   5,  0,  0,0x00,  0,  TV_NTSC,   0, 0 }, // Castlevania III
	{ 0x000DD26F,   5,  0,  0,0x00,  0,  TV_NTSC,   0, 0 }, // Castlevania III
	{ 0x889D5342,   5,  0,  0,0x00,  0,  TV_NTSC,   0, 0 }, // Castlevania III
	{ 0x7BAF3679,   5,  0,  0,0x00,  0,  TV_NTSC,   0, 0 }, // Castlevania III
	{ 0x4933E754,   5,  0,  0,0x00,  0,   TV_PAL,   0, 0 }, // Castlevania III
	{ 0x637E366A,   5,  0,  0,0x00,  0,   TV_PAL,   0, 0 }, // Castlevania III
	{ 0x5E3FF45C,   5,  0,  0,0x00,  0,   TV_PAL,   0, 0 }, // Castlevania III
	{ 0xBC604735,   5,  0,  0,0x00,  0,   TV_PAL,   0, 0 }, // Castlevania III
	{ 0xF4CD4998,   5,  0,  2,0x77,  0,  TV_NTSC,   0, 0 }, // Uncharted Waters
	{ 0x30310DF1,   5,  0,  2,0x77,  0,  TV_NTSC,   0, 0 }, // Uncharted Waters
	{ 0xCD35E2E9,   5,  0,  2,0x77,  0,  TV_NTSC,   0, 0 }, // Uncharted Waters
	{ 0x8F9BE3C8,   5,  0,  2,0x77,  0,  TV_NTSC,   0, 0 }, // Uncharted Waters
	{ 0xE7C72DBB,   5,  0,  2,0x70,  0,  TV_NTSC,   0, 0 }, // Gemfire
	{ 0x57F33F70,   5,  0,  2,0x70,  0,  TV_NTSC,   0, 0 }, // Gemfire
	{ 0x255B129C,   5,  0,  0,0x00,  0,  TV_NTSC,   0, 0 }, // Laser Invasion
	{ 0xBE87A305,   5,  0,  0,0x00,  0,  TV_NTSC,   0, 0 }, // Laser Invasion
	{ 0x51D2112F,   5,  0,  0,0x00,  0,  TV_NTSC,   0, 0 }, // Laser Invasion
	{ 0x8FA95456,   5,  0,  2,0x77,  0,  TV_NTSC,   0, 0 }, // Ishin no Arashi
	{ 0x5D9D9891,   5,  0,  2,0x70,  0,  TV_NTSC,   0, 0 }, // Just Breed
	{ 0x339C034C,   5,  0,  2,0x70,  0,  TV_NTSC,   0, 0 }, // Just Breed
	{ 0x664A480E,   5,  0,  2,0x70,  0,  TV_NTSC,   0, 0 }, // Just Breed
	{ 0x43EC2B09,   5,  0,  2,0x70,  0,  TV_NTSC,   0, 0 }, // Just Breed
	{ 0x562EF3E4,   5,  0,  2,0x70,  0,  TV_NTSC,   0, 0 }, // Just Breed
	{ 0xE1767673,   5,  0,  2,0x70,  0,  TV_NTSC,   0, 0 }, // Just Breed
	{ 0x61F3512A,   5,  0,  2,0x70,  0,  TV_NTSC,   0, 0 }, // Just Breed
	{ 0xC8DD5A23,   5,  0,  2,0x70,  0,  TV_NTSC,   0, 0 }, // Just Breed
	{ 0xEAC51D06,   5,  0,  2,0x70,  0,  TV_NTSC,   0, 0 }, // Just Breed
	{ 0x57E3218B,   5,  0,  2,0x77,  0,  TV_NTSC,   0, 0 }, // L'Empereur
	{ 0x2F50BD38,   5,  0,  2,0x77,  0,  TV_NTSC,   0, 0 }, // L'Empereur
	{ 0x8E9A5E2F,   5,  0,  2,0x77,  0,  TV_NTSC,   0, 0 }, // L'Empereur
	{ 0xFF7C1A14,   5,  0,  0,0x00,  0,  TV_NTSC,   0, 0 }, // Metal Slader Glory
	{ 0xCD9ACF43,   5,  0,  0,0x00,  0,  TV_NTSC,   0, 0 }, // Metal Slader Glory
	{ 0xE91548D8,   5,  0,  2,0x70,  0,  TV_NTSC,   0, 0 }, // Yakuman Tengoku
	{ 0xD979C8B7,   5,  0,  0,0x00,  0,  TV_NTSC,   0, 0 }, // Uchû Keibitai SDF
	{ 0x1D612007,   5,  0,  0,0x00,  0,  TV_NTSC,   0, 0 }, // Uchû Keibitai SDF
	// MMC1 with more than 8 KiB PRG-RAM
	{ 0xFB69743A,   1,  0,  2,0x77,  7,  TV_NTSC,   0, 0 }, // Genghis Khan
	{ 0x067BD24C,   1,  0,  2,0x77,  7,  TV_NTSC,   0, 0 }, // Genghis Khan
	{ 0x2225C20F,   1,  0,  2,0x77,  7,  TV_NTSC,   0, 0 }, // Genghis Khan
	{ 0x0BF7EEDB,   1,  0,  2,0x77,  7,  TV_NTSC,   0, 0 }, // Genghis Khan
	{ 0x3F7AD415,   1,  0,  2,0x77,  7,  TV_NTSC,   0, 0 }, // Nobunaga no Yabô
	{ 0x2B11E0B0,   1,  0,  2,0x77,  7,  TV_NTSC,   0, 0 }, // Nobunaga no Yabô
	{ 0x29449BA9,   1,  0,  2,0x77,  7,  TV_NTSC,   0, 0 }, // Nobunaga no Yabô
	{ 0x4642DDA6,   1,  0,  2,0x77,  7,  TV_NTSC,   0, 0 }, // Nobunaga no Yabô
	{ 0x3B92853D,   1,  0,  2,0x77,  7,  TV_NTSC,   0, 0 }, // Nobunaga no Yabô
	{ 0xA564AB26,   1,  0,  2,0x77,  7,  TV_NTSC,   0, 0 }, // Nobunaga no Yabô
	{ 0xC6182024,   1,  0,  2,0x77,  7,  TV_NTSC,   0, 0 }, // Sangokushi
	{ 0x7F93FB41,   1,  0,  2,0x77,  7,  TV_NTSC,   0, 0 }, // Sangokushi
	{ 0x90388D1B,   1,  0,  2,0x77,  7,  TV_NTSC,   0, 0 }, // Sangokushi
	{ 0xC6182024,   1,  0,  2,0x77,  7,  TV_NTSC,   0, 0 }, // Sangokushi
	{ 0xABBF7217,   1,  0,  2,0x77,  7,  TV_NTSC,   0, 0 }, // Sangokushi
	{ 0x1028FC27,   1,  0,  2,0x77,  7,  TV_NTSC,   0, 0 }, // Sangokushi
	{ 0xCCF35C02,   1,  0,  2,0x77,  7,  TV_NTSC,   0, 0 }, // Sangokushi
	{ 0xC9556B36,   1,  0,  2,0x90,  7,  TV_NTSC,   0, 0 }, // Final Fantasy I+II
	{ 0x7D5CA373,   1,  0,  2,0x90,  7,  TV_NTSC,   0, 0 }, // Final Fantasy I+II
	{ 0xB1FCBBCA,   1,  0,  2,0x90,  7,  TV_NTSC,   0, 0 }, // Final Fantasy I+II
	{ 0x7531D99D,   1,  0,  2,0x90,  7,  TV_NTSC,   0, 0 }, // Final Fantasy I+II
	{ 0x8A9EA60F,   1,  0,  2,0x90,  7,  TV_NTSC,   0, 0 }, // Final Fantasy I+II
	{ 0x7531D99D,   1,  0,  2,0x90,  7,  TV_NTSC,   0, 0 }, // Final Fantasy I+II
	{ 0xB8747ABF,   1,  0,  2,0x90,  7,  TV_NTSC,   0, 0 }, // The Best Play Pro Yakyû Special
	{ 0xC3DE7C69,   1,  0,  2,0x90,  7,  TV_NTSC,   0, 0 }, // The Best Play Pro Yakyû Special
	{ 0xF7D51D87,   1,  0,  2,0x90,  7,  TV_NTSC,   0, 0 }, // The Best Play Pro Yakyû Special
	{ 0xE3C2C174,   1,  0,  2,0x90,  7,  TV_NTSC,   0, 0 }, // The Best Play Pro Yakyû Special
	// MMC6
	{ 0xCBCDCDDF,   4,  1,  2,0x40,  0,  TV_NTSC,   0, 0 }, // StarTropics
	{ 0xBEB88304,   4,  1,  2,0x40,  0,  TV_NTSC,   0, 0 }, // StarTropics
	{ 0x37AE5349,   4,  1,  2,0x40,  0,  TV_NTSC,   0, 0 }, // StarTropics
	{ 0xF3E2B7D7,   4,  1,  2,0x40,  0,  TV_NTSC,   0, 0 }, // StarTropics
	{ 0xD9019387,   4,  1,  2,0x40,  0,   TV_PAL,   0, 0 }, // StarTropics
	{ 0xAC74DD5C,   4,  1,  2,0x40,  0,   TV_PAL,   0, 0 }, // StarTropics
	{ 0x8514EDBC,   4,  1,  2,0x40,  0,   TV_PAL,   0, 0 }, // StarTropics
	{ 0x4F847E00,   4,  1,  2,0x40,  0,   TV_PAL,   0, 0 }, // StarTropics
	{ 0x80FB117E,   4,  1,  2,0x40,  0,  TV_NTSC,   0, 0 }, // StarTropics II
	{ 0xB78E313A,   4,  1,  2,0x40,  0,  TV_NTSC,   0, 0 }, // StarTropics II
	{ 0xB5842CD9,   4,  1,  2,0x40,  0,   TV_PAL,   0, 0 }, // StarTropics II
	{ 0x82F10C9D,   4,  1,  2,0x40,  0,   TV_PAL,   0, 0 }, // StarTropics II
	// Other MMC3 Submappers
	{ 0xA67EA466,   4,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Alien 3
	{ 0x52C5F365,   4,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Alien 3
	{ 0x300E2835,   4,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Alien 3
	{ 0x13A1CAB4,   4,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Alien 3
	{ 0x1F7804A3,   4,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Alien 3
	{ 0x9F4F7318,   4,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Alien 3
	{ 0xD3215AEC,   4,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Alien 3
	{ 0x58D0279D,   4,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Alien 3
	{ 0xA86AF976,   4,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // George Foreman's K.O. Boxing
	{ 0x86277361,   4,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // The Incredible Crash Dummies
	{ 0x647EAFA9,   4,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // The Incredible Crash Dummies
	{ 0xA58DB4B4,   4,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // The Incredible Crash Dummies
	{ 0x3CAB6A08,   4,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // The Incredible Crash Dummies
	{ 0x91AA57F1,   4,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Mickey's Safari in Letterland
	{ 0x804A0570,   4,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Roger Clemens' MVP Baseball
	{ 0x20574509,   4,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Rollerblade Racer
	{ 0xCB392A83,   4,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // The Simpsons: Bart versus the World
	{ 0xF470D4E3,   4,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // The Simpsons: Bart versus the World
	{ 0xD72FB9C8,   4,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // The Simpsons: Bartman Meets Radioactive Man
	{ 0x79CEB751,   4,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // The Simpsons: Bartman Meets Radioactive Man
	{ 0xC18BCF6E,   4,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // The Simpsons: Bartman Meets Radioactive Man
	{ 0x00BEC4D7,   4,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Spider-Man: Return of the Sinister Six
	{ 0xC23E217A,   4,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Spider-Man: Return of the Sinister Six
	{ 0xBABB537D,   4,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Spider-Man: Return of the Sinister Six
	{ 0xD36E29FD,   4,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Spider-Man: Return of the Sinister Six
	{ 0xA3853D63,   4,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Spider-Man: Return of the Sinister Six
	{ 0x6AF98DA3,   4,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Spider-Man: Return of the Sinister Six
	{ 0x233E4DF1,   4,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Spider-Man: Return of the Sinister Six
	{ 0xCA026F1F,   4,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Town and Country Surf Designs 2
	{ 0xD5CF13E5,   4,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Town and Country Surf Designs 2
	{ 0x8E5A7827,   4,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Town and Country Surf Designs 2
	{ 0xD5753AB3,   4,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Town and Country Surf Designs 2
	{ 0x4B647C55,   4,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // T2 - Terminator 2: Judgement Day
	{ 0xCC62D031,   4,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // T2 - Terminator 2: Judgement Day
	{ 0x793E5F0B,   4,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // T2 - Terminator 2: Judgement Day
	{ 0x91D980CC,   4,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // T2 - Terminator 2: Judgement Day
	{ 0xA50C33F4,   4,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // T2 - Terminator 2: Judgement Day
	{ 0xA1A8996C,   4,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // T2 - Terminator 2: Judgement Day
	{ 0x793E5F0B,   4,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // T2 - Terminator 2: Judgement Day
	{ 0xED02F8DC,   4,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // T2 - Terminator 2: Judgement Day
	{ 0x1953A406,   4,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // T2 - Terminator 2: Judgement Day
	{ 0xCB25A2CA,   4,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // WWF King of the Ring
	{ 0x25A44D1B,   4,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // WWF Wrestlemania Steel Cage Challenge
	{ 0x849ED99E,   4,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // WWF Wrestlemania Steel Cage Challenge
	{ 0xC0597CE4,   4,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // WWF Wrestlemania Steel Cage Challenge
	{ 0xF071075B,   4,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // WWF Wrestlemania Steel Cage Challenge
	// WXN
	{ 0xC7BD734E, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // 2D Escape
	{ 0x28E8D2D0, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Access Block
	{ 0x49FA4DD4, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Add'em Up
	{ 0x00DB0DE9, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Aether Fighter
	{ 0x077EA86C, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Alienis
	{ 0x220B8436, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Level 1
	{ 0x159F2449, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Archer
	{ 0x36457E23, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Archery
	{ 0x0DA24590, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Assart
	{ 0xC8868961, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Attacking
	{ 0xC81D724F, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Autoboat Contest
	{ 0xE3BECAB9, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Backstroke
	{ 0xAB59149F, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Baseball
	{ 0x80440A3A, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Battle of the Desert
	{ 0x840F6A51, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Bomb Time
	{ 0x6D862805, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Boxes World
	{ 0x165C324C, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Boxing Wrestle
	{ 0x3247D92F, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Brave Boy
	{ 0x41BACA43, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Car Park
	{ 0xE9C92F63, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Cat versus Dog
	{ 0x041AC3DD, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Clever Way
	{ 0x02B700E2, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Connect Pipe
	{ 0xD8713449, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Super Fight I
	{ 0x2D83A7A4, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Convection
	{ 0x6262BA97, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Corridor
	{ 0x88105311, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // 100 Meters Freestyle
	{ 0x3FB2B6F9, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Crazy Racing
	{ 0x77CCB00B, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Cute Fish
	{ 0xED75511B, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Danger Zone
	{ 0x65053D0B, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Dark Castle
	{ 0x629ADC26, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Defensive
	{ 0xCB846B1D, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Desert Storm
	{ 0x027A90BF, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Diamond
	{ 0xC3F2A336, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Discus
	{ 0xA70DC77D, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Dodgeball
	{ 0x50EF26E7, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Dragon Den
	{ 0x10E88223, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Edacity Snakes
	{ 0x2097A6AF, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // E.T. The Extraterrestial
	{ 0x4F435D2D, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Explorer
	{ 0xF77A8BF5, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Final Fighter
	{ 0x660B4C1C, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Fireman
	{ 0xEAEE9FC8, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Fishing Super
	{ 0x58FA174F, 256,  0,  0,   0,  0,  TV_DUAL,   0, 0 }, // Five Kids
	{ 0x212D201B, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Football
	{ 0xB9EBEADD, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Galaxy Killer
	{ 0xD919449E, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Galaxy War
	{ 0x3D0E92AE, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Gem
	{ 0xE71D67C0, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Germ Killer
	{ 0x945344A1, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Get Lost
	{ 0xA6977003, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Ghost Ship
	{ 0x84309C82, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Goalkeeper
	{ 0x799F94FB, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Hammal
	{ 0xE9FD6359, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Happy Hour
	{ 0xE4F5E858, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Hell
	{ 0x6773A624, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Helli Mission
	{ 0x45F67C67, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Highway Racing
	{ 0x22E90616, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Hitting
	{ 0xE8E7D672, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Hitting Mice
	{ 0x1349428C, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Horrible Area
	{ 0xCEE9B813, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Hot Speed
	{ 0x82423B3B, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // H.Q. Fighter
	{ 0x28DCC800, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Huarongdao
	{ 0xFAB28AC9, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Star Attack
	{ 0x5AACF702, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Kakuro Master
	{ 0x44739703, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Lawn Purge
	{ 0x3F205D7E, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Little Hacker
	{ 0x87B21602, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // M-Day
	{ 0x7D5D2B45, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Magic Doors
	{ 0x1E4885C0, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Magical Kitchen
	{ 0x46F783BA, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Mars Worm
	{ 0xA07974B2, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Master Chess
	{ 0xAE120741, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Maze Puzzle
	{ 0xD1826398, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Maze Treasure
	{ 0x0D1736B0, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Memory Test
	{ 0x3376CA53, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Labyrinth
	{ 0x1724AA8D, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Burrow
	{ 0x96AF0D0E, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Monkey King
	{ 0xA18B1CDA, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Monster Brother
	{ 0xAFB4712B, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Monster War
	{ 0x93A3BDDE, 256,  1,  0,   0,  0,  TV_NTSC,   0, 0 }, // Boat Race
	{ 0x08F05ACE, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Ocean Prospect
	{ 0x49E630EB, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Overspeed Racing
	{ 0xC3591406, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Panda
	{ 0x73A04CA2, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Penalty Kick
	{ 0x7F8B6CB2, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Penguin
	{ 0x7C80FFCD, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Pinball
	{ 0x4186354E, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Ping Pong
	{ 0x40B1A634, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Plumber
	{ 0x233A7C40, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Pop Monster
	{ 0x95102C1C, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Pulver
	{ 0xA0719250, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Radish Field
	{ 0x7470DD81, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Road Hero
	{ 0x06C6208B, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Robot
	{ 0x76A82BF1, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Santa Claus
	{ 0x4D61FF26, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Seaman
	{ 0x46125A82, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Shooting Balloons
	{ 0x9908F90F, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Shot Put
	{ 0x652838DD, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Shudu
	{ 0x255A6B3A, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Slalom
	{ 0x8F5B7974, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Slots
	{ 0x9893246F, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Sniper
	{ 0xB0731F7F, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Solitaire
	{ 0x160F2F23, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Enchanter
	{ 0x17BE7A7E, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Ghost Palace
	{ 0x21E48C03, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Battlefield
	{ 0x28503E15, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Square
	{ 0x4DFC2F7D, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Star Crossed
	{ 0xAC4E2F51, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Strong Pill
	{ 0x923A41E0, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Submarine War
	{ 0x8FC69C33, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Xtreme Robot
	{ 0xDD417693, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Super Racing
	{ 0x74432073, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Super Tank
	{ 0x8E078991, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Super Ward
	{ 0x03610DA2, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Table Tennis
	{ 0xC667F2E7, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Tank
	{ 0xD2B97B5A, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Tennis Ball
	{ 0x675750B3, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Tetris
	{ 0x6C0AC952, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Tower of Doom
	{ 0xCEDBC625, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Twin Copters
	{ 0x56D09276, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Twin Fish
	{ 0xD13123A3, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // UFO Capture
	{ 0xD6637BD8, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Underwater Rescue
	{ 0x586FA4FE, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Undersea
	{ 0xD2C5A423, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Vectron
	{ 0xCA147E00, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Volleyball
	{ 0x6D10FBC1, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Warrior
	{ 0x3C032C3C, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Water Fire
	{ 0x149C973A, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Water Skiing
	{ 0x32C9D45E, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Way Out
	{ 0x266ECFDD, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Wild Shooting
	{ 0xDF6E086C, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Xmas Gift
	{ 0x826FC896, 256,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Super Fight I (three levels)
	// Waixing FS005
	{ 0x8CAAFF73, 176,  0,  2,0x88,  7, TV_DENDY,   0, 0 }, // Baoxiao Sanguo
	{ 0x6F46A940, 176,  0,  2,0x88,  0, TV_DENDY,   0, 0 }, // Chu Han Zengba
	{ 0x8A32F4EC, 176,  0,  2,0x88,  0, TV_DENDY,   0, 0 }, // Chu Liu Xiang
	{ 0xC246E283, 176,  0,  2,0x88,  0, TV_DENDY,   0, 0 }, // Chu Liu Xiang [f1]
	{ 0x901F62BB, 176,  0,  2,0x88,  0, TV_DENDY,   0, 0 }, // Diguo Fengbao - Napoleon's War
	{ 0x5EE2EF97, 176,  0,  2,0x88,  7,  TV_NTSC,   0, 0 }, // Diguo Shidai (Age of Empires)
	{ 0x8650FF71, 176,  0,  2,0x88,  0, TV_DENDY,   0, 0 }, // Dongzhou Lie Guo Zhi
	{ 0x5A35B224, 176,  0,  2,0x88,  0, TV_DENDY,   0, 0 }, // Duobao Xiao Ying Hao
	{ 0xC7E3F93A, 176,  0,  2,0x88,  7, TV_DENDY,   0, 0 }, // Feng Yun
	{ 0xF354D847, 176,  0,  2,0x88,  7, TV_DENDY,   0, 0 }, // Grandia
	{ 0xD2492676, 176,  0,  2,0x88,  0,  TV_NTSC,   0, 0 }, // Le Comte de Monte-Cristo
	{ 0xE3264F2F, 176,  0,  2,0x88,  7, TV_DENDY,   0, 0 }, // Jie A
	{ 0x7C22C219, 176,  0,  2,0x88,  0,  TV_NTSC,   0, 0 }, // Unrivalled Hero
	{ 0x2C3D4EF0, 176,  0,  2,0x88,  7, TV_DENDY,   0, 0 }, // Fei Cui
	{ 0x49F22159, 176,  0,  2,0x88,  7, TV_DENDY,   0, 0 }, // Mei Guo Fu Hao
	{ 0x81C4A95F, 176,  0,  2,0x88,  7, TV_DENDY,   0, 0 }, // M&M Heroes
	{ 0xD97C076C, 176,  0,  2,0x88,  0, TV_DENDY,   0, 0 }, // Mo Shen Fa Shi
	{ 0x950D3B95, 176,  0,  2,0x88,  0, TV_DENDY,   0, 0 }, // Mo Yu Ying Xiong Chuan
	{ 0xA2DC64FF, 176,  0,  2,0x88,  7, TV_DENDY,   0, 0 }, // Po Fu Chen Zhou
	{ 0x377FDB36, 176,  0,  2,0x88,  7, TV_DENDY,   0, 0 }, // Pocket Monsters Gold
	{ 0x8EB4BB51, 164,  0,  2,0x70,  7, TV_DENDY,   0, 0 }, // Pocket Monsters Red
	{ 0x8F6AB5AC, 176,  0,  2,0x88,  7, TV_DENDY,   0, 0 }, // Sanguo Zhong Lie Zhuan
	{ 0xC768098B, 176,  0,  2,0x88,  7, TV_DENDY,   0, 0 }, // Sianxia Wuyi
	{ 0xF29C8186, 176,  0,  2,0x88,  7, TV_DENDY,   0, 0 }, // Shanghai Tycoon
	{ 0x6444006D, 176,  0,  2,0x88,  7, TV_DENDY,   0, 0 }, // Shao Nian Youxia
	{ 0x5A88B5B0, 176,  0,  2,0x88,  7, TV_DENDY,   0, 0 }, // Digimon Crystal
	{ 0x6B4CAC80, 176,  0,  2,0x88,  7, TV_DENDY,   0, 0 }, // Shuihu Shenshou
	{ 0x24E66DF4, 176,  0,  2,0x88,  0, TV_DENDY,   0, 0 }, // Shuhu Zhuan
	{ 0xB1274C83, 176,  0,  2,0x88,  0, TV_DENDY,   0, 0 }, // Suitang Yanyi
	{ 0x89E0502A, 176,  0,  2,0x88,  0,  TV_NTSC,   0, 0 }, // Super Daisenryaku
	{ 0x3FF36623, 176,  0,  2,0x88,  7, TV_DENDY,   0, 0 }, // Xi Chu Ba Wang
	{ 0x5581EF63, 176,  0,  2,0x88,  0, TV_DENDY,   0, 0 }, // Xia Ke Qing
	{ 0x84966C88, 523,  0,  2,0x70,  7,  TV_NTSC,   0, 0 }, // 封神榜: 伏魔三太子 (晶科泰)
	// Waixing Other
	{ 0x07B7EDA9, 242,  0,  2,0x70,  7, TV_DENDY,   0, 0 }, // Xiao Ao Jiang Wu
	{ 0x2E27E0AF, 242,  0,  2,0x70,  7, TV_DENDY,   0, 0 }, // Bio Hazard
	{ 0xBABE5C1C,  15,  0,  2,0x70,  7, TV_DENDY,   0,-1 }, // Bio Hazard [b]
	{ 0x0A2F5560,  15,  0,  2,0x70,  7, TV_DENDY,   0,-1 }, // Bio Hazard [b]
	{ 0x99051CB5, 176,  0,  2,0x88,  7, TV_DENDY,   0, 0 }, // Xiongba Tianxia
	{ 0x34BEAB31, 176,  0,  2,0x88,  0, TV_DENDY,   0, 0 }, // Yang's Troops
	{ 0xAF89E6DD, 176,  0,  2,0x88,  7, TV_DENDY,   0, 0 }, // Ying Lie Qun Xia Zhuan
	{ 0xB4C53114, 176,  0,  2,0x88,  0, TV_DENDY,   0, 0 }, // Zhanguo Qunxiong Chuan
	{ 0xEDEB4F9C, 176,  0,  2,0x88,  0, TV_DENDY,   0, 0 }, // Zheng Ba Shiji
	{ 0x312ECB3E, 176,  0,  2,0x88,  7, TV_DENDY,   0, 0 }, // 龙域天下 (Radia 戦記)
	{ 0x095D8678, 176,  0,  2,0x88,  7,  TV_NTSC,   0, 0 }, // 双月传 (Double Moon 伝説)
	{ 0x7696573A, 176,  0,  2,0x88,  7,  TV_NTSC,   0, 0 }, // 亚特鲁战记 (Ys)
	{ 0x69A3CA5C, 176,  0,  2,0x88,  7,  TV_NTSC,   0, 0 }, // 哆啦A梦: 超时空历险 (ドラえもん: Gigazombiの逆襲)
	{ 0x02380A27, 176,  0,  2,0x88,  7,  TV_NTSC,   0, 0 }, // 大盗伍佑卫门之天下宝藏 (がんばれゴエモン外伝2: 天下の財宝)
	{ 0xB07D9080, 176,  0,  2,0x88,  7,  TV_NTSC,   0, 0 }, // 女神的救赎 (おたくの星座꞉ An Adventure in the Otaku Galaxy)
	{ 0xEABBB630, 176,  0,  2,0x88,  7,  TV_NTSC,   0, 0 }, // 怪物制造者1 (Monster Maker)
	{ 0xF1D40F5B, 176,  0,  2,0x88,  7,  TV_NTSC,   0, 0 }, // 怪物制造者1 (Monster Maker)
	{ 0x5077CAC1, 176,  0,  2,0x88,  0,  TV_NTSC,   0, 0 }, // 八宝奇珠
	{ 0xFA572047, 176,  0,  2,0x88,  7,  TV_NTSC,   0, 0 }, // 梦境之王 (Dream Master)
	{ 0xBFF7C60C, 176,  0,  2,0x88,  7,  TV_NTSC,   0, 0 }, // 激斗圣战士 (ビックリマン World: 激闘聖戦士)
	{ 0xADBDF725, 176,  0,  2,0x88,  7,  TV_NTSC,   0, 0 }, // 银白色沙加 (Silva Saga)
	{ 0x05E63EED, 176,  0,  2,0x88,  7,  TV_NTSC,   0, 0 }, // 高达骑士物语 (SD Gundam 外伝 - Knight Gundam 物語)
	{ 0xB76205B5, 176,  0,  2,0x88,  7,  TV_NTSC,   0, 0 }, // 上古神殿 (SD Gundam 外伝 - Knight Gundam 物語 2: 光の騎士)
	{ 0x65F30078, 176,  0,  2,0x88,  7,  TV_NTSC,   0, 0 }, // 高达骑士物语2: 光之骑士 (SD Gundam 外伝 - Knight Gundam 物語 2: 光の騎士)
	{ 0xEB08BFE9, 176,  0,  2,0x88,  7,  TV_NTSC,   0, 0 }, // 鬼魅战记 (魍魎戦記 Madara)
	{ 0xB9C8BB09, 176,  0,  2,0x88,  7,  TV_NTSC,   0, 0 }, // 龙珠Z3: 人造人列传 (Dragon Ball Z III: 烈戦人造人間)
	{ 0x12B496E2, 176,  0,  2,0x88,  7,  TV_NTSC,   0, 0 }, // DBZ Gaiden
	{ 0xA3279880, 176,  0,  2,0x88,  7,  TV_NTSC,   0, 0 }, // DBZ II
	{ 0xA2FCAA3F, 176,  0,  2,0x88,  7,  TV_NTSC,   0, 0 }, // DBZ II
	{ 0x2048458C, 176,  0,  2,0x88,  7,  TV_NTSC,   0, 0 }, // DBZ II
	{ 0x835E563A, 176,  0,  2,0x88,  7, TV_DENDY,   0, 0 }, // 混沌世界 (Chaos World)
	{ 0x50E9C2B5, 176,  0,  2,0x88,  7, TV_DENDY,   0,-1 }, // 混沌世界 (Chaos World) [VirtuaNES hack]
	{ 0x9993304C, 176,  0,  2,0x88,  7, TV_DENDY,   0,-1 }, // 混沌世界 (Chaos World) [VirtuaNES hack 2]
	{ 0xB0B13DBD, 178,  0,  2,0x70,  7, TV_DENDY,   0, 5 }, // Pokemon Emerald - Coolboy version
	{ 0x925926BC, 178,  0,  2,0x70,  7, TV_DENDY,   0, 5 }, // Pokemon Diamond - Coolboy version
	{ 0xCD8FF973, 195,  0,  2,0x76,  6,  TV_NTSC,   0, 0 }, // 天神之剑 (God Slayer)
	{ 0x095DA3A2, 195,  0,  2,0x76,  6,  TV_NTSC,   0, 0 }, // 铁血联盟 (God Slayer)
	{ 0x2CA9BF7F, 199,  0,  2,0x76,  7,  TV_NTSC,   0, 0 }, // 汤姆历险记 (SquareのTom Sawyer)
	{ 0x2389335F, 199,  0,  2,0x76,  7,  TV_NTSC,   0, 0 }, // 风色幻想 (SquareのTom Sawyer)
	{ 0x48244391, 164,  0,  2,0x70,  7, TV_DENDY,   0, 0 }, // 宠物: 红
	{ 0x0F281589, 194,  0,  2,0x70,  5,  TV_NTSC,   0, 0 }, // 反恐风暴之海湾战争
	{ 0xEAE675EA, 194,  0,  2,0x70,  5,  TV_NTSC,   0, 0 }, // 第2次: 超级机器人大战
	{ 0x270A4DA6, 194,  0,  2,0x70,  5,  TV_NTSC,   0, 0 }, // 第2次: 超级机器人大战
	{ 0x7C28E1E1, 194,  0,  2,0x70,  5,  TV_NTSC,   0, 0 }, // 第2次: 超级机器人大战
	// Jngota KT-008
	{ 0x61FC4D20, 224,  0,  2,0x70,  7, TV_DENDY,   0, 0 }, // 口袋怪兽: 水晶版
	{ 0x4D735CB1, 224,  0,  2,0x70,  7, TV_DENDY,   0, 0 }, // 口袋怪兽: 钻石版
	{ 0x4F427110, 224,  0,  2,0x70,  7, TV_DENDY,   0, 0 }, // 口袋怪兽: 珍珠版
	{ 0xE001DE16, 224,  0,  2,0x70,  7, TV_DENDY,   0, 0 }, // 口袋怪兽: 白金版
	{ 0x9E9B5884, 224,  0,  2,0x70,  7, TV_DENDY,   0, 0 }, // 口袋怪兽: 白玉版
	{ 0x5464D7F8, 224,  0,  2,0x70,  7, TV_DENDY,   0, 0 }, // 口袋怪兽: 翡翠版
	{ 0xCB1BAB3D, 224,  0,  2,0x70,  7, TV_DENDY,   0, 0 }, // 三国志: 蜀汉风云
	{ 0xF8B58B59, 224,  0,  2,0x70,  7, TV_DENDY,   0, 0 }, // 三国志: 蜀魏争霸
	{ 0xCB524B42, 224,  0,  2,0x70,  7, TV_DENDY,   0, 0 }, // 征战天下
	{ 0x24750E5D, 224,  0,  2,0x70,  7, TV_DENDY,   0, 0 }, // 傲视天地
	{ 0xE05FC21F, 224,  0,  2,0x70,  7, TV_DENDY,   0, 0 }, // Ys: 菲尔盖纳之誓约
	{ 0x16143319, 224,  0,  2,0x70,  7, TV_DENDY,   0, 0 }, // Ys VI: 纳比斯订的方舟
	{ 0xB0D011D3, 224,  0,  2,0x70,  7, TV_DENDY,   0, 0 }, // Ys Origin
	{ 0xAA666C19, 224,  0,  2,0x70,  7, TV_DENDY,   0, 0 }, // Ys Origin
	{ 0x4D2811C7, 224,  0,  2,0x70,  7, TV_DENDY,   0, 0 }, // 伏魔英雄传
	{ 0x76BBE916, 224,  0,  2,0x70,  7, TV_DENDY,   0, 0 }, // 落日征战
	{ 0x9B518D54, 224,  0,  2,0x70,  7, TV_DENDY,   0, 0 }, // 轩辕剑: 云的彼端
	{ 0x5D04547C, 224,  0,  2,0x70,  7, TV_DENDY,   0, 0 }, // 失落的神器
	{ 0x36DE88E7, 224,  0,  2,0x70,  7, TV_DENDY,   0, 0 }, // 新魔界
	{ 0x91396B3F, 224,  0,  2,0x70,  7, TV_DENDY,   0, 0 }, // 轩辕剑: 天之痕
	{ 0xEA831217, 224,  0,  2,0x70,  7, TV_DENDY,   0, 0 }, // 剑侠情缘
	{ 0x5F362198, 224,  0,  2,0x70,  7, TV_DENDY,   0, 0 }, // 战神世界
	{ 0xF8E9C9CF, 224,  0,  2,0x70,  7, TV_DENDY,   0, 0 }, // 无双乱舞
	{ 0xAA621FA0, 224,  0,  2,0x70,  7, TV_DENDY,   0, 0 }, // 轩辕剑: 枫之舞
	{ 0xA4C39535, 224,  0,  2,0x70,  7, TV_DENDY,   0, 0 }, // 神魔大陆
	{ 0xDDC122ED, 224,  0,  2,0x70,  7, TV_DENDY,   0, 0 }, // 亡灵崛起
	{ 0xC658B6A8, 224,  0,  2,0x70,  7, TV_DENDY,   0, 0 }, // 刀剑英雄传
	{ 0x48D1F54A, 224,  0,  2,0x70,  7, TV_DENDY,   0, 0 }, // 轩辕剑: 王者归来
	{ 0x48210324, 224,  0,  2,0x70,  7,  TV_NTSC,   0, 0 }, // 勇者鬥惡龍: 勇者的试练
	{ 0x92EBAD5B, 224,  0,  2,0x70,  7,  TV_NTSC,   0, 0 }, // 勇者鬥惡龍 3: 罪恶渊源
	{ 0xB5FDB3CB, 224,  0,  2,0x70,  7,  TV_NTSC,   0, 0 }, // 勇者黑暗世界: 混沌世界
	// CHR-ROM with Security Feature
	{ 0x2A629F7D, 185,  7,  1,   0,  0,  TV_NTSC,   0, 0, 0xA8A9B982 }, // Bird Week
	{ 0x725D5CC4, 185,  7,  0,   0,  0,  TV_NTSC,   0, 0, 0x74F0A89F }, // B-Wings
	{ 0x81A743F5, 185,  6,  0,   0,  0,  TV_NTSC,   0, 0, 0x87D7CAF0 }, // Family Computer Othello
	{ 0xAA755715, 185,  5,  0,   0,  0,  TV_NTSC,   0, 0, 0x9C521240 }, // Mighty Bomb Jack (Japan rev0)
	{ 0x49FF2E0B, 185,  5,  0,   0,  0,  TV_NTSC,   0, 0, 0xD40FA953 }, // Mighty Bomb Jack (Japan rev1)
	{ 0x7ECB759E, 185,  5,  0,   0,  0,   TV_PAL,   0, 0, 0x765ABC94 }, // Mighty Bomb Jack (PAL stripped)
	{ 0x18BA5AC0, 185,  5,  0,   0,  0,  TV_NTSC,   0, 0, 0x02669C33 }, // Mighty Bomb Jack (U.S. stripped)
	{ 0xA03C98A7, 185,  4,  1,   0,  0,  TV_NTSC,   0, 0, 0x0F05FF0A }, // Seicross
	{ 0xB36457C7, 185,  5,  1,   0,  0,  TV_NTSC,   0, 0, 0xCF0C9D97 }, // Spy versus Spy
	{ 0x87A3F91E, 185,  6,  1,   0,  0,  TV_NTSC,   0, 0, 0x8E62D229 }, // Sansû 1-nen Keisan Game
	{ 0xDAADEBFA, 185,  6,  1,   0,  0,  TV_NTSC,   0, 0, 0x892CBBC2 }, // Sansû 2-nen Keisan Game
	{ 0x73DE64AD, 185,  6,  1,   0,  0,  TV_NTSC,   0, 0, 0x87DA4BD0 }, // Sansû 3-nen Keisan Game
	// Cony games
	{ 0xD4B35E2A,   4,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Street Fighter V (20 People) [p1]
	{ 0x6CFBEAE9,   4,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Street Fighter V Turbo 60 [p1]
	{ 0xF7B93E43, 196,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Street Fighter VI 16 Peoples (Unl)
	{ 0x045CB9EC,  83,  1,  0,   0,  0,  TV_NTSC,   0, 0 }, // Super Blaster VII Turbo 28
	{ 0x5721089A,  83,  2,  2,0x90,  0,  TV_NTSC,   0, 0 }, // Dragon Ball Z Party
	{ 0x3ABB4708,  23,  1,  0,   0,  0,  TV_NTSC,   0, 0 }, // 英雄傳 World Hero
	// Daou games
	{ 0x86BE4746,   2,  1,  1,   0,  7,  TV_NTSC,   0, 0 }, // Dooly Bravo Land
	// Waixing games
	{ 0x8B41D49C, 164,  0,  2,0x70,  7,  TV_DENDY,  0, 0 }, // Pet Evolve
	{ 0x02C41438, 176,  0,  2,0x70,  7,  TV_DENDY,  0, 0 }, // 星河战士
	{ 0xB5E83C9A, 241,  0,  2,0x70,  7,  TV_DENDY,  0, 0 }, // 星际争霸
	{ 0x935F2119, 199,  0,  2,0x76,  7,  TV_NTSC,   0, 0 }, // Genghis Khan
	{ 0xBFF2E9EC, 176,  0,  2,0x70,  7,  TV_NTSC,   0, 0 }, // Columbus (Chinese)
	{ 0x345EE51A, 245,  0,  2,0x70,  7,  TV_NTSC,   0, 0 }, // Dragon Quest VII
	{ 0xFB2B6B10, 241,  0,  2,0x70,  7,  TV_DENDY,  0, 0 }, // Grandia
	{ 0x78FB3ED6,  74,  0,  2,0x70,  5,  TV_DENDY,  0, 0 }, // Jia A
	{ 0xE8EAFBC1, 176,  0,  2,0x88,  7,  TV_NTSC,   0, 0 }, // ES-1137
	// Huang Xinwei
	{ 0x86DBA660, 355,  0,  1,   0,  7,  TV_NTSC,   0, 0 }, // 3D Block (巨晶) CANNOT RUN
	{ 0xEFE3D606, 355,  0,  1,   0,  7,  TV_NTSC,   0, 0 }, // 3D Block (Game Master) CANNOT RUN
	{ 0x15612DC8,  27,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Mihunche
	{ 0xFAD97471,  27,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Mihunche
	{ 0x3C10F11C,  27,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Mihunche
	{ 0x3FAFAA4A, 152,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // B.B. Car (usually set to mapper 7, but mapper 7 has no CHR-ROM)
	{ 0xDA587B2C,  11,  0,  0,   0,  0, TV_DENDY,   0, 0 }, // Balda (usually set to mapper 7, but mapper 7 has no CHR-ROM)
	{ 0x13188C21,  11,  0,  0,   0,  0, TV_DENDY,   0, 0 }, // Cvetnye Linii
	// Konami VRC 2/4 games
	{ 0x93794634,  23,  2,  0,   0,  0,  TV_NTSC,   0, 0 }, // Akumajô Special: Boku Dracula-kun
	{ 0x974DFB6A,  23,  2,  0,   0,  0,  TV_NTSC,   0, 0 }, // Akumajô Special: Boku Dracula-kun
	{ 0xB3ED6B8B,  23,  2,  0,   0,  0,  TV_NTSC,   0, 0 }, // Akumajô Special: Boku Dracula-kun
	{ 0xC7829DAE,  23,  2,  0,   0,  0,  TV_NTSC,   0, 0 }, // Akumajô Special: Boku Dracula-kun
	{ 0xBF80FEDC,  23,  2,  0,   0,  0,  TV_NTSC,   0, 0 }, // Akumajô Special: Boku Dracula-kun
	{ 0xD71DBEAB,  23,  2,  0,   0,  0,  TV_NTSC,   0, 0 }, // Akumajô Special: Boku Dracula-kun
	{ 0xD0E0C599,  23,  2,  0,   0,  0,  TV_NTSC,   0, 0 }, // Akumajô Special: Boku Dracula-kun
	{ 0xB87D85EE,  23,  2,  0,   0,  0,  TV_NTSC,   0, 0 }, // Akumajô Special: Boku Dracula-kun
	{ 0xF82DC02F,  23,  2,  0,   0,  0,  TV_NTSC,   0, 0 }, // Akumajô Special: Boku Dracula-kun
	{ 0x90B08058,  23,  2,  0,   0,  0,  TV_NTSC,   0, 0 }, // Akumajô Special: Boku Dracula-kun
	{ 0x9D00E2F6,  23,  2,  0,   0,  0,  TV_NTSC,   0, 0 }, // Akumajô Special: Boku Dracula-kun
	{ 0xCE9F4336,  23,  2,  0,   0,  0,  TV_NTSC,   0, 0 }, // Akumajô Special: Boku Dracula-kun
	{ 0x9CAE70ED,  23,  2,  0,   0,  0,  TV_NTSC,   0, 0 }, // Akumajô Special: Boku Dracula-kun
	{ 0x6A50B553, 272,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Akumajô Special: Boku Dracula-kun
	{ 0x0BBD85FF,  25,  1,  0,   0,  0,  TV_NTSC,   0, 0 }, // Bio Miracle Boku-tte Upa (ROM version)
	{ 0x627B2E4A,  25,  1,  0,   0,  0,  TV_NTSC,   0, 0 }, // Bio Miracle Boku-tte Upa (ROM version)
	{ 0x8E425C5C,  25,  1,  0,   0,  0,  TV_NTSC,   0, 0 }, // Bio Miracle Boku-tte Upa (ROM version)
	{ 0x2BE6D55D,  25,  1,  0,   0,  0,  TV_NTSC,   0, 0 }, // Bio Miracle Boku-tte Upa (ROM version)
	{ 0x23D85105,  25,  1,  0,   0,  0,  TV_NTSC,   0, 0 }, // Bio Miracle Boku-tte Upa (ROM version)
	{ 0x196DEA3E,  25,  1,  0,   0,  0,  TV_NTSC,   0, 0 }, // Bio Miracle Boku-tte Upa (ROM version)
	{ 0x7C65F9FD,  25,  1,  0,   0,  0,  TV_NTSC,   0, 0 }, // Bio Miracle Boku-tte Upa (ROM version)
	{ 0x0A6DF706,  25,  1,  0,   0,  0,  TV_NTSC,   0, 0 }, // Bio Miracle Boku-tte Upa (ROM version)
	{ 0x919AC0FE,  23,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Contra
	{ 0x089CAB72,  23,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Contra
	{ 0xC4260135,  23,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Contra
	{ 0x798157DD,  23,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Contra
	{ 0x8A093F62,  23,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Contra
	{ 0x3078CDA9,  23,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Contra
	{ 0xA15FACB3,  23,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Contra
	{ 0x10D38255,  23,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Contra
	{ 0x65E8B25C,  23,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Contra
	{ 0xCA8D7E9B,  23,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Contra
	{ 0x5B4F8B0C,  23,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Contra
	{ 0x78729598,  23,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Contra
	{ 0xF9ED6B48,  23,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Contra
	{ 0xF724571F,  23,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Contra
	{ 0x8499A998,  23,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Contra
	{ 0x489AC307,  23,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Contra
	{ 0xA691BDFC,  23,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Contra
	{ 0x94AB7154,  23,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Contra
	{ 0x1B2E5E78,  23,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Contra
	{ 0xF5FBDB4B,  23,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Contra
	{ 0x759382DC,  23,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Contra
	{ 0xCE1239B6,  23,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Contra
	{ 0xBEBD7B44,  23,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Contra
	{ 0x21596528,  23,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Contra
	{ 0xAC220CDF,  23,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Contra
	{ 0xC678C2FB,  23,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Contra
	{ 0x862B3ADC,  23,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Contra
	{ 0xBEA0B526,  23,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Contra
	{ 0x5D5EDB43,  23,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Contra
	{ 0x987379C9,  23,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Contra
	{ 0x9770349A,  23,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Contra
	{ 0xC8F8CDD2,  23,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Contra
	{ 0x053B4969,  23,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Contra
	{ 0xECAAF485,  23,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Contra
	{ 0x18F4E9AA,  23,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Contra (Sample)
	{ 0x99580334,  23,  2,  0,   6,  0,  TV_NTSC,   0, 0 }, // Crisis Force
	{ 0xDAEB93ED,  23,  2,  0,   6,  0,  TV_NTSC,   0, 0 }, // Crisis Force
	{ 0x547B6723,  23,  2,  0,   6,  0,  TV_NTSC,   0, 0 }, // Crisis Force
	{ 0xE7B61485,  23,  2,  0,   6,  0,  TV_NTSC,   0, 0 }, // Crisis Force
	{ 0xAB7471C8,  23,  2,  0,   6,  0,  TV_NTSC,   0, 0 }, // Crisis Force
	{ 0xF10DC6CE,  23,  2,  0,   6,  0,  TV_NTSC,   0, 0 }, // Crisis Force
	{ 0x0AC90026,  23,  2,  0,   6,  0,  TV_NTSC,   0, 0 }, // Crisis Force
	{ 0x3056E653,  23,  2,  0,   6,  0,  TV_NTSC,   0, 0 }, // Crisis Force
	{ 0x670F0A03,  23,  2,  0,   6,  0,  TV_NTSC,   0, 0 }, // Crisis Force
	{ 0x6B4B65B3,  23,  2,  0,   6,  0,  TV_NTSC,   0, 0 }, // Crisis Force
	{ 0xD48AA241,  23,  2,  0,   6,  0,  TV_NTSC,   0, 0 }, // Crisis Force
	{ 0x14B027E4,  23,  2,  0,   6,  0,  TV_NTSC,   0, 0 }, // Crisis Force
	{ 0xBDDFBE00,  23,  2,  0,   6,  0,  TV_NTSC,   0, 0 }, // Crisis Force
	{ 0x009B3545,  23,  2,  0,   6,  0,  TV_NTSC,   0, 0 }, // Crisis Force
	{ 0xA33D6628,  23,  2,  0,   6,  0,  TV_NTSC,   0, 0 }, // Crisis Force
	{ 0x90BD41BF,  23,  1,  0,   6,  0,  TV_NTSC,   0, 0 }, // Crisis Force (Sample)
	{ 0xF60F6667,  23,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Dragon Scroll: Yomigaerishi Maryû
	{ 0xF6D4A11F,  23,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Dragon Scroll: Yomigaerishi Maryû
	{ 0xC8DE4081,  23,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Dragon Scroll: Yomigaerishi Maryû
	{ 0x112140A4,  23,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Ganbare Goemon 2
	{ 0x8CC2F7C5,  23,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Ganbare Goemon 2
	{ 0x968FE3E3,  23,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Ganbare Goemon 2
	{ 0xCF25C7FE,  23,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Ganbare Goemon 2
	{ 0xB8D3A041,  23,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Ganbare Goemon 2
	{ 0x5462B028,  23,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Ganbare Goemon 2
	{ 0xACA59227,  23,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Ganbare Goemon 2
	{ 0x8360FA88,  25,  3,  2,0x70,  0,  TV_NTSC,   0, 0 }, // Ganbare Goemon Gaiden: Kieta Ôgon Kiseru
	{ 0x7CC4778C,  21,  2,  2,0x70,  0,  TV_NTSC,   0, 0 }, // Ganbare Goemon Gaiden 2: Tenka no Zaihô
	{ 0xC0905A6A,  21,  2,  2,0x70,  0,  TV_NTSC,   0, 0 }, // Ganbare Goemon Gaiden 2: Tenka no Zaihô
	{ 0x9E0BF355,  21,  2,  2,0x70,  0,  TV_NTSC,   0, 0 }, // Ganbare Goemon Gaiden 2: Tenka no Zaihô
	{ 0x953CA1B6,  22,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Ganbare Pennant Race!
	{ 0xC8859038,  23,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Getsufû Maden
	{ 0xD57C4190,  23,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Getsufû Maden
	{ 0xF3E394D1,  23,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Getsufû Maden
	{ 0x3D58228E,  23,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Getsufû Maden
	{ 0x73677D5C,  23,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Getsufû Maden
	{ 0x3D58228E,  23,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Getsufû Maden
	{ 0xF3E394D1,  23,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Getsufû Maden
	{ 0xA27600C5,  23,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Getsufû Maden
	{ 0xC71D4CE7,  25,  1,  0,   6,  0,  TV_NTSC,   0, 0 }, // Gradius II
	{ 0x5A3EC21C,  23,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Jarinko Chie - Bakudan Musume no Shiawase Sagashi
	{ 0xAC6C95F3,  23,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Mad City (Sample)
	{ 0xE4CEEAD1,  23,  2,  0,   7,  0,  TV_NTSC,   0, 0 }, // Parodius da!
	{ 0xA2E68DA8,  25,  1,  0,   0,  0,  TV_NTSC,   0, 0 }, // Racer Mini 4-ku
	{ 0xEA74C587,  25,  2,  0,   0,  0,  TV_NTSC,   0, 0 }, // Teenage Mutant Ninja Turtles
	{ 0x5F82CB7D,  25,  2,  0,   0,  0,  TV_NTSC,   0, 0 }, // Teenage Mutant Ninja Turtles 2: The Manhattan Project
	{ 0x7A508DBB,  22,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // TwinBee 3: Poko Poko Daimaô
	{ 0xAF49A5B9,  23,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // TwinBee 3: Poko Poko Daimaô (Sample)
	{ 0x64818FC5,  23,  3,  0,   0,  0,  TV_NTSC,   0, 0 }, // Konami Wai Wai World
	{ 0xB201B522,  21,  1,  0,   0,  0,  TV_NTSC,   0, 0 }, // Wai Wai World 2: SOS!! Parsley-jô
	{ 0x592175B3, 520,  0,  0,   0,  7,  TV_NTSC,   0, 0 }, // Datach Yuu Yuu Hakusho + Datach Dragon Ball Z
	{ 0xA27EB838,  23,  1,  0,   0,  7,  TV_NTSC,   0, 0 }, // World Hero (rev1)
	// Joy Van/Sachen Games
	{ 0xBCA0FF02,  79,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Galactic Crusader (Sachen)
	{ 0x8E53F4D8,  79,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Lucky 777
	{ 0xEB3A7E9C, 133,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Jovial Race
	{ 0xC34B0E4C, 136,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Diébiàn
	{ 0xD7DF9B60,  79,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Papillon
	{ 0x7B29979F,  79,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // Metal Fighter µ (Kinema Music)
	{ 0x290A1202, 262,  0,  8,   0,  7,  TV_NTSC,   0, 0 }, // Street Heroes
	{ 0x39B481C4, 150,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Cosmo Cop + Cyber Monster
	{ 0xB4F460F8, 150,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Cosmo Cop + Cyber Monster
	{ 0xE93400B2, 150,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Poker III
	{ 0x1FF8DACE,   0,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // Pyramid
	{ 0x3ACDCC64, 512,  0, 10,0x70,  7,  TV_NTSC,   0, 0 }, // Zhongguo Daheng
	{ 0x3A36A48C,   3,  1,  1,   0,  0,  TV_NTSC,   0, 0 }, // 盜帥 [Mapper 3 Hack]
	{ 0x9989B95A, 243,  0,  0,   0,  0,  TV_NTSC,   0,15, 0x29582CA1 }, // 美女拳
	{ 0x9989B95A, 243,  0,  0,   0,  0,  TV_NTSC,   0,16, 0x695CC180 }, // 美女拳
	// Cony/Yoko Soft/Future Media/Hosekn Games
	{ 0xDB23F02A, 189,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Street Fighter II
	{ 0xB0F32DEF, 189,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Street Fighter II
	{ 0x50EF4987, 189,  1,  0,   0,  0,  TV_NTSC,   0, 0 }, // Street Fighter II
	{ 0x893B02EC, 264,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Mortal Kombat II/Pro V
	{ 0x1CAD83D6,  85,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Mortal Kombat II/Pro V
	// J.Y. Company Games
	{ 0x3E0D61A4,  90,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Mortal Kombat II Turbo
	{ 0x60815A7D, 197,  2,  0,   0,  0,  TV_NTSC,   0, 0 }, // Mortal Kombat III 28 People
	{ 0xA242E696, 209,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Power Rangers III
	{ 0xB8B2596A, 209,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Power Rangers IV
	{ 0x1D720E77,  90,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Aladdin
	// Gouder/Supertone Games
	{ 0x20CA2AD3, 208,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Street Fighter IV
	{ 0x35AF84F3, 208,  1,  0,   0,  0,  TV_NTSC,   0, 0 }, // Street Fighter IV [p]
	{ 0xA677D64E, 116,  1,  0,   0,  0,  TV_NTSC,   0, 0 }, // Garô Densetsu Special
	{ 0x9A24F6A8, 116,  1,  0,   0,  0,  TV_NTSC,   0, 0 }, // Somari-W
	// Kasheng/NT games
	{ 0xBE9E7B69, 121,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Street Fighter Zero 2 '97
	{ 0x5160258A, 121,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Ultimate Mortal Kombat 4
	{ 0xE9CA55A3, 121,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Super 3-in-1 (NT-934)
	{ 0x51CD7301, 121,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Sonic 3-D Blast 6
	{ 0x412B4AD9, 115,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // 包青天
	// SuperGame games
	{ 0xBBFD302A, 215,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Street Fighter Zero 2 '97
	{ 0x6ADC5227,   4,  4,  0,   0,  0,  TV_NTSC,   0, 0 }, // Aladdin (SuperGame)
	{ 0x975A58F4, 215,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Earthworm Jim 2 (SuperGame)
	{ 0xBE5669F4, 215,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Earthworm Jim 2 (SuperGame)
	{ 0xDB8854EE, 263,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Boogerman II (Rex Soft)
	{ 0xC3384691, 114,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // The Lion King
	{ 0xC8252BD3, 258,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Super Hang-On '97
	// Idea-Tek Games
	{ 0x9A1BED01,   3,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Puzzle (Idea-Tek) [hM03]
	{ 0x692D5292, 132,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Mahjong Block (TXC rerelease)
	{ 0x0744648C, 132,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Qiwang
	{ 0xC96C892B, 173,  0,  1,   0,  0,  TV_NTSC,   0, 4, 0x2A5F4C5A }, // Zhanguo Sichuansheng
	// Multicarts
	{ 0xAB2ACA46, 344,  0,  0,   0,  0,  TV_NTSC,   0, 14}, // Kuai Da Jin Ka Zhong Ji Tiao Zhan 3-in-1, 6-in-1
	{ 0xC25FD362, 319,  0,  0,   0,  0,  TV_NTSC,   0, 13}, // Prima Soft 9999999-in-1
	{ 0x4025E0AA, 260,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // 9-in-1 (early version, later version does use Mapper 176)
	{ 0x6C979BAC, 176,  0,  0,   0, 11,  TV_NTSC,   0, 0 }, // 10-in-1 Omake Game
	{ 0x33923995, 256,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // AT-103
	{ 0x28CBD21B, 176,  0,  0,   0,  0,  TV_DUAL,   0, 0 }, // 5/16/22/42/56/62/126-in-1
	{ 0x83A38A2F, 176,  0,  0,   0,  0,  TV_DUAL,   0, 0 }, // 15/18/30/52/58/160/180/288-in-1
	{ 0xE6D869ED, 176,  0,  0,   0,  7,  TV_NTSC,   0, 0 }, // Rockman I-VI
	{ 0x9D20E448, 265,  0,  0,   0,  7,  TV_DUAL,   0, 0 }, // Super HIK 6-in-1 (EK-601)
	{ 0x123674F4, 260,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Aladdin + The Lion King
	{ 0xBF1CE2BC, 260,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Boogerman + The Flintstones
	{ 0x66D0C4D5, 260,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Mortal Kombat V Turbo 30 + Super Aladdin
      //{ 0xE1E81430,  47,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // ????? Mortal Kombat V Turbo 30 + Super Aladdin
	{ 0x46E2045D, 176,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Sonic 5 + Earthworm Jim 3
	{ 0x1D75FD35, 256,  0,  0,   0,  0, TV_DENDY,   0, 0 }, // Street Dance + Hit Mouse
	{ 0xFD9D1925, 176,  0,  0,   0,  0,  TV_DUAL,   0, 0 }, // 15/20/80/160-in-1
	{ 0x9C0C5498, 290,  0,  0,   0,  0,  TV_DUAL,   0, 0 }, // Asder 20-in-1
	{ 0xCC4160A3, 176,  0,  0,   0,  0,  TV_DUAL,   0, 0 }, // 3-in-1 Lion King 5
	{ 0xCE4BA157,  90,  0,  0,   0,  0,  TV_DUAL,   0, 0 }, // 1998 Super HIK 45-in-1
	{ 0xA8B1F53C, 281,  0,  0,   0,  0,  TV_DUAL,   0, 0 }, // Super HIK 4-in-1 (JY-052)
	{ 0xF7C6BF33, 282,  0,  0,   0,  0,  TV_DUAL,   0, 0 }, // 1997 Super HIK 21-in-1
	{ 0xDAFE5F4E, 282,  0,  0,   0,  0,  TV_DUAL,   0, 2, 0x31E99313 }, // 1998 Super HIK 5-in-1 [b1]
	{ 0xDAFE5F4E, 282,  0,  0,   0,  0,  TV_DUAL,   0, 0 }, // 1998 Super HIK 5-in-1
	{ 0x2805FBEF,  57,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // 21/35/66/77/888/999/1200/1800/2000/3000/5000/6000/1000000/500000/37/68-in-1
	{ 0x5ED3485B, 256,  3,  0,   0,  0,  TV_DUAL,   0, 0 }, // Z-Dog
	{ 0x1242DA7F, 256,  4,  0,   0,  0,  TV_DUAL,   0, 0 }, // Sports Game 69-in-1
	{ 0x7DAC8EFE, 256,  3,  0,   0,  0,  TV_DUAL,   0, 0 }, // Samuri Star Angel
	{ 0xBDFFA40A, 270,  1,  0,   0,  0,  TV_DUAL,   0, 0 }, // Game Prince RS-16
	{ 0x37D0FB06, 270,  2,  0,   0,  0,  TV_DUAL,   0, 0 }, // Family Pocket 638-in-1
	{ 0x24C802D7, 270,  3,  0,   0,  0,  TV_DUAL,   0, 0 }, // BittBoy 300-in-1
	{ 0x29A8CB96, 269,  0,  0,   0,  0,  TV_DUAL,   0, 0 }, // 15000-in-1
	{ 0x23C0C325, 269,  0,  0,   7,  0,  TV_DUAL,   0, 0 }, // 18000-in-1
	{ 0x0282D975, 269,  0,  0,   0,  0,  TV_DUAL,   0, 0 }, // Games Xplosion 121-in-1
	{ 0x947AC898, 256,  2,  0,   0,  0,  TV_DUAL,   0, 0 }, // Power Joy Supermax 30-in-1
	{ 0x1AB45228, 256,  2,  0,   0,  0,  TV_DUAL,   0, 0 }, // Power Joy Supermax 60-in-1
	{ 0xABB2F974, 241,  0,  1,   7,  7,  TV_DENDY,  0, 0 }, // Study and Game 32-in-1
	{ 0x6175B9A0, 235,  0,  0,   0,  7,  TV_NTSC,   0, 0 }, // 150-in-1
	{ 0xB42D12BC, 285,  0,  0,   0,  7,  TV_NTSC,   0, 0 }, // 3-in-1 N068
	{ 0x822DA381, 285,  1,  0,   0,  7,  TV_NTSC,   0, 0 }, // 4-in-1 JY-066
	{ 0x0CDEB325, 287,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // 4-in-1 (411120-C)
	{ 0x393C3B5C, 265,  0,  0,   0,  7,  TV_NTSC,   0, 0 }, // 4-in-1 (D-010)
	{ 0x1B98E66E, 265,  0,  0,   0,  7,  TV_NTSC,   0, 0 }, // 4-in-1
	{ 0x60C2139F, 265,  0,  0,   0,  7,  TV_NTSC,   0, 0 }, // 8-in-1 A-020
	{ 0x77E9EE96, 265,  0,  0,   0,  7,  TV_NTSC,   0, 0 }, // 11-in-1
	{ 0xBA45171D, 176,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // 4-in-1 (FK23C8078)
	{ 0x21786899, 176,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // 4-in-1 Digital Adventure
	{ 0xC16708E8, 176,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // 4-in-1 KY 6006
	{ 0xE981A993, 176,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // 4-in-1 KY 6009
	{ 0x63A87C95, 176,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // 8-in-1 KY 8002
	{ 0x60C6D8CD, 176,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // 9-in-1 KY 9005
	{ 0x38BA830E, 176,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // DH1043
	{ 0x01E54556, 286,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // BS-5
	{ 0x73255B58, 286,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // BS-5
	{ 0x6DCE148C, 286,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // BS-5
	{ 0x13E55C4C, 286,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // BS-5
	{ 0x9BB72607, 288,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // 21-in-1
	{ 0x9BB72607, 288,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // 21-in-1 with bad CHR in "Soccer" fixed
	{ 0xC6A91589, 288,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // Other 21-in-1
	{ 0x7A423007,  60,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // 46-in-1 (D1038)
	{ 0x4978BA70,  60,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // 65-in-1 (NT766)
	{ 0x2B81E99F,  60,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // 74-in-1 (NT886)
	{ 0xBA6A6F73,  60,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // 77-in-1 (NT141)
	{ 0x766130C4,  60,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // 35-in-1 (VT5201)
	{ 0xE1E81430, 291,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // 2-in-1 (Mortal Kombat 6&7) (NT-639)
	{ 0x09DF723C, 291,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // 2-in-1 (Mortal Kombat 6, Samurai Spirits)
	{ 0x436F9E3F, 291,  0,  0,   0,  0,  TV_NTSC,   0, 1 }, // 2-in-1 (Mortal Kombat 6, Samurai Spirits) [b1]
	{ 0x558C0DC3, 178,  0,  0,   7,  7,  TV_DUAL,   0, 0 }, // ???? Soccer & Crazy Dance
	{ 0x8CDE7CB9, 283,  0,  0,   0,  7,  TV_NTSC,   0, 0 }, // Block Family 7-in-1
	{ 0x2EED2E34, 289,  0,  0,   0,  7,  TV_NTSC,   0, 0 }, // 76-in-1
	{ 0x016D60C5, 241,  0,  1,   0,  7, TV_DENDY,   0, 0 }, // 12-in-1 (Hwang Shinwei)
	{ 0x9BBF3E5D,  15,  0,  0,   0,  7,  TV_NTSC,   0, 0 }, // 168-in-1
	{ 0xC3566FDE, 300,  0,  0,   0,  7,  TV_NTSC,   0, 0 }, // 190-in-1
	{ 0x3E95577D, 261,  0,  0,   0,  7,  TV_NTSC,   0, 0 }, // 200-in-1 - Elfland
	{ 0xD763B292, 217,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // 255-in-1 (Mapper 204)
	{ 0x4AE9EE7D, 301,  0,  0,   0,  7,  TV_NTSC,   0, 0 }, // 4-in-1 1993 (CK-001)
	{ 0x54F2E672, 226,  0,  0,   0,  7,  TV_NTSC,   0, 0 }, // 63-in-1 (Ghostbusters 63-in-1)
	{ 0x38A1A7E5, 314,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // 64-in-1 Y2K
	{ 0xB9E0E671, 348,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // 7-in-1 (M-022)
	{ 0xF821EB8B, 331,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // 7-in-1 (NS03)
	{ 0x834E6407, 215,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Golden Card 6-in-1
	{ 0x58BCACF6,  45,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // Kunio 8-in-1
	{ 0x668D69C2, 236,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, // 70-in-1
	{ 0x0BB4FD7A, 236,  0,  0,   0,  7,  TV_NTSC,   0, 0 }, // 800-in-1
	{ 0xFAC48E47, 343,  0,  1,   0,  7,  TV_NTSC,   0, 0 }, // Sheng Tian 2-in-1 (Kung Fu, Super Mario Bros.)
	{ 0xEB0265B2, 343,  0,  0,   0,  7,  TV_NTSC,   0, 0 }, // Sheng Tian 2-in-1 (B-Wings, TwinBee)
	{ 0xDC835C48, 205,  1,  0,   7,  0,  TV_NTSC,   0, 0 }, // 4-in-1 (K-3131GS, GN-45)
	{ 0x466BDB58, 205,  1,  0,   7,  0,  TV_NTSC,   0, 0 }, // 4-in-1 (K-3131SS, GN-45)
	{ 0x08AD4796, 205,  2,  0,   7,  0,  TV_NTSC,   0, 0 }, // 4-in-1 (OK-411)
	{ 0xBF8E57D4, 205,  3,  0,   7,  0,  TV_NTSC,   0, 0 }, // 7-in-1
	// Shenzhen Nanjing Games using mode 7
	{ 0x9D8AA034, 163,  0,  3,0x70,  7, TV_DENDY,   0,17 }, // Final Fantasy IV - 最终幻想 4꞉ 光与暗 水晶纷争
	{ 0x5E66E6C4, 163,  0,  3,0x70,  7, TV_DENDY,   0,17 }, // Naruto RPG - 火影忍者
	{ 0x9DE10A91, 163,  0,  3,0x70,  7, TV_DENDY,   0,17 }, // 七龙珠大冒险 - Dragon Ball
	{ 0xFC209609, 163,  0,  3,0x70,  7, TV_DENDY,   0,17 }, // 三国大乱斗꞉ 战国 - Orochi
	{ 0x696D98E3, 163,  0,  3,0x70,  7, TV_DENDY,   0,17 }, // 三国志꞉ 吕布传
	{ 0x9F197F2B, 163,  0,  3,0x70,  7, TV_DENDY,   0,17 }, // 三国群侠传
	{ 0x975F64E2, 163,  0,  3,0x70,  7, TV_DENDY,   0,17 }, // 倚天屠龙记
	{ 0x915C5179, 163,  0,  3,0x70,  7, TV_DENDY,   0,17 }, // 傲视三国志
	{ 0x8589652D, 162,  0,  3,0x70,  7, TV_DENDY,   0,17 }, // 农场小精灵
	{ 0x852BDB36, 163,  0,  3,0x70,  7, TV_DENDY,   0,17 }, // 口袋宝石 - 红
	{ 0xB41CF445, 163,  0,  3,0x70,  7, TV_DENDY,   0,17 }, // 口袋宝石 - 蓝
	{ 0x7829C3A9, 163,  0,  3,0x70,  7, TV_DENDY,   0,17 }, // 口袋宝石 - 金
	{ 0xBC383C09, 163,  0,  3,0x70,  7, TV_DENDY,   0,17 }, // 口袋水晶
	{ 0xA9C4712A, 163,  0,  3,0x70,  7, TV_DENDY,   0,17 }, // 口袋钻石
	{ 0xC2B02B71, 163,  0,  3,0x70,  7, TV_DENDY,   0,17 }, // 吞食天地 VI꞉ 刘备传
	{ 0x054444A0, 163,  0,  3,0x70,  7, TV_DENDY,   0,17 }, // 圣斗士星矢꞉ 北欧突击篇
	{ 0x9BA518BA, 163,  0,  3,0x70,  7, TV_DENDY,   0,17 }, // 宠物高达战记
	{ 0x4CE082F8, 163,  0,  3,0x70,  7, TV_DENDY,   0,17 }, // 拳皇R-1꞉ 最强格斗王
	{ 0x99FE9AB5, 163,  0,  3,0x70,  7, TV_DENDY,   0,17 }, // 时空斗士 - Pegasus Senya
	{ 0x82F204AE, 162,  0,  3,0x70,  7, TV_DENDY,   0,17 }, // 梁山英雄
	{ 0x57414FB6, 163,  0,  3,0x70,  7, TV_DENDY,   0,17 }, // 武侠天地
	{ 0x979239DE, 163,  0,  3,0x70,  7, TV_DENDY,   0,17 }, // 毁灭之神
	{ 0xEFF96E8A, 163,  0,  3,0x70,  7, TV_DENDY,   0,17 }, // 魔兽世界꞉ 恶魔猎人
	// Yancheng games
	{ 0x0878A7EE, 164,  1,  2,0x30,  7, TV_DENDY,   0, 0 }, // Dark Seed - 黑暗之蛊
	{ 0x56A0D271, 164,  1,  2,0x37,  7, TV_DENDY,   0, 0 }, // Final Fantasy 太空戰士 V
	{ 0xEFBD1221, 164,  1,  2,0x37,  7, TV_DENDY,   0, 0 }, // Final Fantasy 太空戰士 V
	{ 0xBC7562A6, 164,  1,  2,0x30,  7, TV_DENDY,   0, 0 }, // 口袋精靈 - 金
	{ 0x65F1DB91, 164,  1,  2,0x37,  7, TV_DENDY,   0, 0 }, // 大話西游
	{ 0x0A244228, 164,  1,  2,0x37,  7, TV_DENDY,   0, 0 }, // 岳飛傳
	{ 0x8209BA79, 164,  1,  2,0x30,  7, TV_DENDY,   0, 0 }, // 櫻桃小丸子
	{ 0x228559A7, 164,  2,  2,0x30,  7, TV_DENDY,   0, 0 }, // 口袋精靈 - 水晶
	// Computers
	{ 0x5A7139F5,   0,  0,  1,   7,  7, TV_DENDY,   0, 0 }, // FC BASIC (7-in-1)
	{ 0x41401C6D, 167,  0,  1,   7,  7, TV_DENDY,   0, 0 }, // 小霸王学习机V4
	{ 0x8B265862, 167,  0,  1,   7,  7, TV_DENDY,   0, 0 }, // English Word Blaster
	{ 0x60E33D27, 167,  0,  1,   7,  7, TV_DENDY,   0, 0 }, // 认知码学习
	{ 0xF58761D0, 241,  0,  1,   7,  7, TV_DENDY,   0, 0 }, // Subor 8
	{ 0x0930349E, 241,  0,  1,   7,  7, TV_DENDY,   0, 0 }, // Subor 3
	{ 0xAC11B570, 241,  0,  1,   7,  7, TV_DENDY,   0, 0 }, // 金卡V5
	{ 0xF1DF9942, 241,  0,  1,   7,  7, TV_DENDY,   0, 0 }, // Education Computer 26-in-1 (R)
	{ 0xF834F634, 178,  0,  0,   7,  7, TV_DENDY,   0, 5 }, // Education Computer 32-in-1 (Game Star) (bad dump)
	{ 0x6D378420, 178,  0,  0,   7,  7, TV_DENDY,   0, 0 }, // Education Computer 48-in-1 (Game Star)
	{ 0x7B959CB2, 178,  0,  0,   7,  7, TV_DENDY,   0, 0 }, // 视窗系统2000
	{ 0xF18BC238, 178,  0,  0,   7,  7, TV_DENDY,   0, 0 }, // 视窗系统2000
	{ 0x04EE9E40, 241,  0,  1,   7,  7, TV_DENDY,   0, 0 }, // 14-in-1 Russian Study Cartridge (R)
	{ 0x2CFF0B46, 241,  0,  1,   7,  7, TV_DENDY,   0, 0 }, // 7-in-1 Russian Study Cartridge (R)
	{ 0x6AA52ED1, 241,  0,  1,   7,  7, TV_DENDY,   0, 0 }, // 16-in-1 Chao Ji Shu Biao Jin Ka
	{ 0xC0A76428, 241,  0,  1,   7,  7, TV_DENDY,   0, 0 }, // Arabic Study Cartridge (32-in-1)
	{ 0x6DE05A33, 241,  0,  1,   7,  7, TV_DENDY,   0, 0 }, // Arabic Study Cartridge (32-in-1)
	{ 0xE90A6AEB, 241,  0,  1,   7,  7, TV_DENDY,   0, 0 }, // Jin Ka Wang
	{ 0xF71CBC0A, 241,  0,  1,   7,  7, TV_DENDY,   0, 0 }, // Jing Dian Shu Biao You Xi
	{ 0x368C19A8, 241,  0,  0,   7,  7, TV_DENDY,   0, 0 }, // LIKO Study Cartridge
	{ 0xE4460DF2, 241,  0,  1,   7,  7, TV_DENDY,   0, 0 }, // 小霸王学习机V1.1
	{ 0x85068811, 241,  0,  1,   7,  7, TV_DENDY,   0, 0 }, // 小霸王学习机V6
	{ 0x36A8FDF6, 241,  0,  1,   7,  7, TV_DENDY,   0, 0 }, // 征服太阳系&小管家
	{ 0xD3906F32, 241,  0,  1,   7,  7, TV_DENDY,   0, 0 }, // 数学教授系统 - 小学三年级
	{ 0x669A32F2, 241,  0,  1,   7,  7, TV_DENDY,   0, 0 }, // 马步迷阵&趣味成语屋
	{ 0xF9BC83B9, 241,  0,  1,   7,  7, TV_DENDY,   0, 0 }, // 仓库世家&动脑筋
	{ 0x0A9329B5, 241,  0,  1,   7,  7, TV_DENDY,   0, 0 }, // 启智屋A
	{ 0x15058291, 241,  0,  1,   7,  7, TV_DENDY,   0, 0 }, // 启智屋B
	{ 0xD5D6EAC4, 241,  0,  1,   7,  7, TV_DENDY,   0, 0 }, // MXMDHTWO学习卡
	{ 0x51C41A5A, 241,  0,  1,   7,  7, TV_DENDY,   0, 0 }, // ABM Study Card v5.0
	{ 0x0A9808AE, 514,  0,  0,   7,  7, TV_DENDY,   0, 0 }, // Subor Karaoke
	{ 0x12D61CE8, 518,  0,  0,   7,  7, TV_DENDY,   0, 0 }, // 小霸王学习机V11
	{ 0x5F693117, 518,  0,  0,   7,  7, TV_DENDY,   0, 0 }, // 小霸王学习机V13
	{ 0x40A4C574, 518,  0,  0,   7,  7, TV_DENDY,   0, 0 }, // 小霸王学习机V5
	// FDS Conversions
	{ 0x25E0B714,  43,  0,  1,   0,  0,  TV_NTSC,   0, 0 }, // Super Mario Brothers 2 (Whirlwind Manu LF36)
	// PAL Games that need it
	{ 0xA4BDCC1D,   1,  0,  2,0x70,  7,   TV_PAL,   0, 0 }, // Elite
	{ 0xBC7485B5,   1,  0,  2,0x70,  7,   TV_PAL,   0, 0 }, // Elite
	{ 0x6A741EC6,   1,  0,  0,   0,  0,   TV_PAL,   0, 0 }, // The Addams Family: Pugsley's Scavenger Hunt (PAL Prototype)
	{ 0x4FD3C549,   1,  0,  0,   0,  0,   TV_PAL,   0, 0 }, // The Addams Family: Pugsley's Scavenger Hunt (PAL)
	{ 0x5E137C5B,   0,  0,  0,   0,  0,   TV_PAL,   0, 0 }, // Balloon Fight
	{ 0x3299971A,   0,  0,  0,   0,  0,   TV_PAL,   0, 0 }, // Balloon Fight
	{ 0xB42FEEB4,   4,  0,  0,   0,  0,   TV_PAL,   0, 0 }, // Beauty and the Beast
	{ 0x41D32FD7,   7,  0,  0,   0,  7,   TV_PAL,   0, 0 }, // Aladdin
	{ 0x89984244,   7,  0,  0,   0,  7,   TV_PAL,   0, 0 }, // The Lion King
	{ 0xED77B453,   2,  0,  1,   0,  7,   TV_PAL,   0, 0 }, // Astérix
	{ 0xC42F29B7,   2,  0,  1,   0,  7,   TV_PAL,   0, 0 }, // Astérix [Russian Translation]
	{ 0x32ABB1F1,   0,  0,  1,   0,  0,   TV_PAL,   0, 0 }, // Magic Mathematic
	{ 0x23212D9E,   0,  0,  1,   0,  0,   TV_PAL,   0, 0 }, // Magic Mathematic [Russian Translation]
	{ 0xFBFC6A6C,   1,  0,  0,   0,  0,   TV_PAL,   0, 0 }, // The Adventures of Bayou Billy
	{ 0xE4C1A245,   1,  0,  0,   0,  0,   TV_PAL,   0, 0 }, // The Adventures of Rad Gravity
	{ 0x75B9C0DB,   1,  0,  0,   7,  0,   TV_PAL,   0, 0 }, // Turbo Racing
	{ 0xDB9C072D,   7,  2,  0,   0,  7,   TV_PAL,   0, 0 }, // Arch Rivals
	{ 0x5E74AAFC,  69,  0,  0,   7,  0,   TV_PAL,   0, 0 }, // Batman: Return of the Joker
	{ 0x0F20210D, 241,  0,  1,   0,  7, TV_DENDY,   0, 0 }, // Journey to the West
	{ 0x3304153A, 241,  0,  1,   0,  7, TV_DENDY,   0, 0 }, // Journey to the West
	// Template
	{ 0xFFFFFFFF,   0,  0,  0,   0,  0,  TV_NTSC,   0, 0 }, //
	{ 0x00000000 } // EOL
};

void	SwapBytes(unsigned char *a, unsigned char *b) {
	unsigned char temp =*a;
	*a =*b;
	*b =temp;
}

void	CorrectHeader(FILE *in) {
	if (!Settings::AutoCorrect) return;

	RI.PRGROMCRC32 =Hash::CRC32C(0, PRG_ROM, RI.INES_PRGSize * 0x4000);
	RI.CHRROMCRC32 =Hash::CRC32C(0, CHR_ROM, RI.INES_CHRSize * 0x2000);
	RI.OverallCRC32 =Hash::CRC32C(RI.PRGROMCRC32, CHR_ROM, RI.INES_CHRSize * 0x2000);

	if (RI.OverallCRC32 ==RI.PRGROMCRC32 && RI.PRGROMCRC32 ==0xD1D5B706) { // Good ROM with incorrect number of PRG banks specified in the NES header!
		RI.INES_PRGSize = 1024*1024 /0x4000;
		fseek(in, 0x10, SEEK_SET);
		fread(PRG_ROM, 1, RI.INES_PRGSize * 0x4000, in);
		PRGSizeROM = RI.INES_PRGSize * 0x4;

		RI.PRGROMCRC32 =Hash::CRC32C(0, PRG_ROM, RI.INES_PRGSize * 0x4000);
		RI.OverallCRC32 =Hash::CRC32C(RI.PRGROMCRC32, CHR_ROM, RI.INES_CHRSize * 0x2000);
	}

	struct CorrectMapperInfo *info =NULL;
	for (info =&CorrectMapperInfos[0]; info->PRGCRC !=0; info++)
		if (info->PRGCRC ==RI.PRGROMCRC32 && (info->AllCRC ==RI.OverallCRC32 || info->AllCRC ==0)) break;
	if (info->PRGCRC ==0) info =NULL;

	if (info ==NULL) {
		if (RI.INES_Version <2 && RI.INES_MapperNum ==16 && RI.INES_CHRSize ==0) {
			if (RI.INES_PRGSize >=512/16)
				RI.INES_MapperNum =153;
			else
				RI.INES_MapperNum =157;
			EI.DbgOut(_T("Auto Correct: Mapper Number changed to %i"), RI.INES_MapperNum);
		}
		//if (RI.INES_MapperNum ==4 && RI.INES_PRGSize ==1024*1024 /16384) RI.INES_MapperNum =224;
		return;
	}

	unsigned char *cPRG_ROM =(unsigned char *) &PRG_ROM, *cCHR_ROM =(unsigned char *) &CHR_ROM; // Contiguous PRG-/CHR-ROM space without that 4K/1K separated bullshit
	switch(info->Special) {
		case -1:MessageBox(hMainWnd, _T("This ROM image is a known bad or incomplete dump."), _T("Nintendulator"), MB_OK | MB_ICONWARNING);
			break;
		case 1:	// 2-in-1 (Mortal Kombat 6, Samurai Spirits): Reverse 128 KiB PRG and 256 KiB CHR banks
			for (int i =0; i <0x20000; i++) SwapBytes(&cPRG_ROM[i +0], &cPRG_ROM[i +0x20000]);
			for (int i =0; i <0x40000; i++) SwapBytes(&cCHR_ROM[i +0], &cCHR_ROM[i +0x40000]);
			break;
		case 2:	// 1998 Super HIK 5-in-1: Reverse 512 KiB CHR banks
			for (int i =0; i <0x80000; i++) SwapBytes(&cCHR_ROM[i +0], &cCHR_ROM[i +0x80000]);
			break;
		case 3:	// Unused: Reverse 128 KiB PRG and 128 KiB CHR banks
			for (int i =0; i <0x20000; i++) SwapBytes(&cPRG_ROM[i +0], &cPRG_ROM[i +0x20000]);
			for (int i =0; i <0x20000; i++) SwapBytes(&cCHR_ROM[i +0], &cCHR_ROM[i +0x20000]);
			break;
		case 4:	{// Zhanguo Sichuansheng 132->173
			unsigned char *oldCHR =(unsigned char *) malloc(32768);
			memcpy(oldCHR, cCHR_ROM, 32768);
			int rightOrder [4] = {2, 1, 3, 0};
			for (int i =0; i <4; i++)
				memcpy(cCHR_ROM +i*0x2000, oldCHR +rightOrder[i]*0x2000, 0x2000);
			free(oldCHR);
			break; }
		case 5: { // Fix messed up banks of Pokemon Emerald (Coolboy)
			unsigned char *oldPRG =(unsigned char *) malloc(1048576);
			memcpy(oldPRG, cPRG_ROM, 1048576);
			for (int i =0; i <64; i+=2) {
				int val4801 =i &7;
				int val4802 =i >>3;
				int bankOld = (val4802 <<1) | (val4801 <<3);
				int bankNew = val4801 | (val4802 <<3);
				bankOld &=63;
				bankNew &=63;
				bankOld >>=1;
				bankNew >>=1;
				memcpy(cPRG_ROM +bankNew*32768, oldPRG +bankOld*32768, 32768);
			}
			#if 0
			FILE* F =fopen("FIX5.BIN", "wb");
			fwrite(cPRG_ROM, 1, 1048576, F);
			fclose (F);
			#endif
			free(oldPRG);
			break; }
		case 6: // FFE Hack of Kaiser Hack of Nintendo's Mahjong
			memcpy(cCHR_ROM, cPRG_ROM +0x18000, 0x8000);
			RI.INES_PRGSize =32768/16384;
			RI.INES_CHRSize =32768/8192;
			PRGSizeROM = RI.INES_PRGSize * 0x4;
			CHRSizeROM = RI.INES_CHRSize * 0x8;
			break;
		case 7: // FFE Matendoji
			memcpy(cCHR_ROM, cPRG_ROM +0x20000, 0x20000);
			RI.INES_PRGSize =128*1024/16384;
			RI.INES_CHRSize =128*1024/8192;
			PRGSizeROM = RI.INES_PRGSize * 0x4;
			CHRSizeROM = RI.INES_CHRSize * 0x8;
			break;
		case 8: // FFE Sky Shark
			memcpy(cCHR_ROM, cPRG_ROM +0x10000, 0x20000);
			RI.INES_PRGSize =64*1024/16384;
			RI.INES_CHRSize =128*1024/8192;
			PRGSizeROM = RI.INES_PRGSize * 0x4;
			CHRSizeROM = RI.INES_CHRSize * 0x8;
			break;
		case 9: // FFE Pyramid
			memcpy(cCHR_ROM, cPRG_ROM +0x8000, 0x10000);
			RI.INES_PRGSize =32768/16384;
			RI.INES_CHRSize =65536/8192;
			PRGSizeROM = RI.INES_PRGSize * 0x4;
			CHRSizeROM = RI.INES_CHRSize * 0x8;
			break;
		case 10: // FFE Jovial Race
			memcpy(cPRG_ROM+0x10000, cCHR_ROM +0, 0x10000);
			RI.INES_PRGSize =128*1024/16384;
			RI.INES_CHRSize =0/8192;
			PRGSizeROM = RI.INES_PRGSize * 0x4;
			CHRSizeROM = RI.INES_CHRSize * 0x8;
			break;
		case 11: // FFE Dragon Ball 2
			memcpy(cPRG_ROM+0x20000, cCHR_ROM +0, 0x20000);
			RI.INES_PRGSize =256*1024/16384;
			RI.INES_CHRSize =0/8192;
			PRGSizeROM = RI.INES_PRGSize * 0x4;
			CHRSizeROM = RI.INES_CHRSize * 0x8;
			break;
		case 12: // FFE Ganbare Goemon Gaiden II
			memcpy(cPRG_ROM+0x40000, cCHR_ROM +0, 0x40000);
			RI.INES_PRGSize =512*1024/16384;
			RI.INES_CHRSize =0/8192;
			PRGSizeROM = RI.INES_PRGSize * 0x4;
			CHRSizeROM = RI.INES_CHRSize * 0x8;
			break;
		case 13:{// Prima Soft 9999999-in-1
			unsigned char *oldPRG =(unsigned char *) malloc(128 *1024);
			memcpy(oldPRG, cPRG_ROM, 128 *1024);
			int rightOrder [8] = {0, 4, 1, 5, 2, 6, 3, 7};
			//int rightOrder [8] = {0, 2, 4, 6, 1, 3, 5, 7};
			for (int i =0; i <8; i++)
				memcpy(cPRG_ROM +i*0x4000, oldPRG +rightOrder[i]*0x4000, 0x4000);
			free(oldPRG);
			#if 0
			FILE* F =fopen("FIX13.BIN", "wb");
			fwrite(cPRG_ROM, 1, 128 *1024, F);
			fclose (F);
			#endif
			break; }
		case 14:{// Kuai Da Jin Ka Zhong Ji Tiao Zhan 3-in-1, 6-in-1
			unsigned char *oldPRG =(unsigned char *) malloc(512 *1024);
			memcpy(oldPRG, cPRG_ROM, 512 *1024);
			int rightOrder [8] = {0, 3, 1, 2};
			for (int i =0; i <8; i++)
				memcpy(cPRG_ROM +i*0x20000, oldPRG +rightOrder[i]*0x20000, 0x20000);
			free(oldPRG);
			#if 0
			FILE* F =fopen("FIX14.BIN", "wb");
			fwrite(cPRG_ROM, 1, 512 *1024, F);
			fclose (F);
			#endif
			break; }
		case 15:{// Meinuquan mapper 150
			unsigned char *oldCHR =(unsigned char *) malloc(128*1024);
			memcpy(oldCHR, cCHR_ROM, 128*1024);
			int bitOrder[4] ={ 2, 3, 1, 0 };
			for (int i =0; i <16; i++) {
				int correct = (i &1? 1: 0) <<bitOrder[0] |
			                      (i &2? 1: 0) <<bitOrder[1] |
					      (i &4? 1: 0) <<bitOrder[2] |
					      (i &8? 1: 0) <<bitOrder[3];
				memcpy(cCHR_ROM +correct*0x2000, oldCHR +i*0x2000, 0x2000);
			}
			free(oldCHR);
			RI.INES_MapperNum =243;
			#if 0
			FILE* F =fopen("FIX243.BIN", "wb");
			fwrite(cCHR_ROM, 1, 128 *1024, F);
			fclose (F);
			#endif
			break; }
		case 16:{// Meinuquan mapper 243 old
			unsigned char *oldCHR =(unsigned char *) malloc(128*1024);
			memcpy(oldCHR, cCHR_ROM, 128*1024);
			int bitOrder[4] ={ 1, 2, 3, 0 };
			for (int i =0; i <16; i++) {
				int correct = (i &1? 1: 0) <<bitOrder[0] |
			                      (i &2? 1: 0) <<bitOrder[1] |
					      (i &4? 1: 0) <<bitOrder[2] |
					      (i &8? 1: 0) <<bitOrder[3];
				memcpy(cCHR_ROM +correct*0x2000, oldCHR +i*0x2000, 0x2000);
			}
			free(oldCHR);
			RI.INES_MapperNum =243;
			#if 0
			FILE* F =fopen("FIX243.BIN", "wb");
			fwrite(cCHR_ROM, 1, 128 *1024, F);
			fclose (F);
			#endif
			break; }
		case 17:{ // Shenzhen Nanjing mode 7
			unsigned char *oldPRG =(unsigned char *) malloc(2048*1024);
			memcpy(oldPRG, cPRG_ROM, RI.PRGROMSize);
			for (unsigned int i =0; i <RI.PRGROMSize; i++) {
				unsigned int j;
				if (RI.PRGROMSize >=2048*1024)
					j =(i&~0x198000 |
					i >>1 &0x008000 | // A16->A15
					i <<1 &0x010000 | // A15->A16
					i >>1 &0x080000 | // A20->A19
					i <<1 &0x100000); // A19->A20
				else
					j =(i&~0x018000 |
					i >>1 &0x008000 | // A16->A15
					i <<1 &0x010000); // A15->A16
				cPRG_ROM[i] =oldPRG[j &(RI.PRGROMSize -1)];
			}				
			free(oldPRG);
			break;
			}
	}
	if (info->Special >0) EI.DbgOut(_T("Auto Correct: Reordered ROM banks, procedure #%i"), info->Special);

	RI.INES_Version =2;
	RI.INES2_PRGRAM = (RI.INES_Flags &F_BATTERY)? 0xA0: 0x0A; // 64 KiB
	RI.INES2_CHRRAM = 0x09; // 32 KIB
	if (RI.INES_MapperNum !=info->MapperNum) EI.DbgOut(_T("Auto Correct: Mapper Number changed to %i"), info->MapperNum);
	if (RI.INES2_SubMapper !=info->SubMapper) EI.DbgOut(_T("Auto Correct: Submapper set to %i"), info->SubMapper);
	RI.INES_MapperNum =info->MapperNum;
	RI.INES2_SubMapper =info->SubMapper;
	RI.INES_Flags =info->Flags;
	RI.INES2_PRGRAM =info->PRG_RAM;
	RI.INES2_CHRRAM =info->CHR_RAM;
	RI.INES2_TVMode =info->TVMode;
	RI.INES2_VSPalette =info->VSData &0x0F;
	RI.INES2_VSFlags =info->VSData >>4;
	if (info->MapperNum ==256) RI.ConsoleType =CONSOLE_VT03;
}