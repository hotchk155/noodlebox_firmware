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
// MIDI HANDLING
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
#ifndef MIDI_H_
#define MIDI_H_

namespace midi {
extern void handle_realtime(byte ch);
extern void handle_note(byte ch, byte note, byte vel);
enum {
	RXBUF_SIZE = 64,
	RXBUF_SIZE_MASK = 0x3F,

	TXBUF_SIZE = 64,
	TXBUF_SIZE_MASK = 0x3F
};
enum {
	MIDI_TICK		   = 0xF8,
	MIDI_START		   = 0xFA,
	MIDI_CONTINUE	   = 0xFB,
	MIDI_STOP		   = 0xFC,
	MIDI_SYSEX_BEGIN   = 0xF0,
	MIDI_SYSEX_END     = 0xF7,
	MIDI_MTC_QTR_FRAME = 0xF1,
	MIDI_SPP 		   = 0xF2,
	MIDI_SONG_SELECT   = 0xF3
};

//////////////////////////////////////////////////////////////////////////////
class CMidi {

	// State flags used while receiving MIDI data
	byte m_midi_status;			// current MIDI message status (running status)
	byte m_midi_num_params;		// number of parameters needed by current MIDI message
	byte m_midi_params[2];		// parameter values of current MIDI message
	byte m_midi_param;			// number of params currently received
	byte m_in_sysex;				// whether in sysex

public:

	volatile byte m_rxbuf[RXBUF_SIZE];
	volatile byte m_rx_head;
	volatile byte m_rx_tail;

	volatile byte m_txbuf[TXBUF_SIZE];
	volatile byte m_tx_head;
	volatile byte m_tx_tail;


	////////////////////////////////////////////////////
	CMidi() {
		m_rx_head = 0;
		m_rx_tail = 0;
		m_tx_head = 0;
		m_tx_tail = 0;

		m_midi_status = 0;
		m_midi_num_params = 0;
		m_midi_param = 0;
		m_in_sysex = 0;

	}

	////////////////////////////////////////////////////
	// INITIALISE MIDI
	void init() {
	    uart_config_t config;
	    UART_GetDefaultConfig(&config);
	    config.baudRate_Bps = 31250;
	    config.enableTx = true;
	    config.enableRx = true;
	    UART_Init(UART0, &config, CLOCK_GetFreq(kCLOCK_BusClk));
	    UART_EnableInterrupts(UART0, kUART_RxDataRegFullInterruptEnable | kUART_RxOverrunInterruptEnable | kUART_TxDataRegEmptyInterruptEnable);
	    EnableIRQ(UART0_IRQn);
	}

	////////////////////////////////////////////////////
	// TRANSMIT A CHARACTER (BLOCKING)
	void send_byte(byte ch) {
		int tx_head = (m_tx_head+1)%TXBUF_SIZE_MASK;
		if(tx_head != m_tx_tail) {
			m_txbuf[m_tx_head] = ch;
			m_tx_head = tx_head;
		}
	    UART_EnableInterrupts(UART0, kUART_TxDataRegEmptyInterruptEnable);
	}

	////////////////////////////////////////////////////
	void send_cc(byte chan, byte cc, byte value) {
		send_byte(0xB0 | chan);
		send_byte(cc & 0x7F);
		send_byte(value & 0x7F);
	}

	////////////////////////////////////////////////////
	void start_note(byte chan, byte note, byte velocity) {
		send_byte(0x90 | chan);
		send_byte(note & 0x7F);
		send_byte(velocity & 0x7F);
	}

	////////////////////////////////////////////////////
	void stop_note(byte chan, byte note) {
		send_byte(0x90 | chan);
		send_byte(note & 0x7F);
		send_byte(0x00);
	}

	////////////////////////////////////////////////////
	void bend(byte chan, int amount) {
		amount += 8192;
		if(amount<0) {
			amount = 0;
		}
		else if(amount > 0x3FFF) {
			amount = 0x3FFF;
		}
		send_byte(0xe0 | chan);
		send_byte(amount&0x7F);
		send_byte((amount>>7)&0x7F);
	}

