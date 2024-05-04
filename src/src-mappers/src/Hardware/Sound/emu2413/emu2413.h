#ifndef _EMU2413_H_
#define _EMU2413_H_

#ifdef EMU2413_DLL_EXPORTS
  #define EMU2413_API __declspec(dllexport)
#elif defined(EMU2413_DLL_IMPORTS)
  #define EMU2413_API __declspec(dllimport)
#else
  #define EMU2413_API
#endif

#define PI 3.14159265358979323846

namespace OPLLsound {

enum OPLL_TONE_ENUM {OPLL_2413_TONE=0, OPLL_VRC7_TONE=1, OPLL_281B_TONE=2} ;

/* voice data */
typedef struct __OPLL_PATCH {
  uint32_t TL,FB,EG,ML,AR,DR,SL,RR,KR,KL,AM,PM,WF ;
} OPLL_PATCH ;

/* slot */
typedef struct __OPLL_SLOT {

  OPLL_PATCH *patch;  

  int32 type ;          /* 0 : modulator 1 : carrier */

  /* OUTPUT */
  int32 feedback ;
  int32 output[2] ;   /* Output value of slot */

  /* for Phase Generator (PG) */
  uint32_t *sintbl ;    /* Wavetable */
  uint32_t phase ;      /* Phase */
  uint32_t dphase ;     /* Phase increment amount */
  uint32_t pgout ;      /* output */

  /* for Envelope Generator (EG) */
  int32 fnum ;          /* F-Number */
  int32 block ;         /* Block */
  int32 volume ;        /* Current volume */
  int32 sustine ;       /* Sustine 1 = ON, 0 = OFF */
  uint32_t tll ;	      /* Total Level + Key scale level*/
  uint32_t rks ;        /* Key scale offset (Rks) */
  int32 eg_mode ;       /* Current state */
  uint32_t eg_phase ;   /* Phase */
  uint32_t eg_dphase ;  /* Phase increment amount */
  uint32_t egout ;      /* output */

} OPLL_SLOT ;

/* Mask */
#define OPLL_MASK_CH(x) (1<<(x))
#define OPLL_MASK_HH (1<<(9))
#define OPLL_MASK_CYM (1<<(10))
#define OPLL_MASK_TOM (1<<(11))
#define OPLL_MASK_SD (1<<(12))
#define OPLL_MASK_BD (1<<(13))
#define OPLL_MASK_RHYTHM ( OPLL_MASK_HH | OPLL_MASK_CYM | OPLL_MASK_TOM | OPLL_MASK_SD | OPLL_MASK_BD )

/* opll */
typedef struct __OPLL {

  uint8_t vrc7_mode;
  uint8_t adr ;
  int32 out ;

#ifndef EMU2413_COMPACTION
  uint32_t realstep ;
  uint32_t oplltime ;
  uint32_t opllstep ;
  int32 prev, next ;
  int32 sprev[2],snext[2];
  float pan[14][2];
#endif

  /* Register */
  uint8_t reg[0x40] ; 
  int32 slot_on_flag[18] ;

  /* Pitch Modulator */
  uint32_t pm_phase ;
  int32 lfo_pm ;

  /* Amp Modulator */
  int32 am_phase ;
  int32 lfo_am ;

  uint32_t quality;

  /* Noise Generator */
  uint32_t noisseed ;

  /* Channel Data */
  int32 patch_number[9];
  int32 key_status[9] ;

  /* Slot */
  OPLL_SLOT slot[18] ;

  /* Voice Data */
  OPLL_PATCH patch[19*2] ;
  int32 patch_update[2] ; /* flag for check patch update */

  uint32_t mask ;

} OPLL ;

/* Create Object */
EMU2413_API OPLL *OPLL_new(uint32_t clk, uint32_t rate, bool isVRC7) ;
EMU2413_API void OPLL_delete(OPLL *) ;

/* Setup */
EMU2413_API void OPLL_reset(OPLL *) ;
EMU2413_API void OPLL_reset_patch(OPLL *, int32) ;
EMU2413_API void OPLL_set_rate(OPLL *opll, uint32_t r) ;
EMU2413_API void OPLL_set_quality(OPLL *opll, uint32_t q) ;
EMU2413_API void OPLL_set_pan(OPLL *, uint32_t ch, int32 pan);

/* Port/Register access */
EMU2413_API void OPLL_writeIO(OPLL *, uint32_t reg, uint32_t val) ;
EMU2413_API void OPLL_writeReg(OPLL *, uint32_t reg, uint32_t val) ;

/* Synthsize */
EMU2413_API int32 OPLL_calc(OPLL *) ;

/* Misc */
EMU2413_API void OPLL_setPatch(OPLL *, const uint8_t *dump) ;
EMU2413_API void OPLL_copyPatch(OPLL *, int32, OPLL_PATCH *) ;
EMU2413_API void OPLL_forceRefresh(OPLL *) ;
/* Utility */
EMU2413_API void OPLL_dump2patch(const uint8_t *dump, OPLL_PATCH *patch) ;
EMU2413_API void OPLL_patch2dump(const OPLL_PATCH *patch, uint8_t *dump) ;
EMU2413_API void OPLL_getDefaultPatch(int32 type, int32 num, OPLL_PATCH *) ;

#define dump2patch OPLL_dump2patch

}

#endif

