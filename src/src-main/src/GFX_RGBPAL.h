const unsigned short PaletteROM_2C03 [64] = {
	0333,0014,0006,0326,0403,0503,0510,0420,0320,0120,0031,0040,0022,0000,0000,0000,
	0555,0036,0027,0407,0507,0704,0700,0630,0430,0140,0040,0053,0044,0000,0000,0000,
	0777,0357,0447,0637,0707,0737,0740,0750,0660,0360,0070,0276,0077,0000,0000,0000,
	0777,0567,0657,0757,0747,0755,0764,0772,0773,0572,0473,0276,0467,0000,0000,0000
};
const unsigned short PaletteROM_RC2C03B [64] = {
	0333,0014,0006,0326,0403,0503,0510,0420,0320,0100,0031,0040,0022,0000,0000,0000,
	0555,0016,0027,0407,0507,0704,0700,0630,0430,0140,0040,0053,0044,0000,0000,0000,
	0777,0357,0447,0637,0707,0717,0740,0750,0660,0340,0070,0276,0077,0000,0000,0000,
	0777,0547,0657,0757,0747,0755,0764,0772,0773,0552,0473,0276,0467,0000,0000,0000
};
const unsigned short PaletteROM_2C04 [64] = {
//	 x0 , x1 , x2 , x3 , x4 , x5 , x6 , x7 , x8 , x9 , xA , xB , xC , xD , xE , xF	
/* 0x*/	0333,0014,0006,0326,0403,0503,0510,0420,0320,0120,0031,0040,0022,0111,0003,0020,
/* 1x*/	0555,0036,0027,0407,0507,0704,0700,0630,0430,0140,0040,0053,0044,0222,0200,0310,
/* 2x*/	0777,0357,0447,0637,0707,0737,0740,0750,0660,0360,0070,0276,0077,0444,0000,0772, // value in index 2F exists in 2C03 only as index 37
/* 3x*/	0777,0567,0657,0757,0747,0755,0764,0770,0773,0572,0473,0276,0467,0666,0653,0760,
};
const unsigned short PaletteROM_2C04_0001 [64] = {
	0755,0637,0700,0447,0044,0120,0222,0704,0777,0333,0750,0503,0403,0660,0320,0777,
	0357,0653,0310,0360,0467,0657,0764,0027,0760,0276,0000,0200,0666,0444,0707,0014,
	0003,0567,0757,0070,0077,0022,0053,0507,0000,0420,0747,0510,0407,0006,0740,0000,
	0000,0140,0555,0031,0572,0326,0770,0630,0020,0036,0040,0111,0773,0737,0430,0473 
};
const unsigned short PaletteROM_2C04_0002 [64] = {
	0000,0750,0430,0572,0473,0737,0044,0567,0700,0407,0773,0747,0777,0637,0467,0040,
	0020,0357,0510,0666,0053,0360,0200,0447,0222,0707,0003,0276,0657,0320,0000,0326,
	0403,0764,0740,0757,0036,0310,0555,0006,0507,0760,0333,0120,0027,0000,0660,0777,
	0653,0111,0070,0630,0022,0014,0704,0140,0000,0077,0420,0770,0755,0503,0031,0444
};
const unsigned short PaletteROM_2C04_0003 [64] = {
	0507,0737,0473,0555,0040,0777,0567,0120,0014,0000,0764,0320,0704,0666,0653,0467,
	0447,0044,0503,0027,0140,0430,0630,0053,0333,0326,0000,0006,0700,0510,0747,0755,
	0637,0020,0003,0770,0111,0750,0740,0777,0360,0403,0357,0707,0036,0444,0000,0310,
	0077,0200,0572,0757,0420,0070,0660,0222,0031,0000,0657,0773,0407,0276,0760,0022
};
const unsigned short PaletteROM_2C04_0004 [64] = {
	0430,0326,0044,0660,0000,0755,0014,0630,0555,0310,0070,0003,0764,0770,0040,0572,
	0737,0200,0027,0747,0000,0222,0510,0740,0653,0053,0447,0140,0403,0000,0473,0357,
	0503,0031,0420,0006,0407,0507,0333,0704,0022,0666,0036,0020,0111,0773,0444,0707,
	0757,0777,0320,0700,0760,0276,0777,0467,0000,0750,0637,0567,0360,0657,0077,0120
};
const unsigned char PaletteLUT_2C04_0001 [64] ={
	0x35,0x23,0x16,0x22,0x1C,0x09,0x1D,0x15,0x20,0x00,0x27,0x05,0x04,0x28,0x08,0x20,
	0x21,0x3E,0x1F,0x29,0x3C,0x32,0x36,0x12,0x3F,0x2B,0x2E,0x1E,0x3D,0x2D,0x24,0x01,
	0x0E,0x31,0x33,0x2A,0x2C,0x0C,0x1B,0x14,0x2E,0x07,0x34,0x06,0x13,0x02,0x26,0x2E,
	0x2E,0x19,0x10,0x0A,0x39,0x03,0x37,0x17,0x0F,0x11,0x0B,0x0D,0x38,0x25,0x18,0x3A
};
const unsigned char PaletteLUT_2C04_0002 [64] ={
	0x2E,0x27,0x18,0x39,0x3A,0x25,0x1C,0x31,0x16,0x13,0x38,0x34,0x20,0x23,0x3C,0x0B,
	0x0F,0x21,0x06,0x3D,0x1B,0x29,0x1E,0x22,0x1D,0x24,0x0E,0x2B,0x32,0x08,0x2E,0x03,
	0x04,0x36,0x26,0x33,0x11,0x1F,0x10,0x02,0x14,0x3F,0x00,0x09,0x12,0x2E,0x28,0x20,
	0x3E,0x0D,0x2A,0x17,0x0C,0x01,0x15,0x19,0x2E,0x2C,0x07,0x37,0x35,0x05,0x0A,0x2D
};
const unsigned char PaletteLUT_2C04_0003 [64] ={
	0x14,0x25,0x3A,0x10,0x0B,0x20,0x31,0x09,0x01,0x2E,0x36,0x08,0x15,0x3D,0x3E,0x3C,
	0x22,0x1C,0x05,0x12,0x19,0x18,0x17,0x1B,0x00,0x03,0x2E,0x02,0x16,0x06,0x34,0x35,
	0x23,0x0F,0x0E,0x37,0x0D,0x27,0x26,0x20,0x29,0x04,0x21,0x24,0x11,0x2D,0x2E,0x1F,
	0x2C,0x1E,0x39,0x33,0x07,0x2A,0x28,0x1D,0x0A,0x2E,0x32,0x38,0x13,0x2B,0x3F,0x0C
};
const unsigned char PaletteLUT_2C04_0004 [64] ={
	0x18,0x03,0x1C,0x28,0x2E,0x35,0x01,0x17,0x10,0x1F,0x2A,0x0E,0x36,0x37,0x0B,0x39,
	0x25,0x1E,0x12,0x34,0x2E,0x1D,0x06,0x26,0x3E,0x1B,0x22,0x19,0x04,0x2E,0x3A,0x21,
	0x05,0x0A,0x07,0x02,0x13,0x14,0x00,0x15,0x0C,0x3D,0x11,0x0F,0x0D,0x38,0x2D,0x24,
	0x33,0x20,0x08,0x16,0x3F,0x2B,0x20,0x3C,0x2E,0x27,0x23,0x31,0x29,0x32,0x2C,0x09
};