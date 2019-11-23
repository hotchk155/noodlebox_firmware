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
public:
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



	// Noodlebox uses the following type for handling "musical time"...
	// TICKS_TYPE is a 32 bit unsigned value where there are 256 * 24ppqn = 6144 LSB
	// increments per quarter note.
	// ~699050 beats before rolling over (about 38h @ 300bpm, 97h @ 120bpm)
	typedef uint32_t TICKS_TYPE;

	enum {
		NEVER = (TICKS_TYPE)(-1)
	};
	inline int ticks_to_pp24(TICKS_TYPE ticks) {
		return ticks>>8;
	}
	inline TICKS_TYPE pp24_to_ticks(int pp24) {
		return ((TICKS_TYPE)pp24)<<8;
	}

private:
	enum {
		CLOCK_TX_HIGH_MS = 15,
		CLOCK_TX_LOW_MS = 1
	};


	static const byte c_clock_in_rate[V_CLOCK_IN_RATE_MAX];
	static const byte c_clock_out_rate[V_CLOCK_OUT_RATE_MAX];


	/////////////////////////////////////////////////////////////////
	inline void clock_out(byte state) {
		g_clock_out.set(state);
	}
	/////////////////////////////////////////////////////////////////
	// called on a 24ppqn tick
	inline void on_pp24(int pp24) {
		++m_pp24_ticks;
		if(!(pp24%PP24_4)) {
			g_tempo_led.blink(g_tempo_led.MEDIUM_BLINK);
		}

		//g_midi.send_realtime(CMidi::MIDI_TICK);
		/*
		byte out_clock_div = c_clock_out_rate[m_cfg.m_clock_out_rate];
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
		}*/


	}
	///////////////////////////////////////////////////////////////////////////////
	// called when external clock tick received
	void on_clock_in(int period_pp24) {

		// stop the millisecond interrupts while we deal with the
		// external clock
		PIT_StopTimer(PIT, kPIT_Chnl_0);

		/////////////////////////////////////////////////////////////////////////
		// Maintain a calculation of the external clock rate
		// as number of fractional ticks per millisecond
		TICKS_TYPE period = pp24_to_ticks(period_pp24);
		if(m_clock_in_ms_last && m_ms > m_clock_in_ms_last) {

			// calculate how many ticks occur per ms at this input rate
			double ticks_per_ms = (double)period / (m_ms - m_clock_in_ms_last);
			m_ticks_per_ms = ticks_per_ms;
//			m_bpm = ((60.0 * 1000.0) * ticks_per_ms)/(PP24_4 * 256.0)

//TODO: lopass
		}
		m_clock_in_ms_last = m_ms;

		// calculate a timeout equal to two clock ticks in the future. If
		// we don't see the next tick by then we assume clock has stopped
		m_clock_in_timeout = m_ms + (2.0*period)/m_ticks_per_ms + 0.5;

		/////////////////////////////////////////////////////////////////////////
		// If the external clock has accelerated, we may not have had time to
		// process all the interpolated PP24 ticks before getting another
		// external tick. We will simply rush through them all now!

		// truncate to clo
		m_part_ticks  = 0;
		m_ticks &= ~0xFF;
		while(m_ticks < m_next_clock_in_ticks) {
			m_ticks+=0x100;
			on_pp24(ticks_to_pp24(m_ticks));
		}
		m_next_clock_in_ticks += period;


		PIT_StartTimer(PIT, kPIT_Chnl_0);


	}


