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
//  CONFIG PARAMETER HELPERS
//
///////////////////////////////////////////////////////////////////////////////////
#ifndef PARAMS_H_
#define PARAMS_H_

class CParams {
	/////////////////////////////////////////////////////////////////////////////////////////////////
	static void format_number(int value, char *buf, int digits) {
		if(digits > 2) {
			*buf++ = '0' + value/100;
		}
		value %= 100;
		if(digits > 1) {
			*buf++ = '0' + value/10;
		}
		value %= 10;
		*buf++ = '0' + value;
		*buf = 0;
	}

public:
	/////////////////////////////////////////////////////////////////////////////////////////////////
	static void set(PARAM_ID param, int value) {
		if(param < P_SQL_MAX) {
			g_sequencer.set(param,value);
		}
		else if(param < P_CLOCK_MAX) {
			g_clock.set(param,value);
		}
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////
	static int get(PARAM_ID param) {
		if(param < P_SQL_MAX) {
			return g_sequencer.get(param);
		}
		else if(param < P_CLOCK_MAX) {
			return g_clock.get(param);
		}
		return 0;
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////
	static int is_valid_for_menu(PARAM_ID param) {
		if(param < P_SQL_MAX) {
			return g_sequencer.is_valid_param(param);
		}
		else if(param < P_CLOCK_MAX) {
			return g_clock.is_valid_param(param);
		}
		return 0;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////
	static const char *value_string(PARAM_TYPE type, int value, const char *values_text) {
		static char buf[9];
		int pos;
		switch(type) {
		case PT_ENUMERATED:
			while(values_text && *values_text && value) {
				if(*values_text == '|') {
					--value;
				}
				++values_text;
			}
			pos = 0;
			while(*values_text && *values_text != '|') {
				buf[pos++] = *values_text++;
			}
			buf[pos] = 0;
			break;
		case PT_MIDI_CHANNEL:
			format_number(value + 1, buf, 2);
			break;
		case PT_DURATION:
			format_number(value, buf, 2);
			break;
		case PT_NUMBER_7BIT:
		case PT_BPM:
			format_number(value, buf, 3);
			break;
		case PT_VOLT_RANGE:
			format_number(value, buf, 1);
			buf[1] = 'V';
			buf[2] = 0;
			break;
		case PT_PATTERN:
			buf[0] = 'A' + value/8;
			buf[1] = '1' + value%8;
			buf[2] = 0;
			break;
		default:
			buf[0] = 0;
			break;
		}
		return buf;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////
	static int max_value(PARAM_TYPE type, const char *values_text) {
		int count;
		switch(type) {
		case PT_ENUMERATED:
			count = 0;
			while(values_text && *values_text) {
				if(*values_text == '|') {
					++count;
				}
				++values_text;
			}
			return count;
		case PT_MIDI_CHANNEL:
			return 15;
		case PT_NUMBER_7BIT:
			return 127;
		case PT_VOLT_RANGE:
			return 8;
		case PT_BPM:
			return 300;
		case PT_PATTERN:
			return 39;
		default:
			return 0;
		}
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////
	static int min_value(PARAM_TYPE type) {
		switch(type) {
		case PT_BPM:
			return 30;
		default:
			return 0;
		}
	}

};



#endif /* PARAMS_H_ */
