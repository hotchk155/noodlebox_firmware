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
//  CV / GATE DRIVER
//
///////////////////////////////////////////////////////////////////////////////////

#ifndef OUTS_H_
#define OUTS_H_

//
// MACRO DEFS
//
#define PORTD_BIT_GATE4 4
#define PORTD_BIT_GATE2 3
#define PORTD_BIT_GATE3 2
#define PORTA_BIT_GATE1 6

#define BIT_GATE4		MK_GPIOA_BIT(PORTD_BASE, PORTD_BIT_GATE4)
#define BIT_GATE2		MK_GPIOA_BIT(PORTD_BASE, PORTD_BIT_GATE2)
#define BIT_GATE3		MK_GPIOA_BIT(PORTD_BASE, PORTD_BIT_GATE3)
#define BIT_GATE1		MK_GPIOA_BIT(PORTA_BASE, PORTA_BIT_GATE1)

// This type is used for passing CV information around. It represents
// a value in the 0-127 range of the sequencer data points. The top
// 16 bits are the whole part and lower 16 bits are fractional part
typedef int32_t CV_TYPE;

//
// GLOBAL DATA
//

// These definitions are used to initialise the port
CDigitalOut<kGPIO_PORTA, PORTA_BIT_GATE1> g_gate_1;
CDigitalOut<kGPIO_PORTD, PORTD_BIT_GATE2> g_gate_2;
CDigitalOut<kGPIO_PORTD, PORTD_BIT_GATE3> g_gate_3;
CDigitalOut<kGPIO_PORTD, PORTD_BIT_GATE4> g_gate_4;

/////////////////////////////////////////////////////////////////////////////////
//
// CLASS WRAPS UP CV AND GATE FUNCTIONS
//
/////////////////////////////////////////////////////////////////////////////////
class COuts {
public:
	typedef enum:byte  {
		GATE_CLOSED,
		GATE_OPEN,
		GATE_TRIG,
	} GATE_STATUS;
	enum {
		MAX_CV = 4,
		MAX_GATE = 4,
		I2C_BUF_SIZE = 100,
		TRIG_DURATION = 15,
		TRIG_DELAY_MS = 2
	};

	enum : long {
		SCALING = 0x10000L
	};

	// calibration values
	typedef struct {
		int scale[MAX_CV];
		int offset[MAX_CV];
	} CONFIG;
	CONFIG m_cfg;

	typedef struct {
		GATE_STATUS	gate_status;	// current state of the gate
		byte trig_delay;			// ms remaining before rising edge in trig state
		int pitch;					// 32-bit current pitch value (dac << 16)
		int target;  				// 32-bit current target value (dac << 16)
		int glide_rate;  			// glide rate applied per ms to the pitch
	} CHAN_STATE;
	CHAN_STATE m_chan[MAX_CV];

	/////////////////////////////////////////////////////////////////////////////////
	void impl_gate_on(byte which) {
		switch(which) {
		case 0: SET_GPIOA(BIT_GATE1); break;
		case 1: SET_GPIOA(BIT_GATE2); break;
		case 2: SET_GPIOA(BIT_GATE3); break;
		case 3: SET_GPIOA(BIT_GATE4); break;
		}
	}

	/////////////////////////////////////////////////////////////////////////////////
	void impl_gate_off(byte which) {
		switch(which) {
		case 0: CLR_GPIOA(BIT_GATE1); break;
		case 1: CLR_GPIOA(BIT_GATE2); break;
		case 2: CLR_GPIOA(BIT_GATE3); break;
		case 3: CLR_GPIOA(BIT_GATE4); break;
		}
	}

public:


	/////////////////////////////////////////////////////////////////////////////////
	COuts()
	{
		memset((byte*)m_chan,0,sizeof m_chan);
	}

	///////////////////////////////////////////////////////////////////////////////
	void event(int event, uint32_t param) {
		switch(event) {
		case EV_SEQ_STOP:
			close_all_gates();
			break;
		}
	}

