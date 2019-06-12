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
//  CHARACTER SET DEFINITIONS
//
///////////////////////////////////////////////////////////////////////////////////

#ifndef _CHARS_H_
#define _CHARS_H_

extern const uint32_t char4x5[];
#define CHAR4X5_ALPHA		0
#define CHAR4X5_HASH		26
#define CHAR4X5_NUMERIC		32
#define CHAR4X5_MINUS		42
#define CHAR4X5_DOT			43
#define CHAR4X5_GT			44
#define CHAR4X5_CROSS		45
#define CHAR4X5_PLUS		46
#define CHAR4X5_BLOCK		47

const uint32_t char4x5[30] = {
		// A thru H
		0xEEECEEEAU,
		0xAA8A888AU,
		0xEC8ACC8EU,
		0xAA8A88AAU,
		0xAEECE8EAU,
		//I thru P
		0xE2A8AEEEU,
		0x42A8EAAAU,
		0x42C8AAAEU,
		0x42A8AAA8U,
		0xECAEAAE8U,
		// Q thru X
		0xEEEEAAAAU,
		0xAA84AAAAU,
		0xACE4AAA4U,
		0xEA24AAEAU,
		0xEAE4ECAAU,
		// Y,Z#
		0xAE200000U,
		0xA2E00000U,
		0x44E00000U,
		0x48800000U,
		0x4E000000U,
		//0-7
		0xECEEAEEEU,
		0xA422A882U,
		0xA4E6EEE2U,
		0xA48222A2U,
		0xEEEE2EE2U,
		//89-
		0xEE00800FU,
		0xAA00404FU,
		0xEEE02AEFU,
		0xA200444FU,
		0xEE048A0FU,
};


#endif // _CHARS_H_
