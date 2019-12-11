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
//  PATCH STORAGE HELPER
// 	USING M24256 EEPROM 256kbit = 32kB
//
///////////////////////////////////////////////////////////////////////////////////

#if 0
//#ifndef STORAGE_H_
#define STORAGE_H_


//
class CStorage {
public:
	enum {
		BUFFER_SIZE = 2048	// this must be large enough to contain the largest object we'll store in EEPROM and must be a whole number multiple of PAGE_SIZE
	};
private:
	enum {
		PAGE_SIZE = 64, // The number of bytes in EEPROM page, which must be on 64 byte boundary
	};

	// The actual data buffer where the entire patch is stored while being read or written
	// over multiple I2C transactions that transfer a single page of data
	volatile byte m_buf[BUFFER_SIZE];

	// the total length of data being transferred
	int m_buf_len;
	int m_buf_index;
	byte m_last_ms;
	byte m_nak_retry;

	// this is the transaction object that is populated and passed to the I2C bus manager
	// to send a single page over to the EEPROM
	CI2CBus::TRANSACTION m_txn;

	enum {
		ST_IDLE,
		ST_TX_IN_PROG,
		ST_TX_WAIT_CYCLE
//		ST_RX_IN_PROG
	};
	byte m_state;
public:
	CStorage() :
		m_txn{0},
		m_state(ST_IDLE)
	{}

	///////////////////////////////////////////////////////////////////////////////////
	// Kick off a write transaction.
	byte init_write(int address, byte *data, int len) {
		DBGLOG0("init_write");
		ASSERT(len < BUFFER_SIZE);
		if(m_state != ST_IDLE) {			// make sure the storage helper is not busy
			DBGLOG0("not idle");
			return 0;
		}
		memcpy((void*)m_buf, data, len);	// copy the data
		m_buf_len = len;
		m_buf_index = 0;

		int block_size;
		if(address%PAGE_SIZE) {
			// address is within a page, so maximum block size is the
			// remainder of the current page
			block_size = 64*(1+address/64) - address;
		}
		else {
			// address is aligned to page boundary
			block_size = PAGE_SIZE;
		}

		// may be less bytes to send than max allowable size
		if(block_size > len) {
			block_size = len;
		}

		// init the transaction object to send the first page of data
		m_txn.addr = I2C_ADDR_EEPROM;
		m_txn.location = address;
		m_txn.location_size = 2;
		m_txn.data = m_buf;
		m_txn.data_len = block_size;
		m_txn.status = -1;
		m_txn.pending = 1;
		g_i2c_bus.transmit(&m_txn);
		m_state = ST_TX_IN_PROG;
		return 1;
	}

	///////////////////////////////////////////////////////////////////////////////////
	// Synchronous write transaction
	byte write(int address, byte *data, int len) {
		DBGLOG0("write");

		// wait for the i2c bus manager to become free
		while(g_i2c_bus.busy());

		// kick off the write
		if(!init_write(address,data,len)) {
			return 0;
		}

		// wait for the whole write process to complete
		while(m_state != ST_IDLE) {
			run_i2c();
		}
		DBGLOG0("write: ok");
		return (m_txn.status == kStatus_Success);
	}

	///////////////////////////////////////////////////////////////////////////////////
	// read
	byte read(int address, byte *data, int len) {
		DBGLOG0("init read");
		ASSERT(len <= BUFFER_SIZE);
		if(m_state != ST_IDLE) {	// make sure we're not already busy
			DBGLOG0("not idle");
			return 0;
		}

		while(g_i2c_bus.busy());

		// prepare the transaction object to read the first page of data
		m_txn.addr = I2C_ADDR_EEPROM;
		m_txn.location = address;
		m_txn.location_size = 2;
		m_txn.data = m_buf;
		m_txn.data_len = len;
		m_txn.status = -1;
		m_txn.pending = 1;

		// kick off the receive
		g_i2c_bus.receive(&m_txn);
		while(g_i2c_bus.busy());
		if(m_txn.status == kStatus_Success) {
			memcpy(data, (void*)m_buf, len);
			return 1;
		}
		else {
			return 0;
		}
	}

