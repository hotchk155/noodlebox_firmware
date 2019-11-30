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
//  BEAT CLOCK HANDLING
//
///////////////////////////////////////////////////////////////////////////////////
#pragma GCC diagnostic ignored "-Wswitch"

#ifndef CLOCK_H_
#define CLOCK_H_


// define the GPIO pins used for clock (will initialise the port)
CDigitalIn<kGPIO_PORTA, 0> g_clock_in;
CPulseOut<kGPIO_PORTC, 5> g_clock_out;

// Noodlebox uses the following type for handling "musical time"...
// TICKS_TYPE is a 32 bit unsigned value where there are 256 * 24ppqn = 6144 LSB
// increments per quarter note.
// ~699050 beats before rolling over (about 38h @ 300bpm, 97h @ 120bpm)
typedef uint32_t TICKS_TYPE;
enum {
	TICKS_INFINITY = (TICKS_TYPE)(-1)
};

// Define different musical beat intervals based on
// number of 24ppqn ticks
enum
{
  PP24_1    	= 96,
  PP24_2D   	= 72,
  PP24_2    	= 48,
  PP24_4D   	= 36,
  PP24_2T   	= 32,
  PP24_4    	= 24,
  PP24_8D   	= 18,
  PP24_4T   	= 16,
  PP24_8    	= 12,
  PP24_16D  	= 9,
  PP24_8T   	= 8,
  PP24_16   	= 6,
  PP24_16T  	= 4,
  PP24_32   	= 3,
  PP24_24PPQN	= 1
};


class IClockSource {
public:
	virtual void reset() = 0;
	virtual TICKS_TYPE min_ticks() = 0;
	virtual TICKS_TYPE max_ticks() = 0;
	virtual double ticks_per_ms() = 0;
};


/////////////////////////////////////////////////////////////////
// This class is used for managing the pulse clock  output
class CPulseClockOut {
	const int HIGH_MS = 15;		// duration of high part of pulse
	const int LOW_MS = 2;		// MIN low time between pulses

