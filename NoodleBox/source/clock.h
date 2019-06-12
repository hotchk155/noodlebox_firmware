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

/////////////////////////////////////////////////////////////////
// This class maintains a count of 24ppqn ticks based on the
// current BPM or the external MIDI clock. Internal clock is
// generated based on a once-per-millisecond interrupt
class CClock {

	enum {
		CLOCK_OUT_WIDTH = 15, //ms
		CLOCK_OUT_WIDTH_24PPQN = 5 //ms
	};
	/////////////////////////////////////////////////////////////////
	// called on a 24ppqn tick
	inline void private_on_tick() {
		static const byte c_out_clock_rate[V_CLOCK_OUT_RATE_MAX] = {
				  RATE_1,	RATE_2D,	RATE_2,		RATE_4D,	RATE_2T,
				  RATE_4,	RATE_8D,	RATE_4T,	RATE_8,		RATE_16D,
				  RATE_8T,  RATE_16,	RATE_16T,	RATE_32,   RATE_24PPQN
		};
		if(!m_beat_count) {
			g_tempo_led.blink(g_tempo_led.MEDIUM_BLINK);
		}
		if(++m_beat_count >= RATE_4) {
			m_beat_count = 0;
		}

		byte out_clock_div = c_out_clock_rate[m_cfg.m_clock_out_rate];
		if(!m_pulse_clock_count) {
			if(m_cfg.m_clock_out_rate == V_CLOCK_OUT_RATE_24PPQN) {
				g_clock_out.blink(CLOCK_OUT_WIDTH_24PPQN);
			}
			else {
				g_clock_out.blink(CLOCK_OUT_WIDTH);
			}
		}
		if(++m_pulse_clock_count >= out_clock_div) {
			m_pulse_clock_count = 0;
		}
	}


public:
	///////////////////////////////////////////////////////////////////////////////
	// called when external clock tick received
	void on_clock_in(int rate) {
		// use the time since the last tick to adjust the internal
		// clock speed
		if(m_clock_in_ms_last && m_ms > m_clock_in_ms_last) {
			m_ticks_per_ms = (double)rate / (m_ms - m_clock_in_ms_last);
		}
		m_clock_in_ms_last = m_ms;

		// update the tick counter
		m_clock_in_ticks_now = m_clock_in_ticks_next;
		m_clock_in_ticks_next += rate;
	}


	// Define different musical beat intervals based on
	// number of 24ppqn ticks
	enum
	{
	  RATE_1    	= 96,
	  RATE_2D   	= 72,
	  RATE_2    	= 48,
	  RATE_4D   	= 36,
	  RATE_2T   	= 32,
	  RATE_4    	= 24,
	  RATE_8D   	= 18,
	  RATE_4T   	= 16,
	  RATE_8    	= 12,
	  RATE_16D  	= 9,
	  RATE_8T   	= 8,
	  RATE_16   	= 6,
	  RATE_16T  	= 4,
	  RATE_32   	= 3,
	  RATE_24PPQN	= 1
	};

	// the actual tick counter, based on a 24ppqn tick and floating point part ticks
	volatile uint32_t m_ticks;
	volatile double m_part_tick;

	//int m_ext_clock_rate;		// the number of 24ppqn ticks represented by external clock event
	volatile uint32_t m_next_ext_tick;	// the number of ticks that have passed when next external clock event received

	float m_bpm;
	volatile double m_ticks_per_ms;
	volatile byte m_ms_tick;
	//uint32_t m_midi_ticks;
	volatile byte m_beat_count;
	volatile byte m_pulse_clock_count;
	//byte m_pulse_clock_div;
	volatile unsigned int m_ms;
	volatile uint32_t m_clock_in_ms_last;
	volatile uint32_t m_clock_in_ticks_now;
	volatile uint32_t m_clock_in_ticks_next;

	//volatile uint32_t m_ticks_at_next_ext_event;
	//volatile uint32_t m_ticks_absolute;

	//volatile uint32_t m_ticks_at_next_ext_event;

	///////////////////////////////////////////////////////////////////////////////
	// Config structure that defines info to store at power off
	typedef struct {
		V_CLOCK_SRC m_source;
		V_CLOCK_IN_RATE m_clock_in_rate;
		V_CLOCK_OUT_RATE m_clock_out_rate;
	} CONFIG;
	CONFIG m_cfg;

	///////////////////////////////////////////////////////////////////////////////
	CClock() {
		init_config();
		init_state();
	}

