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

class CDigitalOut {
	const gpio_port_num_t _P;
	const uint32_t _B;
	const uint8_t _D;
public:
	CDigitalOut(gpio_port_num_t P, uint32_t B, uint8_t D = 0) :
		_P(P), _B(B), _D(D)
	{
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

class CDigitalIn {
	const gpio_port_num_t _P;
	const uint32_t _B;
public:
	CDigitalIn(gpio_port_num_t P, uint32_t B) :
		_P(P), _B(B)
	{
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


class CPulseOut : public CDigitalOut {
public:
	const int SHORT_BLINK = 1;
	const int MEDIUM_BLINK = 10;
	const int LONG_BLINK = 200;
	uint8_t m_timeout;
	CPulseOut(gpio_port_num_t P, uint32_t B, uint8_t D = 0) :
		CDigitalOut(P,B,D), m_timeout(0) {}
	void blink(uint8_t ms) {
		set(1);
		m_timeout = ms;
	}
	inline void run() {
		if(m_timeout) {
			if(!--m_timeout) {
				set(0);
			}
		}
	}
};
#endif /* DIGITAL_OUT_H_ */
