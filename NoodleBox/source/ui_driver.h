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
// DISPLAY AND KEYBOARD HANDLING
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
#ifndef UI_DRIVER_H_
#define UI_DRIVER_H_

//////////////////////////////////////////////////////////////////////////////
//
// MACRO DEFS
//
//////////////////////////////////////////////////////////////////////////////

// declare the GPIO bits used for fast access to data in port registers
#define BIT_KDAT		MK_GPIOA_BIT(PORTD_BASE, 5)
#define BIT_KCLK		MK_GPIOA_BIT(PORTC_BASE, 1)
#define BIT_ARCK		MK_GPIOA_BIT(PORTC_BASE, 0)
#define BIT_ADAT		MK_GPIOA_BIT(PORTB_BASE, 3)
#define BIT_ASCK		MK_GPIOA_BIT(PORTB_BASE, 2)
#define BIT_ENABLE		MK_GPIOA_BIT(PORTA_BASE, 1)
#define BIT_KEYSCAN1	MK_GPIOA_BIT(PORTC_BASE, 6)
#define BIT_KEYSCAN2	MK_GPIOA_BIT(PORTB_BASE, 4)
#define BIT_KEYSCAN3	MK_GPIOA_BIT(PORTD_BASE, 6)
#define BIT_ENCODER1	MK_GPIOA_BIT(PORTD_BASE, 0)
#define BIT_ENCODER2	MK_GPIOA_BIT(PORTD_BASE, 1)

// key scan bits for bottom buttons
#define KEY_B1	(1U<<1)
#define KEY_B2	(1U<<0)
#define KEY_B3	(1U<<2)
#define KEY_B4	(1U<<3)
#define KEY_B5	(1U<<4)
#define KEY_B6	(1U<<5)
#define KEY_B7	(1U<<6)
#define KEY_B8	(1U<<7)

// key scan bits for right side buttons
#define KEY_R1	(1U<<13)
#define KEY_R2	(1U<<14)
#define KEY_R3	(1U<<15)

// maximum key scan bit
#define KEY_MAXBIT	(1U<<23)

// define aliases for specific key bits based on their function
#define KEY_CV			KEY_B1
#define KEY_GATE		KEY_B2
#define KEY_CLONE		KEY_B3
#define KEY_CLEAR		KEY_B4
#define KEY_RAND		KEY_B5
#define KEY_LOOP		KEY_B6
#define KEY_PAGE		KEY_B7
#define KEY_LAYER		KEY_B8

#define KEY_MEMO		KEY_R1
#define KEY_RUN			KEY_R2
#define KEY_FUNC		KEY_R3


#define KEY2_CV_FINE			KEY_B2
#define KEY2_CV_SCROLL			KEY_B3
#define KEY2_CV_MOVE_HORZ		KEY_B4
#define KEY2_CV_MOVE_VERT		KEY_B5
#define KEY2_CV_AUTO_SCROLL		KEY_B7

#define KEY2_GATE_VEL 			KEY_B1
#define KEY2_GATE_HOLD 			KEY_B4
#define KEY2_GATE_PROB 			KEY_B5
#define KEY2_GATE_RETRIG 		KEY_B6
#define KEY2_GATE_REPLACE		KEY_B7
#define KEY2_GATE_SWING			KEY_B8

#define KEY2_CLONE_CV			KEY_B1
#define KEY2_CLONE_GATE			KEY_B2
#define KEY2_CLONE_PAGE			KEY_B7
#define KEY2_CLONE_LAYER		KEY_B8

#define KEY2_CLEAR_CV			KEY_B1
#define KEY2_CLEAR_GATE			KEY_B2
#define KEY2_CLEAR_PAGE			KEY_B7
#define KEY2_CLEAR_LAYER		KEY_B8

#define KEY2_RAND_SAVE_A		KEY_B1
#define KEY2_RAND_SAVE_B		KEY_B2
#define KEY2_RAND_SAVE_C		KEY_B3
#define KEY2_RAND_SAVE_D		KEY_B4
#define KEY2_RAND_CREATE		KEY_B6
#define KEY2_RAND_SAVE_CUR		KEY_B7