public:



	volatile double m_ticks_per_ms;
	volatile double m_part_ticks;
	volatile TICKS_TYPE m_ticks;
	volatile byte m_ms_tick;					// flag set each time 1ms is up
	volatile int m_pp24_ticks;					// counter of how many PP24 ticks need to be notified
	volatile unsigned int m_ms;					// ms counter
	float m_bpm;								// tempo

	//int m_clock_tx_high_ms;	// duration of the high part of out pulse
	//int m_clock_tx_low_ms;	// duration of the low part of out pulse
	//int m_clock_tx_count;   // number of output pulses

	volatile uint32_t m_clock_in_ms_last;		// m_ms value of the last external clock in event
	volatile uint32_t m_clock_in_timeout;
	volatile TICKS_TYPE m_next_clock_in_ticks;	// the value of m_ticks at the next external clock tick



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
		m_ticks_per_ms = ((double)bpm * PP24_4 * 256.0) / (60.0 * 1000.0);
	}

	///////////////////////////////////////////////////////////////////////////////
	void on_restart() {
		m_ticks = 0;
		m_part_ticks = 0.0;
		m_pp24_ticks = 0;
		m_clock_in_ms_last = 0;
		m_next_clock_in_ticks = 0;
		m_clock_in_ms_last = 0;
		m_clock_in_timeout = 0;
		m_next_clock_in_ticks = 0;


//TODO: only for int clock?
		on_pp24(0);
	}

	///////////////////////////////////////////////////////////////////////////////
	void init_state() {
		m_ms = 0;
		m_ms_tick = 0;
		m_ticks_per_ms = 0.0;
		set_bpm(120);
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
	inline TICKS_TYPE get_ticks() {
		return m_ticks;
	}

	///////////////////////////////////////////////////////////////////////////////
	// This method returns the most recent whole 24PPQN tick count
	inline int get_pp24() {
		return ticks_to_pp24(m_ticks);
	}

	///////////////////////////////////////////////////////////////////////////////
	// This method returns 1 if a 24PPQN tick has happened since the previous call
	inline byte is_pp24_tick() {
		if(m_pp24_ticks) {
			--m_pp24_ticks;
			return 1;
		}
		return 0;
	}
	///////////////////////////////////////////////////////////////////////////////
	void on_midi_tick() {
		if(m_cfg.m_source == V_CLOCK_SRC_MIDI) {
			on_clock_in(PP24_24PPQN);
		}
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
		TICKS_TYPE ticks = pp24_to_ticks(pp24);
		return ticks/m_ticks_per_ms;
	}

	///////////////////////////////////////////////////////////////////////////////
	inline int get_ms_per_measure(V_SQL_STEP_RATE step_rate) {
		return get_ms_for_pp24(pp24_per_measure(step_rate));
	}



	///////////////////////////////////////////////////////////////////////////////
	/*
	void run() {
		if(m_clock_tx_high_ms) {
			if(!--m_clock_tx_high_ms) {
				clock_out(0);
			}
		}
		else if(m_clock_tx_low_ms) {
			--m_clock_tx_low_ms;
		}
		else if(m_clock_tx_count) {
			clock_out(1);
			--m_clock_tx_count;
			m_clock_tx_high_ms = CLOCK_TX_HIGH_MS;
			m_clock_tx_low_ms = CLOCK_TX_LOW_MS;

		}
	}
	*/



	///////////////////////////////////////////////////////////////////////////////
	// Interrupt service routine called once per millisecond
	void per_ms_isr() {

		// general timing stuff
		++m_ms;
		m_ms_tick = 1;

		// calculate the next tick count for this ms
		double part_ticks = m_part_ticks + m_ticks_per_ms;
		int whole_ticks = (int)part_ticks;
		part_ticks -= whole_ticks;
		TICKS_TYPE ticks = m_ticks + whole_ticks;

		// if we are running from the external clock, we want to make sure
		// that the interpolated clock does not run ahead of the time
		// at which the next external clock input is expected
		if(m_cfg.m_source == V_CLOCK_SRC_INTERNAL || ticks < m_next_clock_in_ticks) {

			int prev_pp24 = ticks_to_pp24(m_ticks);
			int pp24 = ticks_to_pp24(ticks);

			// commit the new clock
			m_ticks = ticks;
			m_part_ticks  = part_ticks;

			// see if there has been a 24ppqn tick
			if(pp24 != prev_pp24) {
				on_pp24(ticks_to_pp24(m_ticks));
			}
		}

		// assume the external clock has stopped when we do not
		// see an incoming tick within the allowed timeout
		if(m_clock_in_timeout && m_ms > m_clock_in_timeout) {
			m_clock_in_timeout = 0;
			m_clock_in_ms_last = 0;
		}
	}

	/*
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
//TODO - when counter is zero at start need a tick before incrementing it!
		}
	}
*/


	inline void ext_clock_isr() {
		if(m_cfg.m_source == V_CLOCK_SRC_EXTERNAL) {
			on_clock_in(c_clock_in_rate[m_cfg.m_clock_in_rate]);
		}
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
