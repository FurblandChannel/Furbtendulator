/* Nintendulator Mapper DLLs
 * Copyright (C) 2002-2011 QMT Productions
 *
 * $URL: https://nintendulator.svn.sourceforge.net/svnroot/nintendulator/mappers/trunk/src/Dll/d_iNES.cpp $
 * $Id: d_iNES.cpp 1227 2011-08-27 01:29:07Z quietust $
 */

#include	"d_iNES.h"
#include	<stdio.h>

void	iNES_SetMirroring (void) {
	switch(ROM->INES_Flags &9) {
		case 0: EMU->Mirror_H(); break;
		case 1: EMU->Mirror_V(); break;
		default:EMU->Mirror_4(); break;
	}
}

void	iNES_SetSRAM (void)
{
	if (ROM->INES_Flags & 0x02) {
		if (ROM->INES_Version == 2)
		{
			int size = ((ROM->INES2_PRGRAM & 0xF0) >> 4) + ((ROM->INES2_CHRRAM & 0xF0) >> 4);
			if (size ==0)
				EMU->Set_SRAMSize(0);
			else
				EMU->Set_SRAMSize(64 << size);
		}
		else	EMU->Set_SRAMSize(8192);
	}
}

// Shortcuts for automatically mapping CHR RAM if CHR ROM is unavailable
void	iNES_SetCHR_Auto1 (int Bank, int Val)
{
	if (ROM->INES_CHRSize)
		EMU->SetCHR_ROM1(Bank, Val);
	else
	{
		// only NES 2.0 will allow more than 8KB of CHR RAM. NRS: Bullshit.
		/*if (ROM->INES_Version == 1)
			Val &= 0x7;*/
		EMU->SetCHR_RAM1(Bank, Val);
	}
}

void	iNES_SetCHR_Auto2 (int Bank, int Val)
{
	Val <<= 1;
	iNES_SetCHR_Auto1(Bank+0, Val+0);
	iNES_SetCHR_Auto1(Bank+1, Val+1);
}

void	iNES_SetCHR_Auto4 (int Bank, int Val)
{
	Val <<= 2;
	iNES_SetCHR_Auto1(Bank+0, Val+0);
	iNES_SetCHR_Auto1(Bank+1, Val+1);
	iNES_SetCHR_Auto1(Bank+2, Val+2);
	iNES_SetCHR_Auto1(Bank+3, Val+3);
}

void	iNES_SetCHR_Auto8 (int Bank, int Val)
{
	Val <<= 3;
	iNES_SetCHR_Auto1(Bank+0, Val+0);
	iNES_SetCHR_Auto1(Bank+1, Val+1);
	iNES_SetCHR_Auto1(Bank+2, Val+2);
	iNES_SetCHR_Auto1(Bank+3, Val+3);
	iNES_SetCHR_Auto1(Bank+4, Val+4);
	iNES_SetCHR_Auto1(Bank+5, Val+5);
	iNES_SetCHR_Auto1(Bank+6, Val+6);
	iNES_SetCHR_Auto1(Bank+7, Val+7);
}

HWND			hWnd;
HINSTANCE		hInstance;
const EmulatorInterface	*EMU;
ROMInfo		*ROM;

