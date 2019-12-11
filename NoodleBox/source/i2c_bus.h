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
//  I2C BUS HANDLING
//
///////////////////////////////////////////////////////////////////////////////////

#ifndef I2C_BUS_H_
#define I2C_BUS_H_

// Forward declaration
void i2c_master_callback(I2C_Type *base, i2c_master_handle_t *handle, status_t status, void *userData);

///////////////////////////////////////////////////////////////////////////////////
//
// Driver for the I2C DAC
//
///////////////////////////////////////////////////////////////////////////////////
class CI2CDac{
	enum { SZ_DATA = 8 };
	volatile byte m_data[SZ_DATA];
	enum { ST_INIT1, ST_INIT2, ST_IDLE, ST_PENDING } m_state;
	uint16_t m_dac[4];
public:
	///////////////////////////////////////////////////////////////////////////////
	CI2CDac() {
		m_state = ST_INIT1;
		memset(m_dac,0,sizeof(m_dac));
	}
	///////////////////////////////////////////////////////////////////////////////
	inline void set(byte which, uint16_t value) {
		if(m_dac[which] != value) {
			m_dac[which] = value;
			if(m_state == ST_IDLE) {
				m_state = ST_PENDING;
			}
		}
	}
	///////////////////////////////////////////////////////////////////////////////
	byte get_tx(i2c_master_transfer_t& xfer) {
		switch(m_state) {
		case ST_INIT1:
			m_data[0] = 0b10001111; // set each channel to use internal vref
			xfer.dataSize = 1;
			break;
		case ST_INIT2:
			m_data[0] = 0b11001111; // set x2 gain on each channel
			xfer.dataSize = 1;
			break;
		case ST_PENDING:
			m_data[0] = ((m_dac[3]>>8) & 0xF);
			m_data[1] = (byte)m_dac[3];
			m_data[2] = ((m_dac[2]>>8) & 0xF);
			m_data[3] = (byte)m_dac[2];
			m_data[4] = ((m_dac[1]>>8) & 0xF);
			m_data[5] = (byte)m_dac[1];
			m_data[6] = ((m_dac[0]>>8) & 0xF);
			m_data[7] = (byte)m_dac[0];
			xfer.dataSize = 8;
			m_state = ST_IDLE;
			break;
		case ST_IDLE:
		default:
			return 0;
		}

		// configure the transfer object
		xfer.flags = kI2C_TransferDefaultFlag;
		xfer.direction = kI2C_Write;
		xfer.slaveAddress = I2C_ADDR_DAC;
		xfer.subaddress = 0;
		xfer.subaddressSize = 0;
		xfer.data = (byte*)&m_data;
		return 1;
	}

	///////////////////////////////////////////////////////////////////////////////
	void handle_rx(status_t status) {
		switch(m_state) {
		case ST_INIT1:
			m_state = ST_INIT2;
			break;
		case ST_INIT2:
			m_state = ST_PENDING;
			break;
		case ST_IDLE:
		case ST_PENDING:
			break;
		}
	}
};
CI2CDac g_i2c_dac;

///////////////////////////////////////////////////////////////////////////////////
//
// Driver for the I2C EEPROM
// M24256 is a 32kB EEPROM with a 64 byte page size
//
///////////////////////////////////////////////////////////////////////////////////
class CI2CEeprom{
	enum {
		SZ_PAGE_SIZE = 64,
		SZ_DATA = 3200,
		WRITE_DELAY = 10
	};
	volatile byte m_data[SZ_DATA];
	enum { ST_IDLE, ST_READ, ST_WRITE, ST_WRITE_DELAY } m_state;