	/////////////////////////////////////////////////////////////////////////////////
	void gate(byte which, GATE_STATUS gate) {
		m_chan[which].trig_delay = 0;
		if(m_chan[which].gate_status != gate) {
			switch(gate) {
				case GATE_CLOSED:
					impl_gate_off(which);
					m_chan[which].gate_status = GATE_CLOSED;
					break;
				case GATE_TRIG:
					if(m_chan[which].gate_status == GATE_OPEN) {
						// gate is open so need to generate a new rising edge
						impl_gate_off(which);
						m_chan[which].gate_status = GATE_TRIG;
						m_chan[which].trig_delay = TRIG_DELAY_MS;
						g_gate_led.blink(g_gate_led.MEDIUM_BLINK);
						break;
					}
					// else fall thru
				case GATE_OPEN:
					impl_gate_on(which);
					m_chan[which].gate_status = GATE_OPEN;
					g_gate_led.blink(g_gate_led.MEDIUM_BLINK);
					break;
			}
		}
	}

	/////////////////////////////////////////////////////////////////////////////////
	void close_all_gates() {
		for(int i=0; i<MAX_GATE; ++i) {
			impl_gate_off(i);
			m_chan[i].gate_status = GATE_CLOSED;
		}
	}

	/////////////////////////////////////////////////////////////////////////////////
	void cv(int which, CV_TYPE value, V_SQL_CVSCALE scaling, int glide_time) {

		int dac = 0;

		// negative CV value will always map to zero on the DAC
		if(value>0) {

			// CV value parameter is 32 bit signed, fixed precision, where lowest
			// 16 bits are a fractional part. When scaled to 12-bit DAC range the
			// lowest 8 bits have almost no effect and can be ignored. Downscaling
			// the CV value to have 8 bits of fractional part gives headroom to
			// complete calculations in 32 unsigned bits.
			uint32_t v2 = ((uint32_t)value)>>8;

			switch(scaling) {
			case V_SQL_CVSCALE_1_2VOCT:
				if(v2>20964) {
					// this is the highest value we can map. Catch anything higher
					// here to avoid overrun in the calculation below!
					dac = 4095;
				}
				else {
					dac = ((6<<8)+600*v2)/(12<<8);
				}
				break;
			case V_SQL_CVSCALE_HZVOLT:
				//TODO
				break;
			case V_SQL_CVSCALE_1VOCT:
			default:
				if(v2>25160) {
					// this is the highest value we can map. Catch anything higher
					// here to avoid overrun in the calculation below!
					dac = 4095;
				}
				else {
					dac = ((6<<8)+500*v2)/(12<<8);
				}
				break;
			}
		}

		if(glide_time) {
			m_chan[which].target = ((uint32_t)dac)<<16;
			m_chan[which].glide_rate = (m_chan[which].target - m_chan[which].pitch)/glide_time;
		}
		else {
			m_chan[which].pitch = ((uint32_t)dac)<<16;
			m_chan[which].glide_rate = 0;
			g_i2c_dac.set(which, dac);
		}
	}

	/////////////////////////////////////////////////////////////////////////////////
	void test_dac(int which, int dac) {
		g_i2c_dac.set(which, dac);
	}

	/////////////////////////////////////////////////////////////////////////////////
	// called once per ms
	void run() {

		// processing for each output
		for(int i=0; i<MAX_CV; ++i) {

			// handle pitch glide
			if(m_chan[i].glide_rate) {
				m_chan[i].pitch += m_chan[i].glide_rate;
				if(m_chan[i].glide_rate < 0) {
					if(m_chan[i].pitch <= m_chan[i].target) {
						m_chan[i].pitch = m_chan[i].target;
						m_chan[i].glide_rate = 0;
					}
				}
				else {
					if(m_chan[i].pitch >= m_chan[i].target) {
						m_chan[i].pitch = m_chan[i].target;
						m_chan[i].glide_rate = 0;
					}
				}
				int dac = m_chan[i].pitch>>16;
				g_i2c_dac.set(i, dac);
			}


			// handle pre-trig delay
			if(m_chan[i].gate_status == GATE_TRIG && m_chan[i].trig_delay) {
				if(!--m_chan[i].trig_delay) {
					m_chan[i].gate_status = GATE_OPEN;
					impl_gate_on(i);
				}
			}
		}
	}
	///////////////////////////////////////////////////
	static int get_cfg_size() {
		return sizeof(m_cfg);
	}
	///////////////////////////////////////////////////
	void get_cfg(byte **dest) {
		*((CONFIG*)*dest) = m_cfg;
		(*dest)+=get_cfg_size();
	}
	///////////////////////////////////////////////////
	void set_cfg(byte **src) {
		m_cfg = *((CONFIG*)*src);
		(*src)+=get_cfg_size();
	}
};

// define global instance of the CV/Gate controller
COuts g_outs;

#endif /* OUTS_H_ */
