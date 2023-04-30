//////////////////////////////////////////////////////////////////////////////
// sixty four pixels 2020                                       CC-NC-BY-SA //
//                                //  //          //                        //
//   //////   /////   /////   //////  //   /////  //////   /////  //   //   //
//   //   // //   // //   // //   //  //  //   // //   // //   //  // //    //
//   //   // //   // //   // //   //  //  /////// //   // //   //   ///     //
//   //   // //   // //   // //   //  //  //      //   // //   //  // //    //
//   //   //  /////   /////   //////   //  /////  //////   /////  //   //   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// GLOBAL DEFINITIONS
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
#ifndef DEFS_H_
#define DEFS_H_


//#define NB_PROTOTYPE 1		// define this for prototype hardware. Also need to use NoodleboxProto.mex and regenerate code
#define VERSION_NUMBER				"1.2"
#define MIDDLE_C_OCTAVE 4 		// should middle C (note 60) be C3 or C4 as displayed in editor
#define MIDI_TRANSPOSE_ZERO 60	// MIDI note that is zero transposition
#define OFF_SWITCH_MS	500		// how long the power switch is held before power turns off
#define CAL_SETTING_MIN	(-99)
#define CAL_SETTING_MAX 99


#if DEBUG
	#define ASSERT(e) {if(!(e)) for(;;) {}}
#else
	#define ASSERT(e)
#endif

#if DEBUG
	#include "stdio.h"
	#define DBGLOG0(s) printf("%s\r\n",s)
	#define DBGLOG1(s,t) printf(s,t)
	#define DBGLOG2(s,t,u) printf(s,t,u)
#else
	#define DBGLOG0(s)
	#define DBGLOG1(s,t)
	#define DBGLOG2(s,t,u)
#endif

#define NO_HIDE	volatile

enum {
	EV_NONE,

	EV_ENCODER,				// encoder turn
	EV_KEY_PRESS,			// button press down
	EV_KEY_RELEASE,			// button release
	EV_KEY_CLICK,			// button press and release
	EV_KEY_HOLD,			// button held for a long time
	EV_KEY_DOWN_RAW,		// raw key press

	EV_SEQ_RESTART,			// sequencer playback restarting from beginning
	EV_SEQ_STOP,			// sequencer playback stopped
	EV_SEQ_CONTINUE,		// sequencer playback resumes from current position
	EV_SEQ_RUN_STOP,		// send STOP or CONTINUE depending on run state
	EV_CLOCK_RESET,			// reset clock timing info
	EV_CHANGE_LAYER,		// change the current editor layer
	EV_REPAINT_MENU,
	EV_MIDI_IN_RESET,

	EV_LOAD_OK,
	EV_LOAD_FAIL,
	EV_SAVE_OK,
	EV_SAVE_FAIL,

	EV_REAPPLY_CAL_VOLTS,	// called in cal mode when the calibration settings on a layer change
	EV_REAPPLY_CONFIG,		// called when configuration is loaded from EEPROM
};

#define I2C_ADDR_DAC 	0b1100000
#define I2C_ADDR_EEPROM	0b1010000

enum {
	SLOT_CONFIG,		// configuration settings
	SLOT_AUTOSAVE,		// the working state for save at shutdown/powerup
	SLOT_TEMPLATE,		// a patch that is used as a template for new sessions
	SLOT_PATCH1,		// user patches...
	SLOT_PATCH2,
	SLOT_PATCH3,
	SLOT_PATCH4,
	SLOT_PATCH5,
	SLOT_PATCH6,
	SLOT_PATCH7,
	SLOT_PATCH8,
	NUM_SLOTS = 12
};

#define PATCH_SLOT_SIZE				2560
#define PATCH_DATA_COOKIE1			0xAA
#define PATCH_DATA_COOKIE2			0x01
#define CONFIG_DATA_COOKIE1			0xBB
#define CONFIG_DATA_COOKIE2			0x01
#define CALIBRATION_DATA_COOKIE1 	0xCC
#define CALIBRATION_DATA_COOKIE2	0x01