	///////////////////////////////////////////////////////////////////////////////////
	// Fetch the result of a previous asynchronous read
//	byte get_read_result(byte *data, int len) {
//		DBGLOG0("get_read_result");
//		memcpy(data, (void*)m_buf, len);
//		return (m_txn.status == kStatus_Success);
//	}



	/*
	///////////////////////////////////////////////////////////////////////////////////
	// Synchronous read transaction
	byte read(int address, byte *data, int len) {
		DBGLOG0("read");

		// wait for the i2c bus manager to become free
		while(g_i2c_bus.busy());

		// kick off the read
		if(!init_read(address,len)) {
			return 0;
		}

		// wait for the whole read process to complete
		while(m_state != ST_IDLE) {
			run_i2c();
		}
		DBGLOG0("read: ok");

		// pass back the result
		return get_read_result(data,len);
	}
*/
	///////////////////////////////////////////////////////////////////////////////////
	// Run the Storage manager state machine (the I2C bus mananger state machine
	// must also be polled!)
	void run_i2c() {
		if(g_i2c_bus.busy()) {
			// cannot do anything till the bus is free
			return;
		}


		switch(m_state) {
			// waiting for a transmit to complete
			case ST_TX_IN_PROG:
				DBGLOG0("ST_TX_IN_PROG");
				if(!m_txn.pending) { 							// is the transmit complete?
					if(m_txn.status == kStatus_Success) { 		// was it successful
						DBGLOG0("success");

						m_buf_index += m_txn.data_len;			// advance the index
						m_txn.location += m_txn.data_len;				// advance the EEPROM address
						if(m_buf_index < m_buf_len) {			// still more to send?

							// work out size of the next block to sent (up to page size)
							int block_size = m_buf_len - m_buf_index;
							if(block_size > PAGE_SIZE) {
								block_size = PAGE_SIZE;
							}

							m_txn.data = &m_buf[m_buf_index]; 				// set new data start pointer in source buffer
							m_txn.data_len = block_size; 		// next block cued up
							m_txn.status = -1;
							m_nak_retry = 30;
							DBGLOG1("write to %d\r\n", m_txn.location);
							g_i2c_bus.transmit(&m_txn);			// kick off transmit
						}
						else {
							m_state = ST_IDLE;					// all done
						}
					}
					else if(m_txn.status == kStatus_I2C_Addr_Nak && m_nak_retry) {
						DBGLOG0("write nak");
						m_last_ms = (byte)g_clock.get_ms();
						--m_nak_retry;
						m_state = ST_TX_WAIT_CYCLE;
					}
					else {
						DBGLOG0("other fail");
//TODO
						m_state = ST_IDLE;					// error
					}

				}
				break;
			case ST_TX_WAIT_CYCLE:
				if((byte)g_clock.get_ms() != m_last_ms) {
					DBGLOG0("ST_TX_WAIT_CYCLE tick");
					m_state = ST_TX_IN_PROG;
				}
				break;
				/*
			case ST_RX_IN_PROG:
				DBGLOG0("ST_RX_IN_PROG");
				if(!m_txn.pending) { 						// current transaction finished
					m_buf_index += m_block_size;			// advance the index
					m_address += m_block_size;				// advance the EEPROM address
					if(m_buf_index < m_buf_len) {			// still more to receive?
					}

					m_data_index += PAGE_SIZE; 			// move to next block
					if(m_data_index < m_data_len) { 		// any more data to send
						m_txn.data = &m_buf[m_data_index];
						m_txn.data_len = PAGE_SIZE; 		// next block cued up
						m_txn.location += PAGE_SIZE; 		// new source location
						DBGLOG1("read from %d\r\n", m_txn.location);
						g_i2c_bus.receive(&m_txn);			// kick off receive
					}
					else {
						DBGLOG0("done");
						m_state = ST_IDLE;					// all done
					}
				}
				break;*/
		}
	}
};
CStorage g_storage;

#endif /* STORAGE_H_ */