	int m_address;
	int m_bytes;
	int m_index;
	byte m_ms_lsb;
	byte m_write_delay;
	status_t m_status;
public:
	///////////////////////////////////////////////////////////////////////////////
	CI2CEeprom() {
		m_address = 0;
		m_bytes = 0;
		m_index = 0;
		m_ms_lsb = 0;
		m_status = -1;
	}
	status_t get_status() {
		return m_status;
	}
	inline byte& operator[](int i) {
		return (byte&)m_data[i];
	}
	byte is_busy() {
		return (m_state != ST_IDLE);
	}
	byte read(int address, int size) {
		if(m_state != ST_IDLE) {
			return 0;
		}
		ASSERT(size <= SZ_DATA);
		m_status = -1;
		m_address = address;
		m_bytes = size;
		m_state = ST_READ;
		g_midi_led.set(1);
		return 1;
	}
	byte write(int address, int size) {
		if(m_state != ST_IDLE) {
			return 0;
		}
		ASSERT(size <= SZ_DATA);
		m_status = -1;
		m_address = address; // must be on 64 byte boundary
		m_bytes = size;
		m_index = 0;
		m_state = ST_WRITE;
		g_midi_led.set(1);
		return 1;
	}
	byte get_tx(i2c_master_transfer_t& xfer) {

		xfer.slaveAddress = I2C_ADDR_EEPROM;
		xfer.subaddress = m_address;
		xfer.subaddressSize = 2;
		xfer.flags = kI2C_TransferDefaultFlag;
		switch(m_state) {
			case ST_READ:
				xfer.direction = kI2C_Read;
				xfer.data = (byte*)&m_data;
				xfer.dataSize = m_bytes;
			    return 1;
			case ST_WRITE_DELAY:
				if(m_write_delay) {
					if((byte)g_clock.get_ms() != m_ms_lsb) {
						m_ms_lsb = (byte)g_clock.get_ms();
						--m_write_delay;
					}
					return 0;
				}
				m_state = ST_WRITE;
				// no break
			case ST_WRITE:
				xfer.direction = kI2C_Write;
				xfer.data = (byte*)&m_data[m_index];
				xfer.dataSize = SZ_PAGE_SIZE;
				return 1;
		}
		return 0;
	}

	///////////////////////////////////////////////////////////////////////////////
	void handle_rx(status_t status) {
		m_status = status;
		switch(m_state) {
		case ST_READ:
			m_state = ST_IDLE;
			g_midi_led.set(0);
			break;
		case ST_WRITE:
			if(status == kStatus_Success) {
				m_address += SZ_PAGE_SIZE;
				m_index += SZ_PAGE_SIZE;
				m_bytes -= SZ_PAGE_SIZE;
				if(m_bytes <= 0) {
					// done
					m_state = ST_IDLE;
					g_midi_led.set(0);
				}
				else {
					// delay between pages
					if((byte)g_clock.get_ms() != m_ms_lsb);
					m_write_delay = WRITE_DELAY;
					m_state = ST_WRITE_DELAY;
				}
			}
			else {
				// some other error
				m_state = ST_IDLE;
			}
			break;
		default:
			m_state = ST_IDLE;
			break;
		}
	}

	/*
		//M24256 EEPROM
		status_t write_eeprom(uint16_t eeprom_addr, byte *data, int size) {
			status_t result = kStatus_Fail;
			while(size > 0) {

				// get the target EEPROM page base address (64 bytes per page)
				uint16_t page_mask = (eeprom_addr & 0xFFC0);

				// set up transfer block
				i2c_master_transfer_t xfer;
				xfer.slaveAddress = EEPROM_ADDRESS;
				xfer.direction = kI2C_Write;
				xfer.subaddress = eeprom_addr;
				xfer.subaddressSize = 2;
				xfer.data = data;
				xfer.dataSize = 0;
				xfer.flags = kI2C_TransferDefaultFlag;

				// we can only send continuous bytes up until the EEPROM page boundary
				// then we'll need to start a new transaction
				while(size > 0 && ((eeprom_addr & 0xFFC0) == page_mask)) {
					++eeprom_addr;
					++data;
					--size;
					xfer.dataSize++;
				}

				// the EEPROM can NAK us while internal write cycle takes place
				for(int retries = 20;retries>0;--retries) {
					result = I2C_MasterTransferBlocking(I2C0, &xfer);
					if(result != kStatus_I2C_Addr_Nak) {
						break;
					}
					g_clock.wait_ms(1);
				}
			    if(result != kStatus_Success) {
			    	break;
			    }
			}
			return result;
		}

		status_t read_eeprom(uint16_t eeprom_addr, byte *data, int size) {
			i2c_master_transfer_t xfer;
			xfer.slaveAddress = EEPROM_ADDRESS;
			xfer.direction = kI2C_Read;
			xfer.subaddress = eeprom_addr;
			xfer.subaddressSize = 2;
			xfer.data = data;
			xfer.dataSize = size;
			xfer.flags = kI2C_TransferDefaultFlag;
		    return I2C_MasterTransferBlocking(I2C0, &xfer);
		}

		void write_blocking(byte addr, int len) {
			write(addr, len);
			wait();
		}*/


};
CI2CEeprom g_i2c_eeprom;