#ifdef NB_PROTOTYPE
	#define VERSION_STRING VERSION_NUMBER "P"
#else
	#ifdef DEBUG
		#define VERSION_STRING VERSION_NUMBER "D"
	#else
		#define VERSION_STRING VERSION_NUMBER
	#endif
#endif

typedef unsigned char byte;

#define PORTA_BASE 0
#define PORTB_BASE 8
#define PORTC_BASE 16
#define PORTD_BASE 24


#define MK_GPIOA_BIT(port, bit) (((uint32_t)1) << ((port) + (bit)))
#define SET_GPIOA(mask) ((GPIO_Type *)GPIOA_BASE)->PSOR = (mask)
#define CLR_GPIOA(mask) ((GPIO_Type *)GPIOA_BASE)->PCOR = (mask)
#define READ_GPIOA(mask) (((GPIO_Type *)GPIOA_BASE)->PDIR & (mask))

typedef enum:byte {
	P_NONE = 0,


	P_SQL_AUTO_GATE_INSERT,
	P_SQL_SHOW_GRID,
	P_SQL_MIDI_IN_CHAN,
	P_SQL_SEQ_MODE,
	P_SQL_MIX,
	P_SQL_FILL_MODE,
	P_SQL_LOOP_PER_PAGE,
	P_SQL_SCALED_VIEW,
	P_SQL_STEP_RATE,
	P_SQL_OFF_GRID_MODE,
	P_SQL_OFF_GRID_AMOUNT,
	P_SQL_TRIG_DUR,
	P_SQL_MIDI_OUT_CHAN,
	P_SQL_MIDI_CC,
	P_SQL_MIDI_CC_SMOOTH,
	P_SQL_QUANTIZE,
	P_SQL_MIDI_OUT,
	P_SQL_MIDI_VEL,
	P_SQL_MIDI_ACC_VEL,
	P_SQL_CVSCALE,
	P_SQL_CV_OCTAVE,
	P_SQL_CV_TRANSPOSE,
	P_SQL_CVGLIDE,
	P_SQL_CV_ALIAS,
	P_SQL_GATE_ALIAS,
	P_SQL_OUT_CAL_SCALE,
	P_SQL_OUT_CAL_OFFSET,
	P_SQL_MAX,

	P_SEQ_REC_MODE,
	P_SEQ_REC_ARM,
	P_SEQ_SCALE_TYPE,
	P_SEQ_SCALE_ROOT,
	P_SEQ_OUT_CAL,
	P_SEQ_MAX,

	P_CLOCK_BPM,
	P_CLOCK_SRC,
	P_CLOCK_IN_RATE,
	P_CLOCK_OUT_MODE,
	P_CLOCK_OUT_RATE,
	P_MIDI_CLOCK_OUT,
	P_AUX_OUT_MODE,
	P_AUX_OUT_RATE,
	P_AUX_IN_MODE,
	P_CLOCK_MAX
} PARAM_ID;

typedef enum:byte {
	PT_NONE = 0,
	PT_ENUMERATED,
	PT_MIDI_CHANNEL,
	PT_NUMBER_7BIT,
	PT_VOLT_RANGE,
	PT_BPM,
	PT_DURATION,
	PT_PATTERN,
	PT_CALIBRATION,
	PT_TRANSPOSE
} PARAM_TYPE;

typedef enum:byte {
	V_SQL_SEQ_MODE_PITCH = 0,
	V_SQL_SEQ_MODE_MOD,
	V_SQL_SEQ_MODE_OFFSET,
	V_SQL_SEQ_MODE_MAX
} V_SQL_SEQ_MODE;

typedef enum:byte {
	V_SQL_FILL_MODE_OFF,
	V_SQL_FILL_MODE_PAD,
	V_SQL_FILL_MODE_INTERPOLATE,
	V_SQL_FILL_MODE_MAX
} V_SQL_FILL_MODE;