#define KEY2_LOOP_CUE_A			KEY_B1
#define KEY2_LOOP_CUE_B			KEY_B2
#define KEY2_LOOP_CUE_C			KEY_B3
#define KEY2_LOOP_CUE_D			KEY_B4
#define KEY2_LOOP_CUE_RANDOM	KEY_B5
#define KEY2_LOOP_CUE_FOREGROUND 	KEY_B7
#define KEY2_LOOP_CUE_ALL		KEY_B8

#define KEY2_PAGE_A				KEY_B1
#define KEY2_PAGE_B				KEY_B2
#define KEY2_PAGE_C				KEY_B3
#define KEY2_PAGE_D				KEY_B4
#define KEY2_PAGE_MOVE_LAYER	KEY_B8


#define KEY2_LAYER_1			KEY_B1
#define KEY2_LAYER_2			KEY_B2
#define KEY2_LAYER_3			KEY_B3
#define KEY2_LAYER_4			KEY_B4
#define KEY2_LAYER_MUTE			KEY_B7

#define KEY2_FUNC_SCALE_MODE	KEY_B1
#define KEY2_FUNC_AUTO_TRIG		KEY_B2
#define KEY2_FUNC_INTERPOLATE	KEY_B3
#define KEY2_FUNC_GRID			KEY_B4
#define KEY2_FUNC_REC_MODE		KEY_B5
#define KEY2_FUNC_LOOP_POINTS	KEY_B6
#define KEY2_FUNC_REC_ARM		KEY_B7

#define KEY2_MEMO_1				KEY_B1
#define KEY2_MEMO_2				KEY_B2
#define KEY2_MEMO_3				KEY_B3
#define KEY2_MEMO_4				KEY_B4
#define KEY2_MEMO_5				KEY_B5
#define KEY2_MEMO_6				KEY_B6
#define KEY2_MEMO_7				KEY_B7
#define KEY2_MEMO_8				KEY_B8
#define KEY2_MEMO_TEMPLATE		KEY_R3


// debounce times (ms)
#define DEBOUNCE_MS_PRESS 		50		// after a button is pressed
#define DEBOUNCE_MS_RELEASE		50		// after a button is released

// long press time (ms)
#define LONG_PRESS_TIME 		800

// size of the memory map for display buffer (uint32_t)
#define DISPLAY_BUF_SIZE 32

//////////////////////////////////////////////////////////////////////////////
//
// GLOBAL DATA DEFINITIONS
//
//////////////////////////////////////////////////////////////////////////////

// define the output pins used to drive the screen (also initialise the ports)
CDigitalOut g_pin_kdat(kGPIO_PORTD, 5);
CDigitalOut g_pin_kclk(kGPIO_PORTC, 1);
CDigitalOut g_pin_arck(kGPIO_PORTC, 0);
CDigitalOut g_pin_adat(kGPIO_PORTB, 3);
CDigitalOut g_pin_asck(kGPIO_PORTB, 2);
CDigitalOut g_pin_enable(kGPIO_PORTA, 1);

// define the input pins used to read from keyboard scan (also initialise the ports)
CDigitalIn g_pin_keyscan1(kGPIO_PORTC, 6);
CDigitalIn g_pin_keyscan2(kGPIO_PORTB, 4);

// define the input pins used to read from encode (also initialise the ports)
CDigitalIn g_pin_encoder1(kGPIO_PORTD, 0);
CDigitalIn g_pin_encoder2(kGPIO_PORTD, 1);

extern uint32_t g_render_buf[DISPLAY_BUF_SIZE];
extern volatile byte g_disp_update;

//////////////////////////////////////////////////////////////////////////////
//
// CLASS DEFINITION
//
//////////////////////////////////////////////////////////////////////////////

// This class wraps up the driver for the UI
class CUiDriver {

	typedef enum:byte {
		PHASE_NORMAL,		// layer 1 only
		PHASE_DIM,			// layer 2 only
		PHASE_BRIGHT		// layer 1 and layer 2 together
	} UPDATE_PHASE;


	// switch states are read during the display update cycle and values from each input
	// line are accumulated in these variables before being assembled into 32-bit value
	volatile uint16_t m_acc_key1 = 0;
	volatile uint16_t m_acc_key2 = 0;

