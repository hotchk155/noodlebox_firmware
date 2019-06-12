/*
 * patch.h
 *
 *  Created on: 14 Mar 2018
 *      Author: jason
 */

#ifndef STORAGE_H_
#define STORAGE_H_

class CStorage {
	enum {
		BUFFER_SIZE = 512,
		BLOCK_SIZE = 64,
		SLOT_SIZE = 512
	};

	byte m_buf[BUFFER_SIZE];
	volatile CI2CBus::TRANSACTION m_txn;
	int m_data_index;
	int m_data_len;
	byte m_last_ms;
	byte m_nak_retry;

	enum {
		ST_IDLE,
		ST_TX_PENDING,
		ST_TX_IN_PROG,
		ST_TX_WAIT_CYCLE,
		ST_RX_PENDING,
		ST_RX_IN_PROG
	};
	byte m_state;
public:
	CStorage() :
		m_txn{0},
		m_state(ST_IDLE)
	{}

	byte save_patch(byte slot) {
		if(m_state != ST_IDLE) {
			return 0;
		}

		m_buf[0] = 'H';
		m_buf[1] = 'E';
		m_buf[2] = 'L';
		m_buf[3] = 'L';
		m_buf[4] = 'O';
		m_data_index = 0;			// index within transmit buffer of single block of data being sent
		m_data_len = SLOT_SIZE;

		m_txn.addr = I2C_ADDR_EEPROM;
		m_txn.location = slot * SLOT_SIZE;
		m_txn.location_size = 2;
		m_txn.data = m_buf;
		m_txn.data_len = BLOCK_SIZE;
		m_txn.status = -1;
		m_txn.pending = 0;
		m_state = ST_TX_PENDING;
		return 1;
	}

	byte load_patch(byte slot) {
		if(m_state != ST_IDLE) {
			return 0;
		}
		m_buf[0] = 'X';
		m_buf[1] = 'X';
		m_buf[2] = 'X';
		m_buf[3] = 'X';
		m_buf[4] = 'X';

		m_data_index = 0;
		m_data_len = SLOT_SIZE;

		m_txn.addr = I2C_ADDR_EEPROM;
		m_txn.location = slot * SLOT_SIZE;
		m_txn.location_size = 2;
		m_txn.data = m_buf;
		m_txn.data_len = BLOCK_SIZE;
		m_txn.status = -1;
		m_txn.pending = 0;
		m_state = ST_RX_PENDING;
		return 1;
	}

	void run_i2c() {
		if(g_i2c_bus.busy()) {
			// cannot do anything till the bus is free
			return;
		}
		switch(m_state) {
			case ST_TX_PENDING:
				g_i2c_bus.transmit(&m_txn); // initial parameters already prepared
				m_state = ST_TX_IN_PROG;
				break;
			case ST_TX_IN_PROG:
				if(!m_txn.pending) { 						// current transaction finished
					if(m_txn.status == kStatus_Success) {
						m_data_index += BLOCK_SIZE; 			// move to next block
						if(m_data_index < m_data_len) { 		// any more data to send
							m_txn.data = &m_buf[m_data_index];
							m_txn.data_len = BLOCK_SIZE; 		// next block cued up
							m_txn.location += BLOCK_SIZE; 		// new destination location
							m_txn.status = -1;
							m_nak_retry = 30;
							g_i2c_bus.transmit(&m_txn);			// kick off transmit
						}
						else {
							m_state = ST_IDLE;					// all done
						}
					}
					else if(m_txn.status == kStatus_I2C_Addr_Nak && m_nak_retry) {
						m_last_ms = (byte)g_clock.m_ms;
						--m_nak_retry;
						m_state = ST_TX_WAIT_CYCLE;
					}
					else {
//TODO
						m_state = ST_IDLE;					// error
					}

				}
				break;
			case ST_TX_WAIT_CYCLE:
				if((byte)g_clock.m_ms != m_last_ms) {
					m_state = ST_TX_IN_PROG;
				}
				break;
			case ST_RX_PENDING:
				g_i2c_bus.receive(&m_txn); // initial parameters already prepared
				m_state = ST_RX_IN_PROG;
				break;
			case ST_RX_IN_PROG:
				if(!m_txn.pending) { 						// current transaction finished
					m_data_index += BLOCK_SIZE; 			// move to next block
					if(m_data_index < m_data_len) { 		// any more data to send
						m_txn.data = &m_buf[m_data_index];
						m_txn.data_len = BLOCK_SIZE; 		// next block cued up
						m_txn.location += BLOCK_SIZE; 		// new source location
						g_i2c_bus.receive(&m_txn);			// kick off receive
					}
					else {
						m_state = ST_IDLE;					// all done
					}
				}
				break;
		}
	}


/*
	void test() {
		byte r[200];
		status_t p = 0;
		g_i2c_bus.wait();
		//p = g_i2c_bus.write_eeprom(10, qq, sizeof qq);
		p = g_i2c_bus.read_eeprom(10, r, sizeof r);
		g_i2c_bus.wait();
		p=p;
	}*/
};
CStorage g_storage;

#endif /* STORAGE_H_ */