typedef enum:byte {
	V_SQL_SEQ_QUANTIZE_OFF = 0,
	V_SQL_SEQ_QUANTIZE_CHROMATIC,
	V_SQL_SEQ_QUANTIZE_SCALE,
} V_SQL_QUANTIZE;

typedef enum:byte {
	V_SQL_SCALE_TYPE_IONIAN,
	V_SQL_SCALE_TYPE_DORIAN,
	V_SQL_SCALE_TYPE_PHRYGIAN,
	V_SQL_SCALE_TYPE_LYDIAN,
	V_SQL_SCALE_TYPE_MIXOLYDIAN,
	V_SQL_SCALE_TYPE_AEOLIAN,
	V_SQL_SCALE_TYPE_LOCRIAN,
	V_SQL_SCALE_TYPE_MAX
} V_SQL_SCALE_TYPE;

typedef enum:byte {
	V_SQL_SCALE_ROOT_C = 0,
	V_SQL_SCALE_ROOT_CSHARP,
	V_SQL_SCALE_ROOT_D,
	V_SQL_SCALE_ROOT_DSHARP,
	V_SQL_SCALE_ROOT_E,
	V_SQL_SCALE_ROOT_F,
	V_SQL_SCALE_ROOT_FSHARP,
	V_SQL_SCALE_ROOT_G,
	V_SQL_SCALE_ROOT_GSHARP,
	V_SQL_SCALE_ROOT_A,
	V_SQL_SCALE_ROOT_ASHARP,
	V_SQL_SCALE_ROOT_B
} V_SQL_SCALE_ROOT;

typedef enum:byte {
	V_SQL_COMBINE_OFF,
	V_SQL_COMBINE_ADD,
	V_SQL_COMBINE_MASK,
	V_SQL_COMBINE_ADD_MASK,
	V_SQL_COMBINE_MAX
} V_SQL_COMBINE;

typedef enum:byte {
	V_SQL_MIDI_OUT_NONE,
	V_SQL_MIDI_OUT_NOTE,
	V_SQL_MIDI_OUT_CC,
	V_SQL_MIDI_OUT_MAX
} V_SQL_MIDI_OUT;

typedef enum:byte {
	V_SQL_OFF_GRID_MODE_NONE,
	V_SQL_OFF_GRID_MODE_SWING,
	V_SQL_OFF_GRID_MODE_SLIDE,
	V_SQL_OFF_GRID_MODE_RANDOM,
	V_SQL_STEP_MOD_MAX
} V_SQL_OFF_GRID_MODE;

typedef enum:byte {
	V_SQL_STEP_RATE_1 = 0,
	V_SQL_STEP_RATE_2D,
	V_SQL_STEP_RATE_2,
	V_SQL_STEP_RATE_4D,
	V_SQL_STEP_RATE_2T,
	V_SQL_STEP_RATE_4,
	V_SQL_STEP_RATE_8D,
	V_SQL_STEP_RATE_4T,
	V_SQL_STEP_RATE_8,
	V_SQL_STEP_RATE_16D,
	V_SQL_STEP_RATE_8T,
	V_SQL_STEP_RATE_16,
	V_SQL_STEP_RATE_16T,
	V_SQL_STEP_RATE_32,
	V_SQL_STEP_RATE_MAX
} V_SQL_STEP_RATE;

typedef enum:byte {
	V_SQL_NOTE_DUR_TRIG,
	V_SQL_NOTE_DUR_1,
	V_SQL_NOTE_DUR_2,
	V_SQL_NOTE_DUR_3,
	V_SQL_NOTE_DUR_4,
	V_SQL_NOTE_DUR_5,
	V_SQL_NOTE_DUR_6,
	V_SQL_NOTE_DUR_7,
	V_SQL_NOTE_DUR_8,
	V_SQL_NOTE_DUR_9,
	V_SQL_NOTE_DUR_10,
	V_SQL_NOTE_DUR_11,
	V_SQL_NOTE_DUR_12,
	V_SQL_NOTE_DUR_13,
	V_SQL_NOTE_DUR_14,
	V_SQL_NOTE_DUR_15,
	V_SQL_NOTE_DUR_16,
	V_SQL_NOTE_DUR_MAX
} V_SQL_TRIG_DUR;