	// stores the previous 32 bit key scan result. Two consecutive scans must yield the
	// same results for this to register. This filters line glitches
	volatile uint32_t m_last_acc_key_state = 0;

	// the filtered key status
	volatile uint32_t m_key_state = 0;

	// the previous filtered key status, used to detect changes in key status
	volatile uint32_t m_prev_key_state = ~0;


	volatile byte m_keys_pending = 0;
	volatile uint32_t m_disp_buf[32] = {0};
	volatile byte m_enc_state[3] = {0};
	volatile int m_enc_pos = 0;
	volatile int m_prev_enc_pos = 0;
	volatile int m_debounce = 0;
	volatile byte m_disp_update = 0;
	volatile uint32_t m_next_pit_period = 0;

	// The render buffer, contains two "layers". Elements 0-15 are layer 1
	// and elements 16-31 are layer 2
	// Refresh is done in 2 phases
	// Layer 1 time --- Layer 2 time
	uint32_t m_render_buf[DISPLAY_BUF_SIZE];

	uint32_t m_periodShort;
	uint32_t m_periodMedium;
	uint32_t m_periodLong;

	int m_cathode = 0;
	UPDATE_PHASE m_phase = PHASE_NORMAL;

	uint32_t m_shift = 0U;
	uint32_t m_key = 0U;
	int m_button_hold_timeout = 0;

public:
	enum {
		RASTER = 1,
		HILITE = 2,
	};

	//////////////////////////////////////////////////////////////////////////////////
	CUiDriver() {
		m_periodShort = (uint32_t) USEC_TO_COUNT(50, CLOCK_GetBusClkFreq());
		m_periodMedium = (uint32_t) USEC_TO_COUNT(300, CLOCK_GetBusClkFreq());
		m_periodLong = (uint32_t) USEC_TO_COUNT(400, CLOCK_GetBusClkFreq());
	}

	//////////////////////////////////////////////////////////////////////////////////
	void init() {
		g_pin_enable.set(1);
		m_next_pit_period = m_periodShort;
		EnableIRQ(PIT_CH1_IRQn);
		PIT_EnableInterrupts(PIT, kPIT_Chnl_1, kPIT_TimerInterruptEnable);
		PIT_SetTimerPeriod(PIT, kPIT_Chnl_1, m_next_pit_period);
		PIT_StartTimer(PIT, kPIT_Chnl_1);
	}


