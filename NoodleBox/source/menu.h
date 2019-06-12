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
//  MAIN MENU
//
///////////////////////////////////////////////////////////////////////////////////

#ifndef MENU_H_
#define MENU_H_

/////////////////////////////////////////////////////////////////////////////////////////////////
class CMenu {
public:
	typedef struct {
		const char *const prompt;
		const PARAM_ID param;
		const PARAM_TYPE type;
		const char *const values;
	} OPTION;
	enum {
		ACTION_NONE = 0,
		ACTION_VALUE_SELECTED,
		ACTION_VALUE_CHANGED,

		NUM_MENU_OPTS = 22,

	};
	const OPTION m_opts[NUM_MENU_OPTS] = {
			{"TYP", P_SQL_SEQ_MODE, PT_ENUMERATED, "SCAL|CHRO|MOD|TRAN"},
			{"FOR", P_SQL_FORCE_SCALE, PT_ENUMERATED, "OFF|ON"},
			{"SCL", P_SQL_SCALE_TYPE, PT_ENUMERATED, "IONI|DORI|PHRY|LYDI|MIXO|AEOL|LOCR"},
			{"ROO", P_SQL_SCALE_ROOT, PT_ENUMERATED, "C|C#|D|D#|E|F|F#|G|G#|A|A#|B"},
			{0},
			{"RAT", P_SQL_STEP_RATE, PT_ENUMERATED, "1|2D|2|4D|2T|4|8D|4T|8|16D|8T|16|16T|32"},
			{"GAT", P_SQL_NOTE_DUR, PT_ENUMERATED, "TRIG|10|20|30|40|50|60|70|80|90|100|LEGA|OPEN"},
			{0},
			{"MCH", P_SQL_MIDI_CHAN, PT_ENUMERATED, "NONE|1|2|3|4|5|6|7|8|9|10|11|12|13|14|15|16"},
			{"MCC", P_SQL_MIDI_CC, PT_NUMBER_7BIT},
			{"VEL", P_SQL_MIDI_VEL, PT_NUMBER_7BIT},
			{"ACV", P_SQL_MIDI_VEL_ACCENT, PT_NUMBER_7BIT},
			{"CVV", P_SQL_CVRANGE, PT_VOLT_RANGE},
			{"CVO", P_SQL_CVSCALE, PT_ENUMERATED, "1V|1.2V|HZV"},
			{"GLD", P_SQL_CVGLIDE, PT_ENUMERATED, "OFF|ON"},
			{"TRG", P_SQL_TRAN_TRIG, PT_ENUMERATED, "ORIG|THIS|AND|OR|XOR"},
			{"ACC", P_SQL_TRAN_ACC,  PT_ENUMERATED, "NORM|LOCK"},
			{0},
			{"CLK", P_CLOCK_SRC, PT_ENUMERATED, "INT|MIDI|EXT"},
			{"BPM", P_CLOCK_BPM, PT_BPM},
			{"IN", P_CLOCK_IN_RATE, PT_ENUMERATED, "16|8|4|24PP"},
			{"OUT", P_CLOCK_OUT_RATE, PT_ENUMERATED, "1|2D|2|4D|2T|4|8D|4T|8|16D|8T|16|16T|32|24PP"}
	};


	//byte m_num_opts;
	//const OPTION *m_opts;

	byte m_item;
	int m_value;
	byte m_repaint;

	byte m_action;

	/////////////////////////////////////////////////////////////////////////////////////////////////
	CMenu() {
		activate();
	}