typedef enum:byte {
	V_SQL_CVSCALE_1V,
	V_SQL_CVSCALE_2V,
	V_SQL_CVSCALE_3V,
	V_SQL_CVSCALE_4V,
	V_SQL_CVSCALE_5V,
	V_SQL_CVSCALE_6V,
	V_SQL_CVSCALE_7V,
	V_SQL_CVSCALE_8V,
	V_SQL_CVSCALE_1VOCT,
	V_SQL_CVSCALE_1_2VOCT,
	V_SQL_CVSCALE_HZVOLT,
	V_SQL_CVSCALE_MAX
} V_SQL_CVSCALE;

typedef enum:byte {
	V_SQL_CVSHIFT_DOWN5,
	V_SQL_CVSHIFT_DOWN4,
	V_SQL_CVSHIFT_DOWN3,
	V_SQL_CVSHIFT_DOWN2,
	V_SQL_CVSHIFT_DOWN1,
	V_SQL_CVSHIFT_NONE,
	V_SQL_CVSHIFT_UP1,
	V_SQL_CVSHIFT_UP2,
	V_SQL_CVSHIFT_UP3,
	V_SQL_CVSHIFT_UP4,
	V_SQL_CVSHIFT_UP5,
	V_SQL_CVSHIFT_MAX
} V_SQL_CVSHIFT;

typedef enum:byte {
	V_SQL_CVGLIDE_OFF= 0,
	V_SQL_CVGLIDE_ON,
	V_SQL_CVGLIDE_TIE,
	V_SQL_CVGLIDE_MAX
} V_SQL_CVGLIDE;

typedef enum:byte {
	V_CLOCK_SRC_INTERNAL = 0,
	V_CLOCK_SRC_MIDI_CLOCK_ONLY,
	V_CLOCK_SRC_MIDI_TRANSPORT,
	V_CLOCK_SRC_EXTERNAL,
} V_CLOCK_SRC;

typedef enum:byte {
	V_CLOCK_IN_RATE_8,
	V_CLOCK_IN_RATE_16,
	V_CLOCK_IN_RATE_32,
	V_CLOCK_IN_RATE_24PP,
	V_CLOCK_IN_RATE_MAX
} V_CLOCK_IN_RATE;

typedef enum:byte {
	V_CLOCK_OUT_RATE_8,
	V_CLOCK_OUT_RATE_16,
	V_CLOCK_OUT_RATE_32,
	V_CLOCK_OUT_RATE_24PP,
	V_CLOCK_OUT_RATE_MAX
} V_CLOCK_OUT_RATE;


typedef enum:byte {
	V_CLOCK_OUT_MODE_NONE,
	V_CLOCK_OUT_MODE_CLOCK,
	V_CLOCK_OUT_MODE_GATED_CLOCK,
	V_CLOCK_OUT_MODE_START,
	V_CLOCK_OUT_MODE_STOP,
	V_CLOCK_OUT_MODE_START_STOP,
	V_CLOCK_OUT_MODE_RESET,
	V_CLOCK_OUT_MODE_RUNNING,
	V_CLOCK_OUT_MODE_ACCENT,
	V_CLOCK_OUT_MODE_MAX,
} V_CLOCK_OUT_MODE;

typedef enum:byte {
	V_MIDI_CLOCK_OUT_NONE,
	V_MIDI_CLOCK_OUT_ON,
	V_MIDI_CLOCK_OUT_ON_TRAN,
	V_MIDI_CLOCK_OUT_GATE,
	V_MIDI_CLOCK_OUT_GATE_TRAN,
	V_MIDI_CLOCK_OUT_MAX
} V_MIDI_CLOCK_OUT;