	enum { ST_IDLE, ST_HIGH, ST_LOW } m_state; // pulse state
	int m_timeout;				// time to remain in state
	int m_pulses;				// number of pulses remaining to send
	int m_period;
public:
	void reset() {
		m_state = ST_IDLE;
		m_timeout = 0;
		m_pulses = 0;
		m_period = PP24_16;
	}
	inline void on_pp24(int pp24) {
		// check if we need to send a clock out pulse
		ASSERT(m_period);
		if(!(pp24%m_period)) {
			// can we do it immediately?
			if(ST_IDLE == m_state) {
				g_clock_out.set(1);
				m_state = ST_HIGH;
				m_timeout = HIGH_MS;
			}
			else {
				// queue it up
				++m_pulses;
			}
		}

	}
	inline void run() {
		// manage the clock output
		switch(m_state) {
		case ST_LOW:	// forced low state after a pulse
			if(!--m_timeout) { // see if end of low phase
				if(m_pulses) { // do we need another pulse?
					g_clock_out.set(1);
					--m_pulses;
					m_state = ST_HIGH;
					m_timeout = HIGH_MS;
				}
				else { // nope
					m_state = ST_IDLE;
				}
			}
			break;
		case ST_HIGH:
			if(!--m_timeout) {
				g_clock_out.set(0);
				m_state = ST_LOW;
				m_timeout = LOW_MS;
			}
			break;
		}

	}
};
/*
class CPulseClockIn {
	TICKS_TYPE m_period;	// this is the period of the clock in ticks
	TICKS_TYPE m_ticks;

	uint32_t m_last_ms;
	uint32_t m_timeout;
	SUBTICKS_TYPE m_subticks_per_ms; // measured subticks per ms of ext clock
	enum { ST_NONE, ST_STOPPED, ST_RUNNING } m_state;

public:
	void reset() {
		m_state = ST_NONE;
	}
	TICKS_TYPE min_ticks() {
		return m_ticks;
	}
	TICKS_TYPE max_ticks() {
		return m_ticks + m_period;
	}
	SUBTICKS_TYPE subticks_per_ms() {
		return m_subticks_per_ms;
	}


	void on_pulse(uint32_t ms) {
		if(ST_RUNNING == m_state) {

			// check we have not rolled over
			if(ms > m_last_ms) {
				uint32_t elapsed_ms = (ms - m_last_ms);
				m_subticks_per_ms = (m_period<<24) / elapsed_ms;
				m_timeout = ms + 2 * elapsed_ms;
			}
		}
		else {
			m_timeout = 0;
			m_state = ST_RUNNING;
			fire_event(EV_SEQ_RESTART,0);
		}
		m_last_ms = ms;
		m_ticks += m_period;


	}
	void run(uint32_t ms) {
		if(ST_RUNNING == m_state && !!m_timeout && ms > m_timeout) {
			m_timeout = 0;
			m_state = ST_STOPPED;
			fire_event(EV_SEQ_STOP,0);
		}
	}
	inline uint32_t get_subticks_per_ms() {
		return m_subticks_per_ms;
	}

		/////////////////////////////////////////////////////////////////////////
		// If the external clock has accelerated, we may not have had time to
		// process all the interpolated PP24 ticks before getting another
		// external tick. We will simply rush through them all now!

		m_part_ticks  = 0;
		m_ticks &= ~0xFF;
		while(m_ticks < m_next_clock_in_ticks) {
			m_ticks+=0x100;
			on_pp24(ticks_to_pp24(m_ticks));
		}
		m_next_clock_in_ticks += period;


};

class CMidiClockIn {

};
*/
class CFixedClock : public IClockSource {
	double m_ticks_per_ms;
	float m_bpm;
public:
	CFixedClock() {
		reset();
	}
	void reset() {
		set_bpm(120);
	}
	TICKS_TYPE min_ticks() {
		return 0;
	}
	TICKS_TYPE max_ticks() {
		return TICKS_INFINITY;
	}
	double ticks_per_ms() {
		return m_ticks_per_ms;
	}
	void set_bpm(float bpm) {
		m_ticks_per_ms = (bpm * PP24_4 * 256) / (60.0 * 1000.0);
		m_bpm = bpm;
	}
	float get_bpm() {
		return m_bpm;
	}
};

/////////////////////////////////////////////////////////////////
// This class maintains a count of 24ppqn ticks based on the
// current BPM or the external MIDI clock. Internal clock is
// generated based on a once-per-millisecond interrupt
class CClock {
public:


	inline int ticks_to_pp24(TICKS_TYPE ticks) {
		return ticks>>8;
	}
	inline TICKS_TYPE pp24_to_ticks(int pp24) {
		return ((TICKS_TYPE)pp24)<<8;
	}

private:


	static const byte c_clock_in_rate[V_CLOCK_IN_RATE_MAX];
	static const byte c_clock_out_rate[V_CLOCK_OUT_RATE_MAX];

	volatile byte m_ms_tick;					// flag set each time 1ms is up
	volatile uint32_t m_ms;					// ms counter

	volatile TICKS_TYPE m_ticks;
	volatile double m_ticks_remainder;


	CFixedClock m_fixed_clock;
	IClockSource &m_source;

	CPulseClockOut m_pulse_clock_out;
public:


	///////////////////////////////////////////////////////////////////////////////
	// Config structure that defines info to store at power off
	typedef struct {
		V_CLOCK_SRC m_source;
		V_CLOCK_IN_RATE m_clock_in_rate;
		V_CLOCK_OUT_RATE m_clock_out_rate;
	} CONFIG;
	CONFIG m_cfg;

	///////////////////////////////////////////////////////////////////////////////
	CClock() : m_source(m_fixed_clock) {
		init_config();
		init_state();
	}

	///////////////////////////////////////////////////////////////////////////////
	void on_restart() {
		m_ticks = 0;
		m_ticks_remainder = 0;

		m_pulse_clock_out.reset();
	}