	////////////////////////////////////////////////////////////////////////////
	// DISPLAY UPDATE INTERRUPT SERVICE ROUTINE
	// The ISR is called sequentially 3 times for each of the cathode rows (each
	// of which has 32 bits of image data and 32 bits of brightness data)
	//
	// Call 1 - Load image (layer 1) data for this cathode row
	// ("medium" period before next PIT call to ISR)
	// Call 2 - Load brightness (layer 2) data for this cathode row
	// ("short" period before next PIT call to ISR)
	// Call 3 - Load logical AND of layer 1 and layer 2 data for this cathode row
	//          and prepare to move to the next cathode row
	// ("long" period before next PIT call to ISR)
	//
	inline void isr() {
		PIT_ClearStatusFlags(PIT, kPIT_Chnl_1, kPIT_TimerFlag);
		PIT_StopTimer(PIT, kPIT_Chnl_1);

		switch(m_phase) {

		////////////////////////////////////////////////////////////////////////////
		// NORMAL BRIGHTNESS PHASE
		case PHASE_NORMAL:

			if(!m_cathode) { // starting  a new refresh cycle
				// copy over updated render buffer if available
				if(m_disp_update) {
					memcpy((void*)m_disp_buf,(void*)m_render_buf,32*sizeof(uint32_t));
					m_disp_update = 0;
				}
				// clock bit into cathode shift reg
				SET_GPIOA(BIT_KDAT);
				CLR_GPIOA(BIT_KCLK);
				SET_GPIOA(BIT_KCLK);
				CLR_GPIOA(BIT_KDAT);
			}

			// populate the anode shift register with data from display layer 1
			for(int anode = 31; anode >=0; --anode) {
				int src_index= (m_cathode & 0xF8) + (anode & 0x07);
				int src_mask = ((m_cathode & 0x07) + (anode & 0xF8));
				if(m_disp_buf[src_index] & (0x80000000U >> src_mask)) {
					SET_GPIOA(BIT_ADAT);
				}
				else {
					CLR_GPIOA(BIT_ADAT);
				}
				CLR_GPIOA(BIT_ASCK);
				SET_GPIOA(BIT_ASCK);
			}

			SET_GPIOA(BIT_ENABLE);  // turn off the display
			CLR_GPIOA(BIT_KCLK); 	// clock cathode bit along one place..
			SET_GPIOA(BIT_KCLK);	// ..so we are addressing next anode row
			CLR_GPIOA(BIT_ARCK);	// anode shift register store clock pulse..
			SET_GPIOA(BIT_ARCK); 	// ..loads new data on to anode lines
			CLR_GPIOA(BIT_ENABLE);	// turn the display back on

			m_phase = PHASE_DIM;
			PIT_SetTimerPeriod(PIT, kPIT_Chnl_1, m_periodMedium);
			break;

		////////////////////////////////////////////////////////////////////////////
		// LOW BRIGHTNESS PHASE
		case PHASE_DIM:
			SET_GPIOA(BIT_ENABLE); // turn off the display
			for(int anode = 31; anode >=0; --anode) {
				int src_index= (m_cathode & 0xF8) + (anode & 0x07);
				int src_mask = ((m_cathode & 0x07) + (anode & 0xF8));
				if(m_disp_buf[16+src_index] & (0x80000000U >> src_mask)) {
					SET_GPIOA(BIT_ADAT);
				}
				else {
					CLR_GPIOA(BIT_ADAT);
				}
				CLR_GPIOA(BIT_ASCK);
				SET_GPIOA(BIT_ASCK);
			}
			CLR_GPIOA(BIT_ARCK);
			SET_GPIOA(BIT_ARCK);
			CLR_GPIOA(BIT_ENABLE); // turn on the display
			m_phase = PHASE_BRIGHT;
			PIT_SetTimerPeriod(PIT, kPIT_Chnl_1, m_periodShort);
			break;

		////////////////////////////////////////////////////////////////////////////
		// HIGH BRIGHTNESS PHASE
		case PHASE_BRIGHT:
			SET_GPIOA(BIT_ENABLE); // turn off the display
			for(int anode = 31; anode >=0; --anode) {
				int src_index= (m_cathode & 0xF8) + (anode & 0x07);
				int src_mask = ((m_cathode & 0x07) + (anode & 0xF8));
				if(m_disp_buf[src_index] & m_disp_buf[16+src_index] & (0x80000000U >> src_mask)) {
					SET_GPIOA(BIT_ADAT);
				}
				else {
					CLR_GPIOA(BIT_ADAT);
				}
				CLR_GPIOA(BIT_ASCK);
				SET_GPIOA(BIT_ASCK);
			}
			CLR_GPIOA(BIT_ARCK);
			SET_GPIOA(BIT_ARCK);
			CLR_GPIOA(BIT_ENABLE); // turn display back on again

			////////////////////////////////////////////////
			// READ ENCODER
			// get the state of the two encoder inputs into a 2 bit value
			byte new_state = 0;
			if(!(READ_GPIOA(BIT_ENCODER1))) {
				new_state |= 0b10;
			}
			if(!(READ_GPIOA(BIT_ENCODER2))) {
				new_state |= 0b01;
			}

			// make sure the state has changed and does not match
			// the previous state (which may indicate a bounce)
			if(new_state != m_enc_state[0] && new_state != m_enc_state[1]) {

				if(new_state == 0b11) {
					if( (m_enc_state[0] == 0b10) &&
						(m_enc_state[1] == 0b00) &&
						(m_enc_state[2] == 0b01)) {
						++m_enc_pos;
					}
					else if( (m_enc_state[0] == 0b01) &&
						(m_enc_state[1] == 0b00) &&
						(m_enc_state[2] == 0b10)) {
						--m_enc_pos;
					}
				}

				m_enc_state[2] = m_enc_state[1];
				m_enc_state[1] = m_enc_state[0];
				m_enc_state[0] = new_state;
			}

			////////////////////////////////////////////////
			// SCAN KEYBOARD ROW
			if(!READ_GPIOA(BIT_KEYSCAN1)) {
				m_acc_key1 |= (1U<<(m_cathode-8));
			}
			if(!READ_GPIOA(BIT_KEYSCAN2)) {
				m_acc_key2 |= (1U<<m_cathode);
			}
			//if(!READ_GPIOA(BIT_KEYSCAN3)) {
				//m_acc_key3 |= (1U<<m_cathode);
			//}

			// move along to next cathode bit - have we finished a scan?
			if(++m_cathode >= 16) {
				m_cathode = 0;

				// form the final 32 bit key state value
				uint32_t m_acc_key_state = ((uint32_t)m_acc_key2) | ((uint32_t)m_acc_key1);

				// ensure that the same status is read on two consecutive cycles
				// before it is acknowledged. This filters out read glitches
				if(m_acc_key_state == m_last_acc_key_state) {
					m_key_state = m_acc_key_state;
				}
				m_last_acc_key_state = m_acc_key_state;

				// zero the key state accumulators
				m_acc_key1 = 0;
				m_acc_key2 = 0;
			}

			m_phase = PHASE_NORMAL;
			PIT_SetTimerPeriod(PIT, kPIT_Chnl_1, m_periodLong);
			break;
		}

		PIT_StartTimer(PIT, kPIT_Chnl_1);
	}