namespace 
{
MapperInfo *MapperTable[768] =
{
	&MapperInfo_000,&MapperInfo_001,&MapperInfo_002,&MapperInfo_003,&MapperInfo_004,&MapperInfo_005,&MapperInfo_006,&MapperInfo_007,
	&MapperInfo_008,&MapperInfo_009,&MapperInfo_010,&MapperInfo_011,&MapperInfo_012,&MapperInfo_013,&MapperInfo_014,&MapperInfo_015,
	&MapperInfo_016,&MapperInfo_017,&MapperInfo_018,&MapperInfo_019,&MapperInfo_020,&MapperInfo_021,&MapperInfo_022,&MapperInfo_023,
	&MapperInfo_024,&MapperInfo_025,&MapperInfo_026,&MapperInfo_027,&MapperInfo_028,&MapperInfo_029,&MapperInfo_030,&MapperInfo_031,
	&MapperInfo_032,&MapperInfo_033,&MapperInfo_034,&MapperInfo_035,&MapperInfo_036,&MapperInfo_037,&MapperInfo_038,&MapperInfo_039,
	&MapperInfo_040,&MapperInfo_041,&MapperInfo_042,&MapperInfo_043,&MapperInfo_044,&MapperInfo_045,&MapperInfo_046,&MapperInfo_047,
	&MapperInfo_048,&MapperInfo_049,&MapperInfo_050,&MapperInfo_051,&MapperInfo_052,&MapperInfo_053,&MapperInfo_054,&MapperInfo_055,
	&MapperInfo_056,&MapperInfo_057,&MapperInfo_058,&MapperInfo_059,&MapperInfo_060,&MapperInfo_061,&MapperInfo_062,&MapperInfo_063,
	&MapperInfo_064,&MapperInfo_065,&MapperInfo_066,&MapperInfo_067,&MapperInfo_068,&MapperInfo_069,&MapperInfo_070,&MapperInfo_071,
	&MapperInfo_072,&MapperInfo_073,&MapperInfo_074,&MapperInfo_075,&MapperInfo_076,&MapperInfo_077,&MapperInfo_078,&MapperInfo_079,
	&MapperInfo_080,&MapperInfo_081,&MapperInfo_082,&MapperInfo_083,&MapperInfo_084,&MapperInfo_085,&MapperInfo_086,&MapperInfo_087,
	&MapperInfo_088,&MapperInfo_089,&MapperInfo_090,&MapperInfo_091,&MapperInfo_092,&MapperInfo_093,&MapperInfo_094,&MapperInfo_095,
	&MapperInfo_096,&MapperInfo_097,&MapperInfo_098,&MapperInfo_099,&MapperInfo_100,&MapperInfo_101,&MapperInfo_102,&MapperInfo_103,
	&MapperInfo_104,&MapperInfo_105,&MapperInfo_106,&MapperInfo_107,&MapperInfo_108,&MapperInfo_109,&MapperInfo_110,&MapperInfo_111,
	&MapperInfo_112,&MapperInfo_113,&MapperInfo_114,&MapperInfo_115,&MapperInfo_116,&MapperInfo_117,&MapperInfo_118,&MapperInfo_119,
	&MapperInfo_120,&MapperInfo_121,&MapperInfo_122,&MapperInfo_123,&MapperInfo_124,&MapperInfo_125,&MapperInfo_126,&MapperInfo_127,
	&MapperInfo_128,&MapperInfo_129,&MapperInfo_130,&MapperInfo_131,&MapperInfo_132,&MapperInfo_133,&MapperInfo_134,&MapperInfo_135,
	&MapperInfo_136,&MapperInfo_137,&MapperInfo_138,&MapperInfo_139,&MapperInfo_140,&MapperInfo_141,&MapperInfo_142,&MapperInfo_143,
	&MapperInfo_144,&MapperInfo_145,&MapperInfo_146,&MapperInfo_147,&MapperInfo_148,&MapperInfo_149,&MapperInfo_150,&MapperInfo_151,
	&MapperInfo_152,&MapperInfo_153,&MapperInfo_154,&MapperInfo_155,&MapperInfo_156,&MapperInfo_157,&MapperInfo_158,&MapperInfo_159,
	&MapperInfo_160,&MapperInfo_161,&MapperInfo_162,&MapperInfo_163,&MapperInfo_164,&MapperInfo_165,&MapperInfo_166,&MapperInfo_167,
	&MapperInfo_168,&MapperInfo_169,&MapperInfo_170,&MapperInfo_171,&MapperInfo_172,&MapperInfo_173,&MapperInfo_174,&MapperInfo_175,
	&MapperInfo_176,&MapperInfo_177,&MapperInfo_178,&MapperInfo_179,&MapperInfo_180,&MapperInfo_181,&MapperInfo_182,&MapperInfo_183,
	&MapperInfo_184,&MapperInfo_185,&MapperInfo_186,&MapperInfo_187,&MapperInfo_188,&MapperInfo_189,&MapperInfo_190,&MapperInfo_191,
	&MapperInfo_192,&MapperInfo_193,&MapperInfo_194,&MapperInfo_195,&MapperInfo_196,&MapperInfo_197,&MapperInfo_198,&MapperInfo_199,
	&MapperInfo_200,&MapperInfo_201,&MapperInfo_202,&MapperInfo_203,&MapperInfo_204,&MapperInfo_205,&MapperInfo_206,&MapperInfo_207,
	&MapperInfo_208,&MapperInfo_209,&MapperInfo_210,&MapperInfo_211,&MapperInfo_212,&MapperInfo_213,&MapperInfo_214,&MapperInfo_215,
	&MapperInfo_216,&MapperInfo_217,&MapperInfo_218,&MapperInfo_219,&MapperInfo_220,&MapperInfo_221,&MapperInfo_222,&MapperInfo_223,
	&MapperInfo_224,&MapperInfo_225,&MapperInfo_226,&MapperInfo_227,&MapperInfo_228,&MapperInfo_229,&MapperInfo_230,&MapperInfo_231,
	&MapperInfo_232,&MapperInfo_233,&MapperInfo_234,&MapperInfo_235,&MapperInfo_236,&MapperInfo_237,&MapperInfo_238,&MapperInfo_239,
	&MapperInfo_240,&MapperInfo_241,&MapperInfo_242,&MapperInfo_243,&MapperInfo_244,&MapperInfo_245,&MapperInfo_246,&MapperInfo_247,
	&MapperInfo_248,&MapperInfo_249,&MapperInfo_250,&MapperInfo_251,&MapperInfo_252,&MapperInfo_253,&MapperInfo_254,&MapperInfo_255,
	&MapperInfo_256,&MapperInfo_257,&MapperInfo_258,&MapperInfo_259,&MapperInfo_260,&MapperInfo_261,&MapperInfo_262,&MapperInfo_263,
	&MapperInfo_264,&MapperInfo_265,&MapperInfo_266,&MapperInfo_267,&MapperInfo_268,&MapperInfo_269,&MapperInfo_270,&MapperInfo_271,
	&MapperInfo_272,&MapperInfo_273,&MapperInfo_274,&MapperInfo_275,&MapperInfo_276,&MapperInfo_277,&MapperInfo_278,&MapperInfo_279,
	&MapperInfo_280,&MapperInfo_281,&MapperInfo_282,&MapperInfo_283,&MapperInfo_284,&MapperInfo_285,&MapperInfo_286,&MapperInfo_287,
	&MapperInfo_288,&MapperInfo_289,&MapperInfo_290,&MapperInfo_291,&MapperInfo_292,&MapperInfo_293,&MapperInfo_294,&MapperInfo_295,
	&MapperInfo_296,&MapperInfo_297,&MapperInfo_298,&MapperInfo_299,&MapperInfo_300,&MapperInfo_301,&MapperInfo_302,&MapperInfo_303,
	&MapperInfo_304,&MapperInfo_305,&MapperInfo_306,&MapperInfo_307,&MapperInfo_308,&MapperInfo_309,&MapperInfo_310,&MapperInfo_311,
	&MapperInfo_312,&MapperInfo_313,&MapperInfo_314,&MapperInfo_315,&MapperInfo_316,&MapperInfo_317,&MapperInfo_318,&MapperInfo_319,
	&MapperInfo_320,&MapperInfo_321,&MapperInfo_322,&MapperInfo_323,&MapperInfo_324,&MapperInfo_325,&MapperInfo_326,&MapperInfo_327,
	&MapperInfo_328,&MapperInfo_329,&MapperInfo_330,&MapperInfo_331,&MapperInfo_332,&MapperInfo_333,&MapperInfo_334,&MapperInfo_335,
	&MapperInfo_336,&MapperInfo_337,&MapperInfo_338,&MapperInfo_339,&MapperInfo_340,&MapperInfo_341,&MapperInfo_342,&MapperInfo_343,
	&MapperInfo_344,&MapperInfo_345,&MapperInfo_346,&MapperInfo_347,&MapperInfo_348,&MapperInfo_349,&MapperInfo_350,&MapperInfo_351,
	&MapperInfo_352,&MapperInfo_353,&MapperInfo_354,&MapperInfo_355,&MapperInfo_356,&MapperInfo_357,&MapperInfo_358,&MapperInfo_359,
	&MapperInfo_360,&MapperInfo_361,&MapperInfo_362,&MapperInfo_363,&MapperInfo_364,&MapperInfo_365,&MapperInfo_366,&MapperInfo_367,
	&MapperInfo_368,&MapperInfo_369,&MapperInfo_370,&MapperInfo_371,&MapperInfo_372,&MapperInfo_373,&MapperInfo_374,&MapperInfo_375,
	&MapperInfo_376,&MapperInfo_377,&MapperInfo_378,&MapperInfo_379,&MapperInfo_380,&MapperInfo_381,&MapperInfo_382,&MapperInfo_383,
	&MapperInfo_384,&MapperInfo_385,&MapperInfo_386,&MapperInfo_387,&MapperInfo_388,&MapperInfo_389,&MapperInfo_390,&MapperInfo_391,
	&MapperInfo_392,&MapperInfo_393,&MapperInfo_394,&MapperInfo_395,&MapperInfo_396,&MapperInfo_397,&MapperInfo_398,&MapperInfo_399,
	&MapperInfo_400,&MapperInfo_401,&MapperInfo_402,&MapperInfo_403,&MapperInfo_404,&MapperInfo_405,&MapperInfo_406,&MapperInfo_407,
	&MapperInfo_408,&MapperInfo_409,&MapperInfo_410,&MapperInfo_411,&MapperInfo_412,&MapperInfo_413,&MapperInfo_414,&MapperInfo_415,
	&MapperInfo_416,&MapperInfo_417,&MapperInfo_418,&MapperInfo_419,&MapperInfo_420,&MapperInfo_421,&MapperInfo_422,&MapperInfo_423,
	&MapperInfo_424,&MapperInfo_425,&MapperInfo_426,&MapperInfo_427,&MapperInfo_428,&MapperInfo_429,&MapperInfo_430,&MapperInfo_431,
	&MapperInfo_432,&MapperInfo_433,&MapperInfo_434,&MapperInfo_435,&MapperInfo_436,&MapperInfo_437,&MapperInfo_438,&MapperInfo_439,
	&MapperInfo_440,&MapperInfo_441,&MapperInfo_442,&MapperInfo_443,&MapperInfo_444,&MapperInfo_445,&MapperInfo_446,&MapperInfo_447,
	&MapperInfo_448,&MapperInfo_449,&MapperInfo_450,&MapperInfo_451,&MapperInfo_452,&MapperInfo_453,&MapperInfo_454,&MapperInfo_455,
	&MapperInfo_456,&MapperInfo_457,&MapperInfo_458,&MapperInfo_459,&MapperInfo_460,&MapperInfo_461,&MapperInfo_462,&MapperInfo_463,
	&MapperInfo_464,&MapperInfo_465,&MapperInfo_466,&MapperInfo_467,&MapperInfo_468,&MapperInfo_469,&MapperInfo_470,&MapperInfo_471,
	&MapperInfo_472,&MapperInfo_473,&MapperInfo_474,&MapperInfo_475,&MapperInfo_476,&MapperInfo_477,&MapperInfo_478,&MapperInfo_479,
	&MapperInfo_480,&MapperInfo_481,&MapperInfo_482,&MapperInfo_483,&MapperInfo_484,&MapperInfo_485,&MapperInfo_486,&MapperInfo_487,
	&MapperInfo_488,&MapperInfo_489,&MapperInfo_490,&MapperInfo_491,&MapperInfo_492,&MapperInfo_493,&MapperInfo_494,&MapperInfo_495,
	&MapperInfo_496,&MapperInfo_497,&MapperInfo_498,&MapperInfo_499,&MapperInfo_500,&MapperInfo_501,&MapperInfo_502,&MapperInfo_503,
	&MapperInfo_504,&MapperInfo_505,&MapperInfo_506,&MapperInfo_507,&MapperInfo_508,&MapperInfo_509,&MapperInfo_510,&MapperInfo_511,
	&MapperInfo_512,&MapperInfo_513,&MapperInfo_514,&MapperInfo_515,&MapperInfo_516,&MapperInfo_517,&MapperInfo_518,&MapperInfo_519,
	&MapperInfo_520,&MapperInfo_521,&MapperInfo_522,&MapperInfo_523,&MapperInfo_524,&MapperInfo_525,&MapperInfo_526,&MapperInfo_527,
	&MapperInfo_528,&MapperInfo_529,&MapperInfo_530,&MapperInfo_531,&MapperInfo_532,&MapperInfo_533,&MapperInfo_534,&MapperInfo_535,
	&MapperInfo_536,&MapperInfo_537,&MapperInfo_538,&MapperInfo_539,&MapperInfo_540,&MapperInfo_541,&MapperInfo_542,&MapperInfo_543,
	&MapperInfo_544,&MapperInfo_545,&MapperInfo_546,&MapperInfo_547,&MapperInfo_548,&MapperInfo_549,&MapperInfo_550,&MapperInfo_551,
	&MapperInfo_552,&MapperInfo_553,&MapperInfo_554,&MapperInfo_555,&MapperInfo_556,&MapperInfo_557,&MapperInfo_558,&MapperInfo_559,
	&MapperInfo_560,&MapperInfo_561,&MapperInfo_562,&MapperInfo_563,&MapperInfo_564,&MapperInfo_565,&MapperInfo_566,&MapperInfo_567,
	&MapperInfo_568,&MapperInfo_569,&MapperInfo_570,&MapperInfo_571,&MapperInfo_572,&MapperInfo_573,&MapperInfo_574,&MapperInfo_575,
	&MapperInfo_576,&MapperInfo_577,&MapperInfo_578,&MapperInfo_579,&MapperInfo_580,&MapperInfo_581,&MapperInfo_582,&MapperInfo_583,
	&MapperInfo_584,&MapperInfo_585,&MapperInfo_586,&MapperInfo_587,&MapperInfo_588,&MapperInfo_589,&MapperInfo_590,&MapperInfo_591,
	&MapperInfo_592,&MapperInfo_593,&MapperInfo_594,&MapperInfo_595,&MapperInfo_596,&MapperInfo_597,&MapperInfo_598,&MapperInfo_599,
	&MapperInfo_600,&MapperInfo_601,&MapperInfo_602,&MapperInfo_603,&MapperInfo_604,&MapperInfo_605,&MapperInfo_606,&MapperInfo_607,
	&MapperInfo_608,&MapperInfo_609,&MapperInfo_610,&MapperInfo_611,&MapperInfo_612,&MapperInfo_613,&MapperInfo_614,&MapperInfo_615,
	&MapperInfo_616,&MapperInfo_617,&MapperInfo_618,&MapperInfo_619,&MapperInfo_620,&MapperInfo_621,&MapperInfo_622,&MapperInfo_623,
	&MapperInfo_624,&MapperInfo_625,&MapperInfo_626,&MapperInfo_627,&MapperInfo_628,&MapperInfo_629,&MapperInfo_630,&MapperInfo_631,
	&MapperInfo_632,&MapperInfo_633,&MapperInfo_634,&MapperInfo_635,&MapperInfo_636,&MapperInfo_637,&MapperInfo_638,&MapperInfo_639,
	&MapperInfo_640,&MapperInfo_641,&MapperInfo_642,&MapperInfo_643,&MapperInfo_644,&MapperInfo_645,&MapperInfo_646,&MapperInfo_647,
	&MapperInfo_648,&MapperInfo_649,&MapperInfo_650,&MapperInfo_651,&MapperInfo_652,&MapperInfo_653,&MapperInfo_654,&MapperInfo_655,
	&MapperInfo_656,&MapperInfo_657,&MapperInfo_658,&MapperInfo_659,&MapperInfo_660,&MapperInfo_661,&MapperInfo_662,&MapperInfo_663,
	&MapperInfo_664,&MapperInfo_665,&MapperInfo_666,&MapperInfo_667,&MapperInfo_668,&MapperInfo_669,&MapperInfo_670,&MapperInfo_671,
	&MapperInfo_672,&MapperInfo_673,&MapperInfo_674,&MapperInfo_675,&MapperInfo_676,&MapperInfo_677,&MapperInfo_678,&MapperInfo_679,
	&MapperInfo_680,&MapperInfo_681,&MapperInfo_682,&MapperInfo_683,&MapperInfo_684,&MapperInfo_685,&MapperInfo_686,&MapperInfo_687,
	&MapperInfo_688,&MapperInfo_689,&MapperInfo_690,&MapperInfo_691,&MapperInfo_692,&MapperInfo_693,&MapperInfo_694,&MapperInfo_695,
	&MapperInfo_696,&MapperInfo_697,&MapperInfo_698,&MapperInfo_699,&MapperInfo_700,&MapperInfo_701,&MapperInfo_702,&MapperInfo_703,
	&MapperInfo_704,&MapperInfo_705,&MapperInfo_706,&MapperInfo_707,&MapperInfo_708,&MapperInfo_709,&MapperInfo_710,&MapperInfo_711,
	&MapperInfo_712,&MapperInfo_713,&MapperInfo_714,&MapperInfo_715,&MapperInfo_716,&MapperInfo_717,&MapperInfo_718,&MapperInfo_719,
	&MapperInfo_720,&MapperInfo_721,&MapperInfo_722,&MapperInfo_723,&MapperInfo_724,&MapperInfo_725,&MapperInfo_726,&MapperInfo_727,
	&MapperInfo_728,&MapperInfo_729,&MapperInfo_730,&MapperInfo_731,&MapperInfo_732,&MapperInfo_733,&MapperInfo_734,&MapperInfo_735,
	&MapperInfo_736,&MapperInfo_737,&MapperInfo_738,&MapperInfo_739,&MapperInfo_740,&MapperInfo_741,&MapperInfo_742,&MapperInfo_743,
	&MapperInfo_744,&MapperInfo_745,&MapperInfo_746,&MapperInfo_747,&MapperInfo_748,&MapperInfo_749,&MapperInfo_750,&MapperInfo_751,
	&MapperInfo_752,&MapperInfo_753,&MapperInfo_754,&MapperInfo_755,&MapperInfo_756,&MapperInfo_757,&MapperInfo_758,&MapperInfo_759,
	&MapperInfo_760,&MapperInfo_761,&MapperInfo_762,&MapperInfo_763,&MapperInfo_764,&MapperInfo_765,&MapperInfo_766,&MapperInfo_767
};

void	MAPINT	UnloadMapper (void) {
	ROM = NULL;
}

MapperInfo *MAPINT	LoadMapper (ROMInfo *_ROM)
{
	ROM = _ROM;
	if (ROM->ROMType == ROM_UNDEFINED) {	/* Allow enumerating mappers */
		unsigned int i = (unsigned int)ROM->Filename;
		if (i >= 256)
		{
			UnloadMapper();
			return NULL;
		}
		((ROMInfo *)ROM)->ROMType = ROM_INES;
		return MapperTable[i];
	}
	if (ROM->ROMType != ROM_INES) {	/* Only support iNES ROM images*/
		UnloadMapper();
		return NULL;
	}
	if (ROM->ConsoleType ==CONSOLE_VS) {	/* Don't support VS Unisystem ROMs */
		UnloadMapper();
		return NULL;
	}
	if (ROM->INES_MapperNum > 767) {
		UnloadMapper();
		return NULL;
	}
	switch(ROM->INES_MapperNum) {
		case 4:	 // Redirect mapper 4 files with trainer to Nesticle MMC3 emulation (mapper 100)
			return ROM->INES_Flags &4? &MapperInfo_100: MapperTable[ROM->INES_MapperNum];
		case 8:  // FFE mode 4
			return &MapperInfo_006;			
	}
	if (MapperTable[ROM->INES_MapperNum]->Compatibility == COMPAT_NONE) {	/* Don't accept mappers listed with zero compatibility */
		UnloadMapper();
		return NULL;
	}
	return MapperTable[ROM->INES_MapperNum];
}

DLLInfo	DLL_Info =
{
	_T("INES.DLL by Quietust"),
	0x20100102,
	0x00040002,
	LoadMapper,
	UnloadMapper
};
} // namespace

extern "C" __declspec(dllexport)	void	MAPINT	UnloadMapperDLL (void)
{
	EMU = NULL;
	hWnd = NULL;
}

extern "C" __declspec(dllexport)	DLLInfo	*MAPINT	LoadMapperDLL (HWND hWndEmu, const EmulatorInterface *_EMU, int VersionRequired)
{
	hWnd = hWndEmu;
	EMU = _EMU;
	if (VersionRequired != CurrentMapperInterface)
	{
		UnloadMapperDLL();
		return NULL;
	}
	return &DLL_Info;
}

BOOL	WINAPI	DllMain (HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInstance = hinstDLL;
	return TRUE;
}