	///////////////////////////////////////////////////////////////////////////////
	void init_state() {
		m_ms = 0;
		m_ms_tick = 0;
		on_restart();
	}

	///////////////////////////////////////////////////////////////////////////////
	void init_config() {
		m_cfg.m_source = V_CLOCK_SRC_INTERNAL;
		m_cfg.m_clock_in_rate = V_CLOCK_IN_RATE_16;
		m_cfg.m_clock_out_rate = V_CLOCK_OUT_RATE_16;
	}

	///////////////////////////////////////////////////////////////////////////////
	void init() {
		// configure a timer to cause an interrupt once per millisecond
		CLOCK_EnableClock(kCLOCK_Pit0);
		pit_config_t timerConfig = {
		 .enableRunInDebug = true,
		};
		PIT_Init(PIT, &timerConfig);
		EnableIRQ(PIT_CH0_IRQn);
		PIT_EnableInterrupts(PIT, kPIT_Chnl_0, kPIT_TimerInterruptEnable);
		PIT_SetTimerPeriod(PIT, kPIT_Chnl_0, (uint32_t) MSEC_TO_COUNT(1, CLOCK_GetBusClkFreq()));
		PIT_StartTimer(PIT, kPIT_Chnl_0);

		// configure the KBI peripheral to cause an interrupt when sync pulse in is triggered
		kbi_config_t kbiConfig;
		kbiConfig.mode = kKBI_EdgesDetect;
		kbiConfig.pinsEnabled = 0x01; // KBI0 pin 0
		kbiConfig.pinsEdge = 0; // Falling Edge
		KBI_Init(KBI0, &kbiConfig);
	}