///////////////////////////////////////////////////////////////////////////////////
//
// I2C bus manager class
//
///////////////////////////////////////////////////////////////////////////////////
class CI2CBus{
private:
	i2c_master_handle_t m_handle;
	i2c_master_transfer_t m_xfer;
	volatile enum { ST_IDLE, ST_EEPROM_BUSY, ST_DAC_BUSY } m_state;
public:
	///////////////////////////////////////////////////////////////////////////////
	CI2CBus() {
		m_state = ST_IDLE;
	}
	///////////////////////////////////////////////////////////////////////////////
	void init() {
		i2c_master_config_t masterConfig;
		I2C_MasterGetDefaultConfig(&masterConfig);
		masterConfig.baudRate_Bps = 500000U;
		I2C_MasterInit(I2C0, &masterConfig, CLOCK_GetFreq(kCLOCK_BusClk));
		I2C_Enable(I2C0, true);
	}
	///////////////////////////////////////////////////////////////////////////////
	void run() {
		if(m_state == ST_IDLE) {
			if(g_i2c_eeprom.get_tx(m_xfer)) {
				I2C_MasterTransferCreateHandle(I2C0, &m_handle, i2c_master_callback, NULL);
				I2C_MasterTransferNonBlocking(I2C0, &m_handle, &m_xfer);
				m_state = ST_EEPROM_BUSY;
			}
			else if(g_i2c_dac.get_tx(m_xfer)) {
				I2C_MasterTransferCreateHandle(I2C0, &m_handle, i2c_master_callback, NULL);
				I2C_MasterTransferNonBlocking(I2C0, &m_handle, &m_xfer);
				m_state = ST_DAC_BUSY;
			}
		}
	}
	///////////////////////////////////////////////////////////////////////////////
	inline void on_txn_complete(I2C_Type *base, i2c_master_handle_t *handle, status_t status, void *userData) {
		switch(m_state) {
		case ST_EEPROM_BUSY:
			g_i2c_eeprom.handle_rx(status);
			break;
		case ST_DAC_BUSY:
			g_i2c_dac.handle_rx(status);
			break;
		}
		m_state = ST_IDLE;
	}

/*
	//M24256 EEPROM
	status_t write_eeprom(uint16_t eeprom_addr, byte *data, int size) {
		status_t result = kStatus_Fail;
		while(size > 0) {

			// get the target EEPROM page base address (64 bytes per page)
			uint16_t page_mask = (eeprom_addr & 0xFFC0);

			// set up transfer block
			i2c_master_transfer_t xfer;
			xfer.slaveAddress = EEPROM_ADDRESS;
			xfer.direction = kI2C_Write;
			xfer.subaddress = eeprom_addr;
			xfer.subaddressSize = 2;
			xfer.data = data;
			xfer.dataSize = 0;
			xfer.flags = kI2C_TransferDefaultFlag;

			// we can only send continuous bytes up until the EEPROM page boundary
			// then we'll need to start a new transaction
			while(size > 0 && ((eeprom_addr & 0xFFC0) == page_mask)) {
				++eeprom_addr;
				++data;
				--size;
				xfer.dataSize++;
			}

			// the EEPROM can NAK us while internal write cycle takes place
			for(int retries = 20;retries>0;--retries) {
				result = I2C_MasterTransferBlocking(I2C0, &xfer);
				if(result != kStatus_I2C_Addr_Nak) {
					break;
				}
				g_clock.wait_ms(1);
			}
		    if(result != kStatus_Success) {
		    	break;
		    }
		}
		return result;
	}

	status_t read_eeprom(uint16_t eeprom_addr, byte *data, int size) {
		i2c_master_transfer_t xfer;
		xfer.slaveAddress = EEPROM_ADDRESS;
		xfer.direction = kI2C_Read;
		xfer.subaddress = eeprom_addr;
		xfer.subaddressSize = 2;
		xfer.data = data;
		xfer.dataSize = size;
		xfer.flags = kI2C_TransferDefaultFlag;
	    return I2C_MasterTransferBlocking(I2C0, &xfer);
	}

	void write_blocking(byte addr, int len) {
		write(addr, len);
		wait();
	}*/
	/*
	void dac_init() {
		m_buf[0] = 0b10001111; // set each channel to use internal vref
		write_blocking(DAC_ADDRESS, 1);
		m_buf[0] = 0b11001111; // set x2 gain on each channel
		write_blocking(DAC_ADDRESS, 1);
	}
	void dac_write(uint16_t dac[4]) {
		m_buf[0] = ((dac[0]>>8) & 0xF);
		m_buf[1] = (byte)dac[0];
		m_buf[2] = ((dac[2]>>8) & 0xF);
		m_buf[3] = (byte)dac[2];
		m_buf[4] = ((dac[1]>>8) & 0xF);
		m_buf[5] = (byte)dac[1];
		m_buf[6] = ((dac[3]>>8) & 0xF);
		m_buf[7] = (byte)dac[3];
		write(DAC_ADDRESS, 8);
	}*/
};

CI2CBus g_i2c_bus;
void i2c_master_callback(I2C_Type *base, i2c_master_handle_t *handle, status_t status, void *userData)
{
	g_i2c_bus.on_txn_complete(base, handle, status, userData);
}

#endif /* I2C_BUS_H_ */