	// combined keypress
	void key_down(uint32_t key) {

		// fire raw keydown event
		fire_event(EV_KEY_DOWN_RAW, key);

		// now process the key to generate key combination press event
		// and time long keypresses
		m_button_hold_timeout = 0;
		if(!m_key) {	// only valid if pressed when no other keys held
			if(m_shift) {
				m_key = key;
			}
			else {
				m_shift = key;
			}
			fire_event(EV_KEY_PRESS, m_shift|m_key);
			m_button_hold_timeout = LONG_PRESS_TIME;
		}
	}
	void key_up(uint32_t key) {
		if(key == m_shift) { // has the current shift key been released?
			if(m_key == 0 && m_button_hold_timeout) { // only counts as "click" if last key released
				fire_event(EV_KEY_CLICK, key);
			}
			fire_event(EV_KEY_RELEASE, key);
			m_key = 0;
			m_shift = 0;
		}
		else if(key == m_key) {
			if(m_button_hold_timeout) {
				fire_event(EV_KEY_CLICK, m_shift|m_key);
			}
			fire_event(EV_KEY_RELEASE, m_shift|m_key);
			m_key = 0;
		}
		m_button_hold_timeout = 0;
	}

	//////////////////////////////////////////////////////////////////////////////////
	byte is_key_down(uint32_t key) {
		return !!(m_key_state & key);
	}

	//////////////////////////////////////////////////////////////////////////////////
	int get_enc_movement() {
		int result = m_enc_pos - m_prev_enc_pos;
		m_prev_enc_pos = m_enc_pos;
		return result;
	}

	//////////////////////////////////////////////////////////////////////////////////
	void run()
	{
		int pos = m_enc_pos;
		if(pos != m_prev_enc_pos) {
			m_button_hold_timeout = 0;
			fire_event(EV_ENCODER, (uint32_t)(pos - m_prev_enc_pos));
			m_prev_enc_pos = pos;
		}

		if(m_debounce) {
			--m_debounce;
		}
		else {
			uint32_t keys = m_key_state;
			if(m_key_state != m_prev_key_state) {
				uint32_t bit = KEY_MAXBIT;
				uint32_t released = ~keys & m_prev_key_state;
				uint32_t pressed = keys & ~m_prev_key_state;
				while(bit) {
					if(pressed & bit) {
						key_down(bit);
						m_debounce = DEBOUNCE_MS_PRESS;
					}
					if(released & bit) {
						key_up(bit);
						m_debounce = DEBOUNCE_MS_RELEASE;
					}
					bit>>=1;
				}
				m_prev_key_state = keys;
			}
			else if(m_button_hold_timeout) {
				if(!--m_button_hold_timeout) {
					if(m_shift || m_key) {
						fire_event(EV_KEY_HOLD, m_shift|m_key);
					}
				}
			}
		}
	}


	//////////////////////////////////////////////////////////////////////////////////
	inline void lock_for_update() {
		m_disp_update = 0;
	}