typedef enum:byte {
	V_AUX_IN_MODE_NONE,
	V_AUX_IN_MODE_RUN_STOP,
	V_AUX_IN_MODE_CONT,
	V_AUX_IN_MODE_RESTART,
	V_AUX_IN_MODE_MAX
} V_AUX_IN_MODE;

typedef enum:byte {
	V_SQL_CV_ALIAS_NONE,
	V_SQL_CV_ALIAS_FROM_L1,
	V_SQL_CV_ALIAS_FROM_L2,
	V_SQL_CV_ALIAS_FROM_L3,
	V_SQL_CV_ALIAS_FROM_L4,
	V_SQL_CV_ALIAS_MAX
} V_SQL_CV_ALIAS;

typedef enum:byte {
	V_SQL_GATE_ALIAS_NONE,
	V_SQL_GATE_ALIAS_FROM_L1,
	V_SQL_GATE_ALIAS_FROM_L2,
	V_SQL_GATE_ALIAS_FROM_L3,
	V_SQL_GATE_ALIAS_FROM_L4,
	V_SQL_GATE_ALIAS_MAX
} V_SQL_GATE_ALIAS;

typedef enum:byte {
	V_SEQ_OUT_CAL_NONE,
	V_SEQ_OUT_CAL_1V,
	V_SEQ_OUT_CAL_2V,
	V_SEQ_OUT_CAL_3V,
	V_SEQ_OUT_CAL_4V,
	V_SEQ_OUT_CAL_5V,
	V_SEQ_OUT_CAL_6V,
	V_SEQ_OUT_CAL_7V,
	V_SEQ_OUT_CAL_8V,
	V_SEQ_OUT_CAL_MAX
} V_SEQ_OUT_CAL;


typedef enum:byte {
	V_SEQ_REC_MODE_NONE,
	V_SEQ_REC_MODE_CV,
	V_SEQ_REC_MODE_TRANSPOSE,
	V_SEQ_REC_MODE_MAX
} V_SEQ_REC_MODE;

typedef enum:byte {
	V_SEQ_REC_ARM_OFF,
	V_SEQ_REC_ARM_ON,
	V_SEQ_REC_ARM_MAX
} V_SEQ_REC_ARM;

typedef enum:byte {
	V_SQL_MIDI_IN_CHAN_1,
	V_SQL_MIDI_IN_CHAN_2,
	V_SQL_MIDI_IN_CHAN_3,
	V_SQL_MIDI_IN_CHAN_4,
	V_SQL_MIDI_IN_CHAN_5,
	V_SQL_MIDI_IN_CHAN_6,
	V_SQL_MIDI_IN_CHAN_7,
	V_SQL_MIDI_IN_CHAN_8,
	V_SQL_MIDI_IN_CHAN_9,
	V_SQL_MIDI_IN_CHAN_10,
	V_SQL_MIDI_IN_CHAN_11,
	V_SQL_MIDI_IN_CHAN_12,
	V_SQL_MIDI_IN_CHAN_13,
	V_SQL_MIDI_IN_CHAN_14,
	V_SQL_MIDI_IN_CHAN_15,
	V_SQL_MIDI_IN_CHAN_16,
	V_SQL_MIDI_IN_CHAN_OMNI,
	V_SQL_MIDI_IN_CHAN_MAX
} V_SQL_MIDI_IN_CHAN;

extern void fire_event(int event, uint32_t param);
extern void fire_note(byte midi_note, byte midi_vel);
extern void force_full_repaint();
void set(PARAM_ID param, int value);
int get(PARAM_ID param);
int is_valid_for_menu(PARAM_ID param);
int is_cal_mode();
void save_config();



#endif /* DEFS_H_ */
