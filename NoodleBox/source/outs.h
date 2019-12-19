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
		MAX_CHAN = 4,
		I2C_BUF_SIZE = 100,
		TRIG_DURATION = 15,
		TRIG_DELAY_MS = 2
	};
	enum : long {
		SCALING = 0x10000L
	};

	// calibration values
	typedef struct {
		int scale[MAX_CHAN];
		int offset[MAX_CHAN];
	} CONFIG;
	CONFIG m_cfg;

	typedef struct {
		GATE_STATUS	gate_status;	// current state of the gate
		byte trig_delay;			// ms remaining before rising edge in trig state
		int pitch;					// 32-bit current pitch value (dac << 16)
		int target;  				// 32-bit current target value (dac << 16)
		int glide_rate;  			// glide rate applied per ms to the pitch
		byte cv_src;
		byte gate_src;
	} CHAN_STATE;
	CHAN_STATE m_chan[MAX_CHAN];

	/////////////////////////////////////////////////////////////////////////////////
	void impl_set_gate(byte which, byte state) {
		if(m_chan[0].gate_src == which) {
			if(state) {
				SET_GPIOA(BIT_GATE1);
			}
			else {
				CLR_GPIOA(BIT_GATE1);
			}
		}
		if(m_chan[1].gate_src == which) {
			if(state) {
				SET_GPIOA(BIT_GATE2);
			}
			else {
				CLR_GPIOA(BIT_GATE2);
			}
		}
		if(m_chan[2].gate_src == which) {
			if(state) {
				SET_GPIOA(BIT_GATE3);
			}
			else {
				CLR_GPIOA(BIT_GATE3);
			}
		}
		if(m_chan[3].gate_src == which) {
			if(state) {
				SET_GPIOA(BIT_GATE4);
			}
			else {
				CLR_GPIOA(BIT_GATE4);
			}
		}
	}

	/////////////////////////////////////////////////////////////////////////////////
	void impl_set_cv(byte which, uint16_t dac) {
		if(m_chan[0].cv_src == which) {
			g_i2c_dac.set(0, dac);
		}
		if(m_chan[1].cv_src == which) {
			g_i2c_dac.set(1, dac);
		}
		if(m_chan[2].cv_src == which) {
			g_i2c_dac.set(2, dac);
		}
		if(m_chan[3].cv_src == which) {
			g_i2c_dac.set(3, dac);
		}
	}

public:


	/////////////////////////////////////////////////////////////////////////////////
	COuts()
	{
		memset((byte*)m_chan,0,sizeof m_chan);
		for(int i=0; i<MAX_CHAN; ++i) {
			m_chan[i].cv_src = i;
			m_chan[i].gate_src = i;
		}
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
					impl_set_gate(which,0);
					m_chan[which].gate_status = GATE_CLOSED;
					break;
				case GATE_TRIG:
					if(m_chan[which].gate_status == GATE_OPEN) {
						// gate is open so need to generate a new rising edge
						impl_set_gate(which,0);
						m_chan[which].gate_status = GATE_TRIG;
						m_chan[which].trig_delay = TRIG_DELAY_MS;
						g_gate_led.blink(g_gate_led.MEDIUM_BLINK);
						break;
					}
					// else fall thru
				case GATE_OPEN:
					impl_set_gate(which,1);
					m_chan[which].gate_status = GATE_OPEN;
					g_gate_led.blink(g_gate_led.MEDIUM_BLINK);
					break;
			}
		}
	}

	/////////////////////////////////////////////////////////////////////////////////
	void close_all_gates() {
		for(int i=0; i<MAX_CHAN; ++i) {
			impl_set_gate(i,0);
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
			impl_set_cv(which,dac);
		}
	}

	/////////////////////////////////////////////////////////////////////////////////
	void test_dac(int which, int dac) {
		impl_set_cv(which,dac);
	}

	/////////////////////////////////////////////////////////////////////////////////
	// called once per ms
	void run() {

		// processing for each output
		for(int i=0; i<MAX_CHAN; ++i) {

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
				impl_set_cv(i,dac);
			}


			// handle pre-trig delay
			if(m_chan[i].gate_status == GATE_TRIG && m_chan[i].trig_delay) {
				if(!--m_chan[i].trig_delay) {
					m_chan[i].gate_status = GATE_OPEN;
					impl_set_gate(i,1);
				}
			}
		}
	}

	///////////////////////////////////////////////////
	void set_cv_alias(byte which, byte src) {
		ASSERT(out<MAX_CHAN);
		ASSERT(src<MAX_CHAN);
		m_chan[which].cv_src = src;
	}
	///////////////////////////////////////////////////
	void set_gate_alias(byte which, byte src) {
		ASSERT(out<MAX_CHAN);
		ASSERT(src<MAX_CHAN);
		m_chan[which].gate_src = src;
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
