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
// CV AND GATE OUTPUT MANAGER
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
#ifndef OUTS_H_
#define OUTS_H_

//
// MACRO DEFS
//
#if NB_PROTOTYPE
	#define BIT_GATE1		MK_GPIOA_BIT(PORTA_BASE, 6)
	#define BIT_GATE2		MK_GPIOA_BIT(PORTD_BASE, 3)	// gate 2 on different pin
	#define BIT_GATE3		MK_GPIOA_BIT(PORTD_BASE, 2)
	#define BIT_GATE4		MK_GPIOA_BIT(PORTD_BASE, 4)
	CDigitalOut g_gate_1(kGPIO_PORTA, 6);
	CDigitalOut g_gate_2(kGPIO_PORTD, 3);
	CDigitalOut g_gate_3(kGPIO_PORTD, 2);
	CDigitalOut g_gate_4(kGPIO_PORTD, 4);
#else
	#define BIT_GATE1		MK_GPIOA_BIT(PORTA_BASE, 6)
	#define BIT_GATE2		MK_GPIOA_BIT(PORTA_BASE, 7)
	#define BIT_GATE3		MK_GPIOA_BIT(PORTD_BASE, 2)
	#define BIT_GATE4		MK_GPIOA_BIT(PORTD_BASE, 4)
	CDigitalOut g_gate_1(kGPIO_PORTA, 6);
	CDigitalOut g_gate_2(kGPIO_PORTD, 7);
	CDigitalOut g_gate_3(kGPIO_PORTD, 2);
	CDigitalOut g_gate_4(kGPIO_PORTD, 4);
#endif



// This type is used for passing CV information around. It represents
// a value in the 0-127 range of the sequencer data points. The top
// 16 bits are the whole part and lower 16 bits are fractional part
typedef int32_t CV_TYPE;

//
// GLOBAL DATA
//


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
		TRIG_DURATION = 5,
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
		//byte cv_src;
		//byte gate_src;
	} CHAN_STATE;
	CHAN_STATE m_chan[MAX_CHAN];

	/////////////////////////////////////////////////////////////////////////////////
	void impl_set_gate(byte which, byte state) {
		switch(which) {
		case 0:
			if(state) {
				SET_GPIOA(BIT_GATE1);
			}
			else {
				CLR_GPIOA(BIT_GATE1);
			}
			break;
		case 1:
			if(state) {
				SET_GPIOA(BIT_GATE2);
			}
			else {
				CLR_GPIOA(BIT_GATE2);
			}
			break;
		case 2:
			if(state) {
				SET_GPIOA(BIT_GATE3);
			}
			else {
				CLR_GPIOA(BIT_GATE3);
			}
			break;
		case 3:
			if(state) {
				SET_GPIOA(BIT_GATE4);
			}
			else {
				CLR_GPIOA(BIT_GATE4);
			}
			break;
		}
	}

	/////////////////////////////////////////////////////////////////////////////////
	void impl_set_cv(byte which, int dac) {
		int this_dac = (dac * (4096 + m_cfg.scale[which]))/4096 + m_cfg.offset[which];
		if(this_dac < 0) {
			this_dac = 0;
		}
		if(this_dac > 4095) {
			this_dac = 4095;
		}
		g_i2c_dac.set(which, this_dac);
	}

	/////////////////////////////////////////////////////////////////////////////////
	int calc_hzvolt(CV_TYPE value) {

		int bend = value & 0xFF;
		value >>= 8;

		// use a hard coded lookup table to get the
		// the DAC value for note in top octave
		int dac;
		if(value == 72) {
			dac = 4000;	// we can just about manage a C6!
		} else switch((byte)value % 12) {
			case 0: dac = 2000; break;
			case 1: dac = 2119; break;
			case 2: dac = 2245; break;
			case 3: dac = 2378; break;
			case 4: dac = 2520; break;
			case 5: dac = 2670; break;
			case 6: dac = 2828; break;
			case 7: dac = 2997; break;
			case 8: dac = 3175; break;
			case 9: dac = 3364; break;
			case 10: dac = 3564; break;
			case 11: dac = 3775; break;
		}

		/*
			next dac note will be dac*(2^(1/12))
			approximately equal to dac * (1 + 244/(16*256))
			pitch_bend contains 256 * fractional note (signed)
			so dac offset for fractional bend =
				dac * (pitch_bend/256)*(244/(16*256))

				= (dac * 244 * pitch_bend) / (16*256*256)
				= (dac * 244 * pitch_bend) / 1048576 		(0x100000L)


			NB: this results in linear interpolation between the notes,
			which is a bad approximation of a smooth bend since the
			mapping of V to Hz is logarithmic not linear!

			It'll have to do for now...
		*/
		dac += ((dac*244*bend)/0x100000L);

		// transpose to the requested octave by
		// right shifting
		byte octave = ((byte)value)/12;
		if(octave > 5) octave = 5;
		dac >>= (5-octave);

		return dac;
	}

public:


	/////////////////////////////////////////////////////////////////////////////////
	COuts()
	{
		memset((byte*)m_chan,0,sizeof m_chan);
		init_config();
	}

	///////////////////////////////////////////////////////////////////////////////
	void init_config() {
		memset((byte*)&m_cfg,0,sizeof m_cfg);
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
				dac = calc_hzvolt(v2);
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
	void test_dac(int which, int volts) {
		impl_set_cv(which,500 * volts);
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

/*
	///////////////////////////////////////////////////
	void set_cv_alias(byte which, byte src) {
		ASSERT(which<MAX_CHAN);
		ASSERT(src<MAX_CHAN);
		m_chan[which].cv_src = src;
	}
	///////////////////////////////////////////////////
	void set_gate_alias(byte which, byte src) {
		ASSERT(which<MAX_CHAN);
		ASSERT(src<MAX_CHAN);
		m_chan[which].gate_src = src;
	}
*/

	///////////////////////////////////////////////////
	void set_cal_scale(byte which, int value) {
		m_cfg.scale[which] = value;
	}
	///////////////////////////////////////////////////
	int get_cal_scale(byte which) {
		return m_cfg.scale[which];
	}
	///////////////////////////////////////////////////
	void set_cal_ofs(byte which, int value) {
		m_cfg.offset[which] = value;
	}
	///////////////////////////////////////////////////
	int get_cal_ofs(byte which) {
		return m_cfg.offset[which];
	}
	///////////////////////////////////////////////////
	static int get_cfg_size() {
		return sizeof(m_cfg);
	}
	///////////////////////////////////////////////////
	void get_cfg(byte **dest) {
		memcpy(*dest, &m_cfg, sizeof m_cfg);
		(*dest)+=get_cfg_size();
	}
	///////////////////////////////////////////////////
	void set_cfg(byte **src) {
		memcpy(&m_cfg, *src, sizeof m_cfg);
		(*src)+=get_cfg_size();
	}
};

// define global instance of the CV/Gate controller
COuts g_outs;

#endif /* OUTS_H_ */