	//////////////////////////////////////////////////////////////////////////////////
	inline void unlock_for_update() {
		m_disp_update = 1;
	}

	//////////////////////////////////////////////////////////////////////////////////
	inline void clear() {
		memset((byte*)m_render_buf, 0, sizeof m_render_buf);
	}

	//////////////////////////////////////////////////////////////////////////////////
	void bitmap(const uint32_t *src) {
		memcpy((byte*)m_render_buf, (byte*)src, 16*sizeof(uint32_t));
	}

	//////////////////////////////////////////////////////////////////////////////////
	inline uint32_t& raster(int index) {
		return m_render_buf[index];
	}

	//////////////////////////////////////////////////////////////////////////////////
	inline uint32_t& hilite(int index) {
		return m_render_buf[16+index];
	}

	//////////////////////////////////////////////////////////////////////////////////
	inline uint32_t bit(int index) {
		return 0x80000000U >> index;
	}

	//////////////////////////////////////////////////////////////////////////////////
	uint32_t make_mask(int from, int to) {
		uint32_t mask = 0x80000000U >> from;
		uint32_t result = 0;
		while(from++ < to) {
			result |= mask;
			mask >>= 1;
		}
		return result;
	}

	//////////////////////////////////////////////////////////////////////////////////
	void print_char(int ch, int col, int row, uint32_t *raster, uint32_t *hilite, int size) {

		switch(ch) {
		case '#': ch = CHAR4X5_HASH; break;
		case '?': ch = CHAR4X5_QUESTION; break;
		case ':': ch = CHAR4X5_COLON; break;
		case '-': ch = CHAR4X5_MINUS; break;
		case '+': ch = CHAR4X5_PLUS; break;
		case '~': ch = CHAR4X5_BLOCK; break;
		case '.': ch = CHAR4X5_DOT; break;
		case '>': ch = CHAR4X5_GT; break;
		case '$': ch = CHAR4X5_CROSS; break;
		case '=': ch = CHAR4X5_LIST; break;
		default:
			if(ch >= '0' && ch <= '9') {
				ch = CHAR4X5_NUMERIC + ch - '0';
			}
			else if(ch >= 'A' && ch <= 'Z') {
				ch = CHAR4X5_ALPHA + ch - 'A';
			}
			else if(ch >= 'a' && ch <= 'z') {
				ch = CHAR4X5_ALPHA + ch - 'a';
			}
			else {
				return;
			}
		}

		// 8 characters are stored in 5 x 32-bit words so
		// first word containing data for character ch is
		// at index 5 * (ch/8)
		const uint32_t *p = char4x5 + 5 * (ch/8);

		// to extract the data for the character from each of
		// these 5 words, we need to first right-shift the data
		// intobit positions 0-3 to be masked from the other chars
		// in these words
		int shift1 = (28 - 4*(ch%8));

		// then a second left or right shift is needed to get the
		// data into the required column to be masked into the
		// display buffer
		int shift2 = col - 28;

		for(int i=0; i<5 && row >= 0 && row < size; ++i) {

			// fetch data word
			uint32_t d = p[i];

			// extract the row of data for requested char
			d>>=shift1;
			d &= 0x0F;

			// shift to the display position
			if(shift2<0) {
				d<<=-shift2;
			}
			else {
				d>>=shift2;
			}

			if(raster) {
				raster[row] |= d;
			}
			if(hilite) {
				hilite[row] |= d;
			}
			++row;
		}
	}

	//////////////////////////////////////////////////////////////////////////////////
	void print_text(const char *sz, int col, int row, byte layer) {
		uint32_t *raster = (layer & RASTER)? m_render_buf : 0;
		uint32_t *hilite = (layer & HILITE)? &m_render_buf[16] : 0;
		while(*sz) {
			print_char(*sz, col, row, raster, hilite, 16);
			++sz;
			col+=4;
		}
	}

};

//////////////////////////////////////////////////////////////////////////////////
// define a single instance of the UI class
CUiDriver g_ui;

//////////////////////////////////////////////////////////////////////////////////
// The timer interrupt handler which refreshes the screen
extern "C" void PIT_CH1_IRQHandler(void) {
	g_ui.isr();
}

#endif /* UI_DRIVER_H_ */
