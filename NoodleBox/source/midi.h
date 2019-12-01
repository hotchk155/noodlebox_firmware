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
//  MIDI HANDLING
//
///////////////////////////////////////////////////////////////////////////////////

//TODO: transmit buffering
#ifndef MIDI_H_
#define MIDI_H_

namespace midi {
extern void handle_realtime(byte ch);
enum {
	RXBUF_SIZE = 64,
	RXBUF_SIZE_MASK = 0x3F,

	TXBUF_SIZE = 64,
	TXBUF_SIZE_MASK = 0x3F
};
enum {
	MIDI_TICK		= 0xF8,
	MIDI_START		= 0xFA,
	MIDI_CONTINUE	= 0xFB,
	MIDI_STOP		= 0xFC
};

class CMidi {

public:

	volatile byte m_rxbuf[RXBUF_SIZE];
	volatile byte m_rx_head;
	volatile byte m_rx_tail;

	volatile byte m_txbuf[TXBUF_SIZE];
	volatile byte m_tx_head;
	volatile byte m_tx_tail;


	CMidi() {
		m_rx_head = 0;
		m_rx_tail = 0;
		m_tx_head = 0;
		m_tx_tail = 0;
	}

	/////////////////////////////////////////////////////////////////////////////////
	// INITIALISE MIDI
	void init() {
	    uart_config_t config;
	    UART_GetDefaultConfig(&config);
	    config.baudRate_Bps = 31250;
	    config.enableTx = true;
	    config.enableRx = true;
	    UART_Init(UART0, &config, CLOCK_GetFreq(kCLOCK_BusClk));
	    UART_EnableInterrupts(UART0, kUART_RxDataRegFullInterruptEnable | kUART_RxOverrunInterruptEnable | kUART_TxDataRegEmptyInterruptEnable);
	    //UART_EnableInterrupts(UART0, kUART_RxDataRegFullInterruptEnable | kUART_RxOverrunInterruptEnable | kUART_TransmissionCompleteInterruptEnable);
	    EnableIRQ(UART0_IRQn);
	}



	/////////////////////////////////////////////////////////////////////////////////
	// TRANSMIT A CHARACTER (BLOCKING)
	void send_byte(byte ch) {
		int tx_head = (m_tx_head+1)%TXBUF_SIZE_MASK;
		if(tx_head != m_tx_tail) {
			m_txbuf[m_tx_head] = ch;
			m_tx_head = tx_head;
		}
	    UART_EnableInterrupts(UART0, kUART_TxDataRegEmptyInterruptEnable);
	}
	void send_cc(byte chan, byte cc, byte value) {
		send_byte(0xB0 | chan);
		send_byte(cc & 0x7F);
		send_byte(value & 0x7F);
	}
	void start_note(byte chan, byte note, byte velocity) {
		send_byte(0x90 | chan);
		send_byte(note & 0x7F);
		send_byte(velocity & 0x7F);
	}
	void stop_note(byte chan, byte note) {
		send_byte(0x90 | chan);
		send_byte(note & 0x7F);
		send_byte(0x00);
	}
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
	    	if(flags & kUART_TxDataRegEmptyFlag) {
	    		UART_DisableInterrupts(UART0, kUART_TxDataRegEmptyInterruptEnable);
	    	}
	    	if(m_tx_head != m_tx_tail) {
		        UART_WriteByte(UART0, m_txbuf[m_tx_tail]);
	    		m_tx_tail = (m_tx_tail+1)%TXBUF_SIZE_MASK;
	    	}
	    }
	}

	/////////////////////////////////////////////////////////////////////////////////
	// once per ms
	void run() {
		while(m_rx_tail != m_rx_head) {
			byte ch = m_rxbuf[m_rx_tail];
			switch(ch) {
			case MIDI_TICK:
			case MIDI_START:
			case MIDI_CONTINUE:
			case MIDI_STOP:
				handle_realtime(ch);
				break;
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
