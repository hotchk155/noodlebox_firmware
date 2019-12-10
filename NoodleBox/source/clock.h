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

// This namespace wraps up various clock based utilities
namespace clock {

// Noodlebox uses the following type for handling "musical time"...
// TICKS_TYPE is a 32 bit unsigned value where there are 256 * 24ppqn = 6144 LSB
// increments per quarter note.
// ~699050 beats before rolling over (about 38h @ 300bpm, 97h @ 120bpm)
typedef uint32_t TICKS_TYPE;
enum {
	TICKS_INFINITY = (TICKS_TYPE)(-1)
};

// Define different musical beat intervals based on number of 24ppqn ticks
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

///////////////////////////////////////////////////////////////////////////////
// Conversion from TICKS_TYPE to PP24
inline int ticks_to_pp24(TICKS_TYPE ticks) {
	return ticks>>8;
}

///////////////////////////////////////////////////////////////////////////////
// Conversion from PP24 to TICKS_TYPE
inline TICKS_TYPE pp24_to_ticks(int pp24) {
	return ((TICKS_TYPE)pp24)<<8;
}

///////////////////////////////////////////////////////////////////////////////
// Conversion from V_SQL_STEP_RATE to PP24
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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Interface to be implemented by clock sources
class IClockSource {
public:
	virtual void event(int event, uint32_t param) = 0;
	virtual TICKS_TYPE min_ticks() = 0;
	virtual TICKS_TYPE max_ticks() = 0;
	virtual double ticks_per_ms() = 0;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Clock source to handle an incoming pulse clock
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CPulseClockSource : public IClockSource {

	V_CLOCK_IN_RATE m_clock_in_rate;

	TICKS_TYPE m_ticks; 	// tick counter incremented by m_period when external clock pulse is received
	TICKS_TYPE m_period;	// the period of the clock in whole ticks
	double m_ticks_per_ms;  // calculated tick rate from ext clock

	uint32_t m_last_ms;		// used to time between incoming pulses
	uint32_t m_timeout;		// used to decide when the external clock has stopped
	byte m_state;
	enum {
		CLOCK_UNKNOWN,
		CLOCK_STOPPED,
		CLOCK_RUNNING
	};

public:
	////////////////////////////////////////
	CPulseClockSource() {
		set_rate(V_CLOCK_IN_RATE_16);
	}
	////////////////////////////////////////
	void event(int event, uint32_t param) {
		switch(event) {
		case EV_CLOCK_RESET:
			m_state = CLOCK_UNKNOWN;
			m_ticks_per_ms = 0;
			m_last_ms = 0;
			// fallthru
		case EV_SEQ_RESTART:
			m_ticks = 0;
			break;
		}
	}
	////////////////////////////////////////
	TICKS_TYPE min_ticks() {
		return m_ticks;
	}
	////////////////////////////////////////
	TICKS_TYPE max_ticks() {
		return m_ticks + m_period;
	}
	////////////////////////////////////////
	double ticks_per_ms() {
		return m_ticks_per_ms;
	}
	////////////////////////////////////////
	void set_rate(V_CLOCK_IN_RATE clock_in_rate) {
		const byte rate[V_CLOCK_IN_RATE_MAX] = {PP24_16, PP24_8};
		m_clock_in_rate = clock_in_rate;
		m_period = pp24_to_ticks(rate[clock_in_rate]);
	}
	////////////////////////////////////////
	V_CLOCK_IN_RATE get_rate() {
		return m_clock_in_rate;
	}
	////////////////////////////////////////
	void on_pulse(uint32_t ms) {
		if(CLOCK_RUNNING == m_state) {
			// check we have not rolled over
			if(ms > m_last_ms) {
				uint32_t elapsed_ms = (ms - m_last_ms);
				m_ticks_per_ms = (double)m_period / elapsed_ms;
				m_timeout = ms + 4 * elapsed_ms;
			}
		}
		else {
			m_timeout = 0;
			m_state = CLOCK_RUNNING;
			fire_event(EV_SEQ_CONTINUE,0);
		}
		m_last_ms = ms;
		m_ticks += m_period;


	}
	////////////////////////////////////////
	void run(uint32_t ms) {
		if(CLOCK_RUNNING == m_state && !!m_timeout && ms > m_timeout) {
			m_timeout = 0;
			m_state = CLOCK_STOPPED;
			fire_event(EV_SEQ_STOP,0);
		}
	}
};
CPulseClockSource g_pulse_clock_in;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Clock source to handle an incoming midi clock
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CMidiClockSource 	: public IClockSource {
	TICKS_TYPE m_ticks; 	// tick counter incremented by m_period when external clock pulse is received
	double m_ticks_per_ms;  // calculated tick rate from ext clock
	uint32_t m_last_ms;		// used to time between incoming pulses
	int m_transport:1;		// whether we should act on MIDI transport messages
	enum : byte { PENDING_NONE, PENDING_START, PENDING_CONTINUE } m_pending_event;
	const TICKS_TYPE MIDI_CLOCK_RATE_TICKS = (1<<8);
public:
	///////////////////////////////////////////////////////////////////////////////
	CMidiClockSource() {
		m_transport = 1;
		m_pending_event = PENDING_NONE;
		m_ticks_per_ms = 0;
		m_last_ms = 0;
		m_ticks = 0;
	}
	///////////////////////////////////////////////////////////////////////////////
	void set_transport(byte transport) {
		m_transport = transport;
	}
	///////////////////////////////////////////////////////////////////////////////
	void event(int event, uint32_t param) {
		switch(event) {
		case EV_CLOCK_RESET:
			m_ticks_per_ms = 0;
			m_last_ms = 0;
			// fall thru
		case EV_SEQ_RESTART:
			m_ticks = 0;
			break;
		}
	}
	///////////////////////////////////////////////////////////////////////////////
	TICKS_TYPE min_ticks() {
		return m_ticks;
	};
	///////////////////////////////////////////////////////////////////////////////
	TICKS_TYPE max_ticks() {
		return m_ticks + MIDI_CLOCK_RATE_TICKS;
	};
	///////////////////////////////////////////////////////////////////////////////
	double ticks_per_ms() {
		return m_ticks_per_ms;
	};
	///////////////////////////////////////////////////////////////////////////////
	void on_midi_realtime(byte ch, uint32_t ms) {
		switch(ch) {
		case midi::MIDI_TICK:
			m_ticks += MIDI_CLOCK_RATE_TICKS;
			// per MIDI spec, we only act on START or CONTINUE message
			// at the time we receive the next tick
			if(m_pending_event != PENDING_NONE) {
				switch(m_pending_event) {
					case PENDING_START:
						fire_event(EV_SEQ_RESTART,0);
						break;
					case PENDING_CONTINUE:
						fire_event(EV_SEQ_CONTINUE,0);
						break;
				}
				m_pending_event = PENDING_NONE;
			}
			if(ms > m_last_ms) {
				uint32_t elapsed_ms = (ms - m_last_ms);
				m_ticks_per_ms = (double)MIDI_CLOCK_RATE_TICKS / elapsed_ms;
			}
			m_last_ms = ms;
			break;
		case midi::MIDI_START:
			if(m_transport) {
				m_pending_event = PENDING_START;
			}
			break;
		case midi::MIDI_CONTINUE:
			if(m_transport) {
				m_pending_event = PENDING_CONTINUE;
			}
			break;
		case midi::MIDI_STOP:
			if(m_transport) {
				m_pending_event = PENDING_NONE;
				fire_event(EV_SEQ_STOP,0);
			}
			break;
		}
	}
};
CMidiClockSource g_midi_clock_in;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Clock source to provide fixed clock info based on a specified BPM
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CFixedClockSource	: public IClockSource {
	double m_ticks_per_ms;
	float m_bpm;
public:
	////////////////////////////////////////
	CFixedClockSource() {
		set_bpm(120);
	}
	////////////////////////////////////////
	void event(int event, uint32_t param) {
	}
	////////////////////////////////////////
	TICKS_TYPE min_ticks() {
		return 0;
	}
	////////////////////////////////////////
	TICKS_TYPE max_ticks() {
		return TICKS_INFINITY;
	}
	////////////////////////////////////////
	double ticks_per_ms() {
		return m_ticks_per_ms;
	}
	////////////////////////////////////////
	void set_bpm(float bpm) {
		m_ticks_per_ms = (bpm * PP24_4 * 256) / (60.0 * 1000.0);
		m_bpm = bpm;
	}
	////////////////////////////////////////
	float get_bpm() {
		return m_bpm;
	}
};
CFixedClockSource g_fixed_clock;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// This class is used for managing the pulse clock output
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CPulseClockOut {

	V_CLOCK_OUT_RATE m_clock_out_rate;

	const int HIGH_MS = 15;		// duration of high part of pulse
	const int LOW_MS = 2;		// MIN low time between pulses

	enum { ST_IDLE, ST_HIGH, ST_LOW } m_state; // pulse state
	int m_timeout;				// time to remain in state
	int m_pulses;				// number of pulses remaining to send
	int m_period;
	byte m_running;
public:
	///////////////////////////////////////////////////////////////////////////////
	CPulseClockOut() {
		set_rate(V_CLOCK_OUT_RATE_16);
		m_running = 0;
	}
	///////////////////////////////////////////////////////////////////////////////
	void set_rate(V_CLOCK_OUT_RATE clock_out_rate) {
		const byte rate[V_CLOCK_OUT_RATE_MAX] = { PP24_16, PP24_8, PP24_16, PP24_8 };
		m_clock_out_rate = clock_out_rate;
		m_period = rate[clock_out_rate];
	}
	///////////////////////////////////////////////////////////////////////////////
	V_CLOCK_OUT_RATE get_rate() {
		return m_clock_out_rate;
	}
	void event(int event, uint32_t param) {
		switch(event) {
		case EV_CLOCK_RESET:
			m_state = ST_IDLE;
			m_timeout = 0;
			m_pulses = 0;
			m_running = 0;
			break;
		case EV_SEQ_RESTART:
			m_running = 1;
			break;
		case EV_SEQ_STOP:
			m_running = 0;
			break;
		case EV_SEQ_CONTINUE:
			m_running = 1;
			break;
		}
	}
	///////////////////////////////////////////////////////////////////////////////
	inline void on_pp24(int pp24) {
		// check if we need to send a clock out pulse
		ASSERT(m_period);
		switch(m_clock_out_rate) {
		case V_CLOCK_OUT_RATE_16_GATE:
		case V_CLOCK_OUT_RATE_8_GATE:
			if(!m_running) {
				break;
			}
			//fallthru
		case V_CLOCK_OUT_RATE_16:
		case V_CLOCK_OUT_RATE_8:
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
			break;
		}
	}
	///////////////////////////////////////////////////////////////////////////////
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
CPulseClockOut g_pulse_clock_out;


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// This class is used for managing the MIDI clock output
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CMidiClockOut {
	V_MIDI_CLOCK_OUT m_mode;
	byte m_is_running;
public:
	///////////////////////////////////////////////////////////////////////////////
	CMidiClockOut() {
		set_mode(V_MIDI_CLOCK_OUT_NONE);
		m_is_running = 0;
	}
	///////////////////////////////////////////////////////////////////////////////
	void set_mode(V_MIDI_CLOCK_OUT mode) {
		m_mode = mode;
	}
	///////////////////////////////////////////////////////////////////////////////
	V_MIDI_CLOCK_OUT get_mode() {
		return m_mode;
	}
	///////////////////////////////////////////////////////////////////////////////
	void event(int event, uint32_t param) {
		switch(event) {
		case EV_SEQ_RESTART:
			switch(m_mode) {
			case V_MIDI_CLOCK_OUT_ON_TRAN:
			case V_MIDI_CLOCK_OUT_GATE_TRAN:
				g_midi.send_byte(midi::MIDI_START);
				break;
			}
			m_is_running = 1;
			break;
		case EV_SEQ_STOP:
			switch(m_mode) {
			case V_MIDI_CLOCK_OUT_ON_TRAN:
			case V_MIDI_CLOCK_OUT_GATE_TRAN:
				g_midi.send_byte(midi::MIDI_STOP);
				break;
			}
			m_is_running = 0;
			break;
		case EV_SEQ_CONTINUE:
			switch(m_mode) {
			case V_MIDI_CLOCK_OUT_ON_TRAN:
			case V_MIDI_CLOCK_OUT_GATE_TRAN:
				g_midi.send_byte(midi::MIDI_CONTINUE);
				break;
			}
			m_is_running = 1;
			break;
		case EV_CLOCK_RESET:
			break;
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	inline void on_pp24() {
		switch(m_mode) {
		case V_MIDI_CLOCK_OUT_GATE:
		case V_MIDI_CLOCK_OUT_GATE_TRAN:
			if(!m_is_running) {
				break;
			}
			// else fall thru
		case V_MIDI_CLOCK_OUT_ON_TRAN:
		case V_MIDI_CLOCK_OUT_ON:
			g_midi.send_byte(midi::MIDI_TICK);
			break;
		}
	}
};
CMidiClockOut g_midi_clock_out;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// This class is used for blinking the beat LED
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CBeatLedOut {
public:
	inline void on_pp24(int pp24) {
		if(!(pp24%PP24_4)) {
			g_tempo_led.blink(g_tempo_led.MEDIUM_BLINK);
		}
	}

};
CBeatLedOut g_beat_led_out;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// This class implements the clock for the sequencer functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CClock {
	V_CLOCK_SRC m_source_mode;
	IClockSource *m_source;
	volatile byte m_ms_tick;					// flag set each time 1ms is up
	volatile uint32_t m_ms;					// ms counter
	volatile TICKS_TYPE m_ticks;
	volatile double m_ticks_remainder;
public:
	///////////////////////////////////////////////////////////////////////////////
	CClock() : m_source(&g_fixed_clock) {
		init_config();
		init_state();
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
	void init_state() {
		m_ms = 0;
		m_ms_tick = 0;
		m_ticks = 0;
		m_ticks_remainder = 0;
	}

	///////////////////////////////////////////////////////////////////////////////
	void init_config() {


	}

	///////////////////////////////////////////////////////////////////////////////
	void set(PARAM_ID param, int value) {
		switch(param) {
			case P_CLOCK_BPM:
				g_fixed_clock.set_bpm(value);
				break;
			case P_CLOCK_SRC:
				switch(value) {
				case V_CLOCK_SRC_EXTERNAL:
					m_source = &g_pulse_clock_in;
					break;
				case V_CLOCK_SRC_MIDI_CLOCK_ONLY:
					m_source = &g_midi_clock_in;
					g_midi_clock_in.set_transport(0);
					break;
				case V_CLOCK_SRC_MIDI_TRANSPORT:
					m_source = &g_midi_clock_in;
					g_midi_clock_in.set_transport(1);
					break;
				case V_CLOCK_SRC_INTERNAL:
				default:
					m_source = &g_fixed_clock;
					break;
				}
				m_source_mode = (V_CLOCK_SRC)value;
				fire_event(EV_CLOCK_RESET, 0);
				break;
			case P_CLOCK_IN_RATE:
				g_pulse_clock_in.set_rate((V_CLOCK_IN_RATE)value);
				fire_event(EV_CLOCK_RESET, 0);
				break;
			case P_CLOCK_OUT_RATE:
				g_pulse_clock_out.set_rate((V_CLOCK_OUT_RATE)value);
				fire_event(EV_CLOCK_RESET, 0);
				break;
			case P_MIDI_CLOCK_OUT:
				g_midi_clock_out.set_mode((V_MIDI_CLOCK_OUT)value);
				break;
		default:
			break;
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	int get(PARAM_ID param) {
		switch(param) {
		case P_CLOCK_BPM: return g_fixed_clock.get_bpm();
		case P_CLOCK_SRC: return m_source_mode;
		case P_CLOCK_IN_RATE: return g_pulse_clock_in.get_rate();
		case P_CLOCK_OUT_RATE: return g_pulse_clock_out.get_rate();
		case P_MIDI_CLOCK_OUT: return g_midi_clock_out.get_mode();
		default: return 0;
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	int is_valid_param(PARAM_ID param) {
		switch(param) {
		case P_CLOCK_BPM: return !!(m_source_mode == V_CLOCK_SRC_INTERNAL);
		case P_CLOCK_IN_RATE: return !!(m_source_mode == V_CLOCK_SRC_EXTERNAL);
		default: return 1;
		}
	}

	/*
	void restart() {
		m_ticks = 0;
		m_ticks_remainder = 0;
		g_pulse_clock_out.set_running(1);
		g_midi_clock_out.start();
		g_midi_clock_in.reset();
	}

	///////////////////////////////////////////////////////////////////////////////
	// When sequencer is stopped
	void stop() {
		g_pulse_clock_out.set_running(0);
		g_midi_clock_out.stop();

	}

	///////////////////////////////////////////////////////////////////////////////
	// When sequencer is started
	void cont() {
		g_pulse_clock_out.set_running(1);
		g_midi_clock_out.cont();

	}

	///////////////////////////////////////////////////////////////////////////////
	// This notfies us that
	void reset() {
		m_ticks = 0;
		m_ticks_remainder = 0;
		g_pulse_clock_out.reset();
		g_fixed_clock.reset();
		g_pulse_clock_in.reset();
		g_midi_clock_in.reset();
	}
*/
	///////////////////////////////////////////////////////////////////////////////
	// handle sequencer events
	void event(int event, uint32_t param) {
		g_fixed_clock.event(event, param);
		g_pulse_clock_in.event(event, param);
		g_pulse_clock_out.event(event, param);
		g_midi_clock_in.event(event, param);
		g_midi_clock_out.event(event, param);

		switch(event) {
		case EV_SEQ_STOP:
		case EV_SEQ_CONTINUE:
			break;
		case EV_CLOCK_RESET:
		case EV_SEQ_RESTART:
			m_ticks = 0;
			m_ticks_remainder = 0;
			break;
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// Return incrementing number of ticks (256 per 24PPQN period)
	inline TICKS_TYPE get_ticks() {
		return m_ticks;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Return incrementing number of ms
	inline uint32_t get_ms() {
		return m_ms;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Checks if its been 1ms since previous call
	inline byte is_ms_tick() {
		if(m_ms_tick) {
			m_ms_tick = 0;
			return 1;
		}
		return 0;
	}

	///////////////////////////////////////////////////////////////////////////////
	// utility function to wait a number of ms (blocking)
	void wait_ms(int ms) {
		while(ms) {
			m_ms_tick = 0;
			while(!m_ms_tick);
			--ms;
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// Based on current clock rate, convert PP24 to ms
	inline int get_ms_for_pp24(int pp24) {
		return (int)(pp24_to_ticks(pp24)/m_source->ticks_per_ms());
	}

	///////////////////////////////////////////////////////////////////////////////
	// Based on current clock rate, convert V_SQL_STEP_RATE to ms
	inline int get_ms_per_measure(V_SQL_STEP_RATE step_rate) {
		return get_ms_for_pp24(pp24_per_measure(step_rate));
	}

	///////////////////////////////////////////////////////////////////////////////
	// Method called approx once per ms
	void run() {
		g_pulse_clock_in.run(m_ms);
		g_pulse_clock_out.run();
	}

	///////////////////////////////////////////////////////////////////////////////
	// Interrupt service routine called exactly once per millisecond
	void per_ms_isr() {

		// maintain a millisecond counter for general
		// timing purposes
		++m_ms;

		// set a tick flag which is used for general
		// ting purposes
		m_ms_tick = 1;

		TICKS_TYPE prev_ticks = m_ticks;

		// update the tick counter used for scheduling the sequencer
		if(m_ticks < m_source->min_ticks()) {
			m_ticks = m_source->min_ticks();
			m_ticks_remainder = 0;
		}
		else {
			// count the appropriate number of ticks for this millisecond
			// but don't yet save the values
			double ticks_remainder = m_ticks_remainder + m_source->ticks_per_ms();
			int whole_ticks = (int)ticks_remainder;
			ticks_remainder -= whole_ticks;
			TICKS_TYPE ticks = m_ticks + whole_ticks;

			if(ticks < m_source->max_ticks()) {
				m_ticks = ticks;
				m_ticks_remainder = ticks_remainder;
			}
		}

		// check for a rollover into the next 24PPQN tick
		if((m_ticks ^ prev_ticks)&~0xFF) {
			int pp24 = m_ticks>>8;
			g_pulse_clock_out.on_pp24(pp24);
			g_midi_clock_out.on_pp24();
			g_beat_led_out.on_pp24(pp24);
		}

	}
	inline void ext_clock_isr() {
		g_pulse_clock_in.on_pulse(m_ms);
	}

};


}; // namespace

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// define the clock instance
clock::CClock g_clock;

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

// Global event notification so MIDI code can pass realtime messages over to the
// the clock code
void midi::handle_realtime(byte ch) {
	clock::g_midi_clock_in.on_midi_realtime(ch, g_clock.get_ms());
}

#endif // CLOCK_H_
