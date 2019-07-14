///////////////////////////////////////////////////////////////////////////////////
//
//                                  ~~  ~~             ~~
//  ~~~~~~    ~~~~~    ~~~~~    ~~~~~~  ~~     ~~~~~   ~~~~~~    ~~~~~   ~~    ~~
//  ~~   ~~  ~~   ~~  ~~   ~~  ~~   ~~  ~~    ~~   ~~  ~~   ~~  ~~   ~~   ~~  ~~
//  ~~   ~~  ~~   ~~  ~~   ~~  ~~   ~~  ~~    ~~~~~~~  ~~   ~~  ~~   ~~     ~~
//  ~~   ~~  ~~   ~~  ~~   ~~  ~~   ~~  ~~    ~~       ~~   ~~  ~~   ~~   ~~  ~~
//  ~~   ~~   ~~~~~    ~~~~~    ~~~~~~   ~~~   ~~~~~   ~~~~~~    ~~~~~   ~~    ~~
//
//  Serendipity Sequencer                                   CC-NC-BY-SA
//  hotchk155/2018                                          Sixty-four pixels ltd
//
//  GLOBAL DEFINITIONS
//
///////////////////////////////////////////////////////////////////////////////////

#ifndef DEFS_H_
#define DEFS_H_

#if DEBUG
	#define ASSERT(e) {if(!(e)) for(;;) {}}
#else
	#define ASSERT(e)
#endif

enum {
	EV_NONE,
	EV_ENCODER,
	EV_KEY_PRESS,
	EV_KEY_RELEASE,
	EV_KEY_CLICK,
	EV_KEY_HOLD,
	EV_SEQ_START,
	EV_SEQ_STOP,
	EV_SEQ_RESTART,
	EV_CLOCK_RESET
};

#define I2C_ADDR_DAC 	0b1100000
#define I2C_ADDR_EEPROM	0b1010000


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

	P_EDIT_AUTO_GATE_INSERT,
	P_EDIT_SHOW_GRID,
	P_EDIT_SCALE_GRID,

	P_SQL_SEQ_MODE,
	P_SQL_MIX,
	P_SQL_FILL_MODE,
	P_SQL_CUE_MODE,
	P_SQL_LOOP_PER_PAGE,
	P_SQL_STEP_RATE,
	P_SQL_TRIG_DUR,
	P_SQL_MIDI_CHAN,
	P_SQL_MIDI_CC,
	P_SQL_MIDI_CC_SMOOTH,
	P_SQL_SCALE_TYPE,
	P_SQL_SCALE_ROOT,
	P_SQL_QUANTIZE,
	P_SQL_MIDI_OUT,
	P_SQL_MIDI_VEL,
	P_SQL_MIDI_BEND,
	P_SQL_CVSCALE,
	P_SQL_CVSHIFT,
	P_SQL_CVGLIDE,
	P_SQL_MAX,

	P_CLOCK_BPM,
	P_CLOCK_SRC,
	P_CLOCK_IN_RATE,
	P_CLOCK_OUT_RATE,
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
	PT_PATTERN
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
	V_CLOCK_SRC_MIDI,
	V_CLOCK_SRC_EXTERNAL,
} V_CLOCK_SRC;

typedef enum:byte {
	V_CLOCK_IN_RATE_32 = 0,
	V_CLOCK_IN_RATE_16,
	V_CLOCK_IN_RATE_8,
	V_CLOCK_IN_RATE_4,
	V_CLOCK_IN_RATE_24PPQN,
	V_CLOCK_IN_RATE_MAX
} V_CLOCK_IN_RATE;

typedef enum:byte {
	V_CLOCK_OUT_RATE_32 = 0,
	V_CLOCK_OUT_RATE_16,
	V_CLOCK_OUT_RATE_8,
	V_CLOCK_OUT_RATE_4,
	V_CLOCK_OUT_RATE_24PPQN,
	V_CLOCK_OUT_RATE_MAX
} V_CLOCK_OUT_RATE;

extern void fire_event(int event, uint32_t param);
extern void fire_note(byte midi_note, byte midi_vel);
extern void force_full_repaint();



#endif /* DEFS_H_ */