	///////////////////////////////////////////////////////////////////////////////
	void set_bpm(float bpm) {
		m_bpm = bpm;
		m_ticks_per_ms = ((double)bpm * RATE_4) / (60.0 * 1000.0);
	}

	///////////////////////////////////////////////////////////////////////////////
	void init_state() {
		m_ms = 0;
		m_ms_tick = 0;
		set_bpm(120);
		on_restart();
	}

	///////////////////////////////////////////////////////////////////////////////
	void init_config() {
		m_cfg.m_source = V_CLOCK_SRC_INTERNAL;
		m_cfg.m_clock_in_rate = V_CLOCK_IN_RATE_8;
		m_cfg.m_clock_out_rate = V_CLOCK_OUT_RATE_8;
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
				set_bpm(value);
				break;
			case P_CLOCK_SRC:
				m_cfg.m_source = (V_CLOCK_SRC)value;
				fire_event(EV_CLOCK_RESET, 0);
				break;
			case P_CLOCK_IN_RATE:
				m_cfg.m_clock_in_rate = (V_CLOCK_IN_RATE)value;
				fire_event(EV_CLOCK_RESET, 0);
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
		case P_CLOCK_BPM: return m_bpm;
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
	inline uint32_t get_ticks() {
		return m_ticks;
	}

	///////////////////////////////////////////////////////////////////////////////
	inline byte get_part_ticks() {
		if(m_cfg.m_source == V_CLOCK_SRC_INTERNAL) {
			return (byte)(256*m_part_tick);
		}
		else {
			//TODO..?
			return 0;
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	void on_midi_tick() {
		if(m_cfg.m_source == V_CLOCK_SRC_MIDI) {
			on_clock_in(RATE_24PPQN);
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	void on_restart() {
		m_ticks = 0;
		m_part_tick = 0.0;
		m_beat_count = 0;
		m_pulse_clock_count = 0;
		m_clock_in_ticks_now = 0;
		m_clock_in_ticks_next = 0;
		m_clock_in_ms_last = 0;


	}

	///////////////////////////////////////////////////////////////////////////////
	inline int ticks_per_measure(V_SQL_STEP_RATE step_rate) {
		const byte ticks[V_SQL_STEP_RATE_MAX] = {
		  RATE_1,
		  RATE_2D,
		  RATE_2,
		  RATE_4D,
		  RATE_2T,
		  RATE_4,
		  RATE_8D,
		  RATE_4T,
		  RATE_8,
		  RATE_16D,
		  RATE_8T,
		  RATE_16,
		  RATE_16T,
		  RATE_32
		};
		return ticks[step_rate];
	}

	///////////////////////////////////////////////////////////////////////////////
	int get_ms_for_ticks(int ticks) {
		return ticks/m_ticks_per_ms;
	}

	///////////////////////////////////////////////////////////////////////////////
	int get_ms_per_measure(V_SQL_STEP_RATE step_rate) {
		return ticks_per_measure(step_rate)/m_ticks_per_ms;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Interrupt service routine called once per millisecond
	void tick_isr() {

		++m_ms;
		// set flag to indicate that a milliecond has elapsed
		// this is used for general timing purposes
		m_ms_tick = 1;

		if(m_clock_in_ticks_next && m_ticks >= m_clock_in_ticks_next) {
			// hold the clock back until there is another
			// external tick event
		}
		else if(m_ticks < m_clock_in_ticks_now) {
			// catch the clock up
			while(m_ticks < m_clock_in_ticks_now) {
				private_on_tick();
				++m_ticks;
				m_part_tick = 0;
			}
		}
		else {
			// add the fractional number of ticks per millisecond to
			// the tick counter and see whether we now have at least
			// one complete tick
			m_part_tick += m_ticks_per_ms;
			int whole_tick = (int)m_part_tick;
			if(whole_tick) {
				m_ticks ++;
				m_part_tick -= whole_tick;
				private_on_tick();
			}
		}
	}

	inline void ext_clock_isr() {
		static const byte c_in_clock_rate[V_CLOCK_IN_RATE_MAX] = {
			RATE_16, RATE_8, RATE_4, RATE_24PPQN
		};
		if(m_cfg.m_source == V_CLOCK_SRC_EXTERNAL) {
			on_clock_in(c_in_clock_rate[m_cfg.m_clock_in_rate]);
		}
	}
};



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// define the clock instance
CClock g_clock;


// ISR for the millisecond timer
extern "C" void PIT_CH0_IRQHandler(void) {
	PIT_ClearStatusFlags(PIT, kPIT_Chnl_0, kPIT_TimerFlag);
	g_clock.tick_isr();
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