	void activate() {
		m_action = ACTION_NONE;
		m_item = 0;
		m_repaint = 1;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////
	void event(int evt, uint32_t param) {
		int i;
		switch(evt) {
		case EV_ENCODER:
			if(m_action == ACTION_VALUE_SELECTED || m_action == ACTION_VALUE_CHANGED) {
				i = m_value + (int)param;
				if(i >= CParams::min_value(m_opts[m_item].type) && i <= CParams::max_value(m_opts[m_item].type, m_opts[m_item].values)) {
					m_value = i;
					m_action = ACTION_VALUE_CHANGED;
					m_repaint = 1;
				}
			}
			else {
				i = m_item;
				for(;;) {
					if((int)param < 0) {
						--i;
					}
					else {
						++i;
					}
					if(i<0 || i>=NUM_MENU_OPTS) {
						break;
					}
					if(m_opts[i].prompt && CParams::is_valid_for_menu(m_opts[i].param)) {
						m_item = i;
						m_repaint = 1;
						break;
					}
				}
			}
			break;
		case EV_KEY_PRESS:
			if(param == KEY_EDIT) {
				m_value = CParams::get(m_opts[m_item].param);
				m_action = ACTION_VALUE_SELECTED;
			}
			break;
		case EV_KEY_RELEASE:
			if(m_action == ACTION_VALUE_CHANGED) {
				CParams::set(m_opts[m_item].param, m_value);
				m_repaint = 1;
			}
			m_action = ACTION_NONE;
			break;

		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////
	void force_repaint() {
		m_repaint = 1;
	}

	int draw_menu_option(int opt, int row) {
		uint32_t buf[5] = {0};
		int state = 0;
		int visible = 0;
		if(opt >= 0 && opt < NUM_MENU_OPTS) {
			const CMenu::OPTION &this_opt = m_opts[opt];
			if(this_opt.prompt) {
				int value;
				if(opt == m_item) {
					if(m_action == ACTION_VALUE_CHANGED) {
						state = 2;
						value = m_value;
					}
					else {
						state = 1;
					}
				}
				if(state != 2) {
					value = CParams::get(this_opt.param);
				}

				const char *sz = this_opt.prompt;
				int col = 0;
				while(*sz) {
					g_ui.print_char(*sz, col, 0, buf, NULL, 5);
					++sz;
					col += 4;
				}
				sz = CParams::value_string(this_opt.type, value, this_opt.values);
				col = 16;
				while(*sz && *sz != '|') {
					g_ui.print_char(*sz, col, 0, buf, NULL, 5);
					++sz;
					col += 4;
				}

				for(int i=0; i<5; ++i) {
					if(row>=0 && row<16) {
						switch(state) {
						case 0:
							g_ui.hilite(row) = buf[i];
							g_ui.raster(row) = 0;
							break;
						case 1:
							g_ui.hilite(row) = buf[i] & 0xFFFF0000U;
							g_ui.raster(row) = buf[i];
							break;
						case 2:
							g_ui.hilite(row) = buf[i];
							g_ui.raster(row) = buf[i];
							break;
						}
						visible = 1;
					}
					++row;
				}
			}
			else {
				if(row>=0 && row<16) {
					g_ui.hilite(row) = 0xFFFFFFFF;
					visible = 1;
				}
			}
		}
		return visible;
	}


	/////////////////////////////////////////////////////////////////////////////////////////////////
	void repaint() {
		if(m_repaint) {
			g_ui.clear();

			int visible = 1;
			int opt = m_item - 1;
			int row = 5;
			while(opt >= 0 && visible) {
				if(!m_opts[opt].prompt || CParams::is_valid_for_menu(m_opts[opt].param)) {
					row -= (m_opts[opt].prompt)? 6:2;
					visible = draw_menu_option(opt,row);
				}
				--opt;
			}

			visible = 1;
			opt = m_item;
			row = 5;
			while(opt < NUM_MENU_OPTS && visible) {
				if(!m_opts[opt].prompt || CParams::is_valid_for_menu(m_opts[opt].param)) {
					visible = draw_menu_option(opt,row);
					row += (m_opts[opt].prompt)? 6:2;
				}
				++opt;
			}
			m_repaint = 0;
		}
	}
};

CMenu g_menu;

#endif