	///////////////////////////////////////////////////////////////////////////////
	void set(PARAM_ID param, int value) {
		switch(param) {
			case P_CLOCK_BPM:
				m_fixed_clock.set_bpm(value);
				break;
			case P_CLOCK_SRC:
				m_cfg.m_source = (V_CLOCK_SRC)value;
				fire_event(EV_SEQ_RESET, 0);
				break;
			case P_CLOCK_IN_RATE:
				m_cfg.m_clock_in_rate = (V_CLOCK_IN_RATE)value;
				fire_event(EV_SEQ_RESET, 0);
				break;
			case P_CLOCK_OUT_RATE:
				m_cfg.m_clock_out_rate = (V_CLOCK_OUT_RATE)value;
				break;
		default:
			break;
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	int get(PARAM_ID param) {
		switch(param) {
		case P_CLOCK_BPM: return m_fixed_clock.get_bpm();
		case P_CLOCK_SRC: return m_cfg.m_source;
		case P_CLOCK_IN_RATE: return m_cfg.m_clock_in_rate;
		case P_CLOCK_OUT_RATE: return m_cfg.m_clock_out_rate;
		default: return 0;
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	int is_valid_param(PARAM_ID param) {
		switch(param) {
		case P_CLOCK_BPM: return !!(m_cfg.m_source == V_CLOCK_SRC_INTERNAL);
		case P_CLOCK_IN_RATE: return !!(m_cfg.m_source == V_CLOCK_SRC_EXTERNAL);
		default: return 1;
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	void wait_ms(int ms) {
		while(ms) {
			m_ms_tick = 0;
			while(!m_ms_tick);
			--ms;
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	inline TICKS_TYPE get_ticks() {
		return m_ticks;
	}

/*
	///////////////////////////////////////////////////////////////////////////////
	void on_midi_tick() {
		if(m_cfg.m_source == V_CLOCK_SRC_MIDI) {
			on_clock_in(PP24_24PPQN);
		}
	}
*/
	inline uint32_t get_ms() {
		return m_ms;
	}

	inline byte is_ms_tick() {
		if(m_ms_tick) {
			m_ms_tick = 0;
			return 1;
		}
		return 0;
	}


	///////////////////////////////////////////////////////////////////////////////
	inline int pp24_per_measure(V_SQL_STEP_RATE step_rate) {
		const byte pp24[V_SQL_STEP_RATE_MAX] = {
			PP24_1,
			PP24_2D,
			PP24_2,
			PP24_4D,
			PP24_2T,
			PP24_4,
			PP24_8D,
			PP24_4T,
			PP24_8,
			PP24_16D,
			PP24_8T,
			PP24_16,
			PP24_16T,
			PP24_32
		};
		return pp24[step_rate];
	}

	///////////////////////////////////////////////////////////////////////////////
	inline int get_ms_for_pp24(int pp24) {
		return (int)(pp24_to_ticks(pp24)/m_source.ticks_per_ms());
	}

	///////////////////////////////////////////////////////////////////////////////
	inline int get_ms_per_measure(V_SQL_STEP_RATE step_rate) {
		return get_ms_for_pp24(pp24_per_measure(step_rate));
	}



	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////
	// once per ms
	void run() {
		m_pulse_clock_out.run();

/*
		m_clock_out.run();

		int pp24 = ticks_to_pp24(m_ticks);
		if(pp24 != m_last_pp24) {
			m_last_pp24 = pp24;
			if(!(pp24%PP24_4)) {
				g_tempo_led.blink(g_tempo_led.MEDIUM_BLINK);
			}
		}

		if(m_cfg.m_source == V_CLOCK_SRC_EXTERNAL) {
			// assume the external clock has stopped when we do not
			// see an incoming tick within the allowed timeout
			if(m_clock_in_timeout && m_ms > m_clock_in_timeout) {
				fire_event(EV_SEQ_STOP,0);
				m_clock_in_timeout = 0;
				m_clock_in_ms_last = 0;
			}
		}

*/
	}

	///////////////////////////////////////////////////////////////////////////////
	// Interrupt service routine called once per millisecond
	void per_ms_isr() {

		// maintain a millisecond counter for general
		// timing purposes
		++m_ms;

		// set a tick flag which is used for general
		// ting purposes
		m_ms_tick = 1;

		TICKS_TYPE prev_ticks = m_ticks;

		// update the tick counter used for scheduling the sequencer
		if(m_ticks < m_source.min_ticks()) {
			m_ticks = m_source.min_ticks();
			m_ticks_remainder = 0;
		}
		else {
			// count the appropriate number of ticks for this millisecond
			// but don't yet save the values
			double ticks_remainder = m_ticks_remainder + m_source.ticks_per_ms();
			int whole_ticks = (int)ticks_remainder;
			ticks_remainder -= whole_ticks;
			TICKS_TYPE ticks = m_ticks + whole_ticks;

			if(ticks < m_source.max_ticks()) {
				m_ticks = ticks;
				m_ticks_remainder = ticks_remainder;
			}
		}

		if((m_ticks ^ prev_ticks)&~0xFF) {
			int pp24 = m_ticks>>8;
			m_pulse_clock_out.on_pp24(pp24);
		}

	}





	inline void ext_clock_isr() {
		/*
		if(m_cfg.m_source == V_CLOCK_SRC_EXTERNAL) {
			on_clock_in(c_clock_in_rate[m_cfg.m_clock_in_rate]);
		}
		*/
	}
};



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


// define the clock instance
CClock g_clock;

const byte CClock::c_clock_in_rate[V_CLOCK_IN_RATE_MAX] = {
	PP24_32, PP24_16, PP24_8, PP24_4, PP24_24PPQN
};
const byte CClock::c_clock_out_rate[V_CLOCK_OUT_RATE_MAX] = {
	PP24_32, PP24_16, PP24_8, PP24_4, PP24_24PPQN
};


// ISR for the millisecond timer
extern "C" void PIT_CH0_IRQHandler(void) {
	PIT_ClearStatusFlags(PIT, kPIT_Chnl_0, kPIT_TimerFlag);
	g_clock.per_ms_isr();
}

// ISR for the KBI interrupt (SYNC IN)
extern "C" void KBI0_IRQHandler(void)
{
    if (KBI_IsInterruptRequestDetected(KBI0)) {
        KBI_ClearInterruptFlag(KBI0);
        g_clock.ext_clock_isr();
    }
}

#endif // CLOCK_H_
