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


void i2c_master_callback(I2C_Type *base, i2c_master_handle_t *handle, status_t status, void *userData);

class CI2CBus{

public:
	typedef struct {
		byte addr;				// i2c address of device
		uint16_t location;		// "subaddress" (e.g. EEPROM address)
		byte location_size;		// size of sub address
		byte *data;				// pointer to data buffer
		int data_len;			// amount of data in the buffer
		status_t status;		// status of the transaction
		byte pending;
	} TRANSACTION;
private:
//	enum {
	//	BUFFER_SIZE = 100
		//DAC_ADDRESS = 0b1100000,
		//EEPROM_ADDRESS = 0b1010000
	//};
	i2c_master_handle_t m_handle;
	i2c_master_transfer_t m_xfer;
	//volatile byte m_busy;
	volatile TRANSACTION *m_txn;
	//volatile status_t m_status;
	//byte m_buf[BUFFER_SIZE];
	//size_t m_len;
public:


	CI2CBus() :
		m_txn(NULL)
	{
	}
	void init() {
		i2c_master_config_t masterConfig;
		I2C_MasterGetDefaultConfig(&masterConfig);
		I2C_MasterInit(I2C0, &masterConfig, CLOCK_GetFreq(kCLOCK_BusClk));
		I2C_Enable(I2C0, true);

	}

	inline byte busy() {
		return !!m_txn;
	}

	//inline byte transaction() {
		//return m_transaction;
	//}
	//void wait() {
		//while(s_busy);
	//}

	//	void transmit(byte addr, uint16_t subaddress, byte subaddressSize, byte *buf, int len) {
	byte transmit(volatile TRANSACTION *txn) {
		if(!m_txn) {
			m_xfer.slaveAddress = txn->addr;
			m_xfer.direction = kI2C_Write;
			m_xfer.subaddress = txn->location;
			m_xfer.subaddressSize = txn->location_size;
			m_xfer.data = txn->data;
			m_xfer.dataSize = txn->data_len;
			m_xfer.flags = kI2C_TransferDefaultFlag;
			I2C_MasterTransferCreateHandle(I2C0, &m_handle, i2c_master_callback, NULL);
			m_txn = txn;
			m_txn->pending = 1;
			I2C_MasterTransferNonBlocking(I2C0, &m_handle, &m_xfer);
			return 1;
		}
		else {
			return 0;
		}
	}


	byte receive(volatile TRANSACTION *txn) {
//	void receive(byte addr, uint16_t subaddress, byte subaddressSize, byte *data, int size) {
		if(!m_txn) {
			m_xfer.slaveAddress = txn->addr;
			m_xfer.direction = kI2C_Read;
			m_xfer.subaddress = txn->location;
			m_xfer.subaddressSize = txn->location_size;
			m_xfer.data = txn->data;
			m_xfer.dataSize = txn->data_len;
			m_xfer.flags = kI2C_TransferDefaultFlag;
			I2C_MasterTransferCreateHandle(I2C0, &m_handle, i2c_master_callback, NULL);
			m_txn = txn;
			m_txn->pending = 1;
			I2C_MasterTransferNonBlocking(I2C0, &m_handle, &m_xfer);
			return 1;
		}
		else {
			return 0;
		}
	}

	inline void on_txn_complete(I2C_Type *base, i2c_master_handle_t *handle, status_t status, void *userData) {
		if(m_txn) {
			m_txn->status = status;
			m_txn->data_len = m_xfer.dataSize;
			m_txn->pending = 0;
			m_txn = NULL;
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