	////////////////////////////////////////////////////
	inline void irq_handler() {

		uint32_t flags = UART_GetStatusFlags(UART0);
	    // character received at UART
	    if (flags & (kUART_RxDataRegFullFlag | kUART_RxOverrunFlag))
	    {
	        byte data = UART_ReadByte(UART0); // clears the status flags
	        byte next = (m_rx_head+1)%RXBUF_SIZE_MASK;
			if(next != m_rx_tail) {
				m_rxbuf[m_rx_head] = data;
				m_rx_head = next;
				// TODO: flag overflow!
			}
	    }
	    if(flags & kUART_TxDataRegEmptyFlag) {
	    	if(m_tx_head != m_tx_tail) {
		        UART_WriteByte(UART0, m_txbuf[m_tx_tail]);
	    		m_tx_tail = (m_tx_tail+1)%TXBUF_SIZE_MASK;
	    	}
	    	else {
	    		UART_DisableInterrupts(UART0, kUART_TxDataRegEmptyInterruptEnable);
	    	}
	    }
	}

	/////////////////////////////////////////////////////////////////////////////////
	// once per ms
	void run() {
		while(m_rx_tail != m_rx_head) {
			byte ch = m_rxbuf[m_rx_tail];
			if((ch & 0xf0) == 0xf0) {
				switch(ch) {
				case MIDI_TICK:
				case MIDI_START:
				case MIDI_CONTINUE:
				case MIDI_STOP:
					handle_realtime(ch);
					break;
				case MIDI_MTC_QTR_FRAME:	// 1 param byte follows
				case MIDI_SONG_SELECT:		// 1 param byte follows
				case MIDI_SPP:				// 2 param bytes follow
					m_midi_param = 0;
					m_midi_status = ch;
					m_midi_num_params = (ch==MIDI_SPP)? 2:1;
					break;
				case MIDI_SYSEX_BEGIN:
					m_in_sysex = 1;
					break;
				case MIDI_SYSEX_END:
					m_in_sysex = 0;
					break;
				}
				// Ignoring....
				//  0xF4	RESERVED
				//  0xF5	RESERVED
				//  0xF6	TUNE REQUEST
				//	0xF9	RESERVED
				//	0xFD	RESERVED
				//	0xFE	ACTIVE SENSING
				//	0xFF	RESET
			}
			else if(!!(ch & 0x80))
			{
				// a status byte cancels sysex state
				m_in_sysex = 0;

				m_midi_param = 0;
				m_midi_status = ch;
				switch(ch & 0xF0)
				{
				case 0xC0: //  Patch change  1  instrument #
				case 0xD0: //  Channel Pressure  1  pressure
					m_midi_num_params = 1;
					break;
				case 0xA0: //  Polyphonic aftertouch  2  key  touch
				case 0x80: //  Note-off  2  key  velocity
				case 0x90: //  Note-on  2  key  veolcity
				case 0xB0: //  Continuous controller  2  controller #  controller value
				case 0xE0: //  Pitch bend  2  lsb (7 bits)  msb (7 bits)
				default:
					m_midi_num_params = 2;
					break;
				}
			}
			else if(!m_in_sysex)
			{
				if(m_midi_status)
				{
					// gathering parameters
					m_midi_params[m_midi_param++] = ch;
					if(m_midi_param >= m_midi_num_params)
					{
						// we have a complete message.. is it one we care about?
						m_midi_param = 0;
						switch(m_midi_status&0xF0)
						{
						case 0x80: // note off
							handle_note(m_midi_status&0x0F, m_midi_params[0], 0);
							break;
						case 0x90: // note on
							handle_note(m_midi_status&0x0F, m_midi_params[0], m_midi_params[1]);
							break;
						}
					}
				}
			}
			m_rx_tail = (m_rx_tail+1)%RXBUF_SIZE_MASK;
		}
	}
};

}; // namespace

midi::CMidi g_midi;
extern "C" void UART0_IRQHandler(void)
{
	g_midi.irq_handler();
}

#endif /* MIDI_H_ */
