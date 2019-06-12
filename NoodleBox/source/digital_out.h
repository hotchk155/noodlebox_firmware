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
//  GENERIC DIGITAL IO HELPERS
//
///////////////////////////////////////////////////////////////////////////////////

#ifndef DIGITAL_OUT_H_
#define DIGITAL_OUT_H_

#include "fsl_gpio.h"

template<gpio_port_num_t _P, uint32_t _B, int _D = 0> class CDigitalOut {
public:
	CDigitalOut() {
		gpio_pin_config_t config =
		{
				kGPIO_DigitalOutput,
				_D,
		};
		GPIO_PinInit(_P, _B, &config);
	}
	void set(int state) {
		GPIO_PinWrite(_P, _B, state);
	}
};

template<gpio_port_num_t _P, uint32_t _B> class CDigitalIn {
public:
	CDigitalIn() {
		gpio_pin_config_t config =
		{
				kGPIO_DigitalInput,
				0,
		};
		GPIO_PinInit(_P, _B, &config);
	}
	int get() {
		return GPIO_PinRead(_P, _B);
	}
};

template<gpio_port_num_t _P, uint32_t _B, int _D = 0> class CPulseOut {
public:
	uint8_t m_timeout;
	CPulseOut() {
		gpio_pin_config_t config =
		{
				kGPIO_DigitalOutput,
				_D,
		};
		GPIO_PinInit(_P, _B, &config);
	}
	void set(int state) {
		GPIO_PinWrite(_P, _B, state);
	}
	void blink(uint8_t ms) {
		GPIO_PinWrite(_P, _B, 1);
		m_timeout = ms;
	}
	inline void run() {
		if(m_timeout) {
			if(!--m_timeout) {
				GPIO_PinWrite(_P, _B, 0);
			}
		}
	}

	const int SHORT_BLINK = 1;
	const int MEDIUM_BLINK = 10;
	const int LONG_BLINK = 200;

};
#endif /* DIGITAL_OUT_H_ */
