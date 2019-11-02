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
//  SEQUENCE EDITOR
//
///////////////////////////////////////////////////////////////////////////////////

#ifndef SEQUENCE_EDITOR_H_
#define SEQUENCE_EDITOR_H_

// This class provides the user interface for editing one layer of the sequence
class CSequenceEditor {

	// misc constants
	enum {
		GRID_WIDTH = 32,	// number of columns in the grid
		GRID_HEIGHT = 16,	// number of rows in the grid
		POPUP_MS = 2000,		// how long popup window is to be displayed
		PPI_MS = 100		// play page indicator timeout
	};

	// enumeration of the "gestures" or actions that the user can perform
	typedef enum:byte {
		ACTION_NONE,		// no action in progress
		ACTION_BEGIN,		// start of an action, when a button is first pressed
		ACTION_ENC_LEFT,	// encoder is turned anticlockwise
		ACTION_ENC_RIGHT,   // encoder is turned clockwise
		ACTION_HOLD,		// button has been held down for a certain period with no encoder turn
		ACTION_CLICK,		// button pressed and release without encoder turn
		ACTION_KEY_COMBO,	// button is pressed with EDIT key used as shift
		ACTION_END			// end of an action, when button is released
	} ACTION;

	// once a command mode is entered from an action being in progress, all input is
	// directed to the command handler, which can deal with confirmation, prompts etc
	// command mode ends when all the action buttons are released
	typedef enum {
		CMD_NONE,
		CMD_CLEAR_PAGE,
		CMD_CLEAR_LAYER,
		CMD_CLONE_PAGE,
		CMD_CLONE_LAYER,
		CMD_MEMORY
	} COMMAND;

	enum {
		GATE_VIEW_GATE_TIE,
		GATE_VIEW_VELOCITY,
		GATE_VIEW_PROB,
		GATE_VIEW_RETRIG,
		GATE_VIEW_MAX = GATE_VIEW_RETRIG
	};

	enum {
		BRIGHT_OFF,
		BRIGHT_LOW,
		BRIGHT_MED,
		BRIGHT_HIGH
	};

	enum {
		CLONE_NONE,
		CLONE_MARKED,
		CLONE_ACTIONED
	};

	typedef struct {
		int m_show_grid:1;
		int m_auto_gate:1;
	} CONFIG;
	CONFIG m_cfg;

	//
	// MEMBER VARIABLES
	//
	ACTION m_action;			// the action being performed by the user
	uint32_t m_action_key;		// the key to which the action applies
	uint32_t m_last_action_key;
	uint32_t m_key_combo;		// keys pressed in conjunction with edit shift
	byte m_encoder_moved;		// whether encoder has been previously moved since action was in progress
	byte m_combo_clicks;
	int m_cursor;				// position of the vertical cursor bar
	int m_edit_value;			// the value being edited (e.g. shift offset)
	PARAM_ID m_edit_param;
	COMMAND m_command;
	const char *m_cmd_prompt;
	const char *m_cmd_values;
	int m_num_values;

	int m_sel_from;				// start of selection range
	int m_sel_to;				// end of selection range
	int m_gate_view;			// which gate layer is being viewed
	int m_clone_source;			// column from which to clone data
	byte m_clone_status;
	//byte m_confirm_pending;
	byte m_cur_layer;			// the layer number that is being viewed
	byte m_cur_page;			// the page within the layer that is being viewed
	byte m_memo_slot;
	byte m_ppi_timeout;			// play page indicator timeout

	CSequenceStep m_clone_step;	// during clone operation..
	//
	// PRIVATE METHODS
	//

	///////////////////////////////////////////////////////////////////////////////
	// initialise everything
	void init_state() {
		m_action = ACTION_NONE;
		m_action_key = 0;
		m_last_action_key = 0;
		m_key_combo = 0;
		m_encoder_moved = 0;
		m_cursor = 0;
		m_edit_value = 0;
		m_sel_from = -1;
		m_sel_to = -1;
		m_gate_view = GATE_VIEW_GATE_TIE;
		m_clone_source = 0;
		m_clone_status = CLONE_NONE;
		m_cur_layer = 0;
		m_cur_page = 0;
		//m_confirm_pending = 0;
		m_command = CMD_NONE;
		m_cmd_prompt = NULL;
		m_cmd_values = NULL;
		m_num_values = 0;
		m_edit_param = P_NONE;
		m_memo_slot = 0;
	}

	///////////////////////////////////////////////////////////////////////////////
	void init_config() {
		m_cfg.m_show_grid = 1;
		m_cfg.m_auto_gate = 1;
	}

	/*
	///////////////////////////////////////////////////////////////////////////////
	void show_gate_view() {
		switch(m_gate_view) {
		case GATE_VIEW_GATE_TIE:
			g_popup.text("GATE", 4);
			break;
		case GATE_VIEW_PROB:
			g_popup.text("PROB", 4);
			break;
		case GATE_VIEW_RETRIG:
			g_popup.text("RETR", 4);
			break;
		}
		g_popup.avoid(m_cursor);
	}
*/
	///////////////////////////////////////////////////////////////////////////////
	void show_gate_prob(CSequenceStep& step) {
		g_popup.text("PRB.");
		g_popup.num2digits(step.get_prob(),1);
		g_popup.avoid(m_cursor);
	}

	///////////////////////////////////////////////////////////////////////////////
	void show_gate_retrig(CSequenceStep& step) {
		g_popup.text("RTR.");
		g_popup.num2digits(step.get_retrig(),1);
		g_popup.avoid(m_cursor);
	}

	///////////////////////////////////////////////////////////////////////////////
	void show_gate_accent(CSequenceStep& step) {
		g_popup.text("ACC.");
		g_popup.text_value("--|LO|ME|HI", step.get_accent(), 1);
		g_popup.avoid(m_cursor);
	}

	///////////////////////////////////////////////////////////////////////////////
	const char *get_layer() {
		static char text[3];
		text[0] = 'L';
		text[1] = '1' + m_cur_layer;
		text[2] = '\0';
		return text;
	}
	///////////////////////////////////////////////////////////////////////////////
	const char *get_layer_page() {
		static char text[3];
		text[0] = '1' + m_cur_layer;
		text[1] = 'A' + m_cur_page;
		text[2] = '\0';
		return text;
	}
	///////////////////////////////////////////////////////////////////////////////
	const char *get_memo_slot() {
		static char text[4];
		text[0] = 'M' ;
		text[1] = '0' + m_memo_slot;
		text[2] = ':';
		text[3] = '\0';
		return text;
	}
	///////////////////////////////////////////////////////////////////////////////
	void show_layer_page() {
		g_popup.text(get_layer_page());
		if(!g_sequence.get_layer(m_cur_layer).get_enabled()) {
			g_popup.text("$",1);
		}
		g_popup.avoid(m_cursor);
	}

	///////////////////////////////////////////////////////////////////////////////
	void show_page_list(int value) {
		g_popup.text_value("A   |AB  |ABC |ABCD", value);
		g_popup.avoid(m_cursor);
	}

	///////////////////////////////////////////////////////////////////////////////
	void show_grid(CSequenceLayer& layer) {
		int n;
		int notes_per_octave;
		int row;
		int spacing = 0;
		switch (layer.get_mode()) {
		case V_SQL_SEQ_MODE_PITCH:
			if(layer.is_scaled_view()) {
				notes_per_octave = CScale::instance().get_notes_per_octave();	// e.g. 7
				n = layer.get_scroll_ofs() + 15; // note at top row of screen
				n = notes_per_octave*(n/notes_per_octave); // C at bottom of that octave
				row = 12 - n + layer.get_scroll_ofs(); // now take scroll offset into account
				spacing = notes_per_octave;
			}
			else {
				n = layer.get_scroll_ofs() + 15; // note at top row of screen
				n = 12*(n/12); // C at bottom of that octave
				row = 12 - n + layer.get_scroll_ofs(); // now take scroll offset into account
				spacing = 12;
			}
			break;
		case V_SQL_SEQ_MODE_OFFSET:
			row = CSequenceLayer::OFFSET_ZERO;
			break;
		case V_SQL_SEQ_MODE_MOD:
		default:
			row =-1; // no grid
			break;
		}

		while(row < 16) {
			if(row >= 0 && row <=12) {
				g_ui.hilite(row) = 0x11111111U;
			}
			if(spacing > 0) {
				row += spacing;
			}
			else {
				break;
			}
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// display a popup window with info about the current step
	void show_step_value(CSequenceLayer& layer, int value) {
		switch(layer.get_mode()) {
		case V_SQL_SEQ_MODE_PITCH:
			g_popup.note_name(value);
			break;
		case V_SQL_SEQ_MODE_OFFSET:
			g_popup.show_offset(value-CSequenceLayer::OFFSET_ZERO);
			break;
		case V_SQL_SEQ_MODE_MOD:
			g_popup.num3digits(value);
			break;
		}
		g_popup.avoid(m_cursor);
	}

	///////////////////////////////////////////////////////////////////////////////
	void inc_step_value(CSequenceStep& step, int delta, byte fine, CSequenceLayer& layer) {
		int value;
		int max_value = 127;
		value = step.get_value();
		switch(layer.get_mode()) {
		case V_SQL_SEQ_MODE_MOD:
			if(fine) {
				value += delta;
			}
			else {
				value = 10*(value/10 + delta);
			}
			break;
		case V_SQL_SEQ_MODE_PITCH:
			if(layer.is_scaled_view() && !fine) {
				value = CScale::instance().note_to_index(value);
				value += delta;
				value = CScale::instance().index_to_note(value);
				max_value = CScale::instance().max_index();
				break;
			} // else fall thru
		case V_SQL_SEQ_MODE_OFFSET:
		default:
			value += delta;
			break;
		}
		if(value<0) {
			value = 0;
		}
		else if(value>max_value) {
			value = max_value;
		}
		step.set_value(value);
	}

	///////////////////////////////////////////////////////////////////////////////
	// scroll display up or down
	void scroll(CSequenceLayer& layer, int dir) {
		switch(layer.get_mode()) {
		case V_SQL_SEQ_MODE_PITCH:
		case V_SQL_SEQ_MODE_OFFSET:
			{
				int scroll_ofs = layer.get_scroll_ofs();
				scroll_ofs += dir;
				if(scroll_ofs < 0) {
					scroll_ofs = 0;
				}
				else if(scroll_ofs > 114) {
					scroll_ofs = 114;
				}
				layer.set_scroll_ofs(scroll_ofs);
				break;
			}
		case V_SQL_SEQ_MODE_MOD:
		default:
			break;
		}
	}


	///////////////////////////////////////////////////////////////////////////////
	// change step value based on encoder event
	void value_action(CSequenceLayer& layer, CSequenceStep& step, ACTION what, byte fine) {
		switch(what) {
		case ACTION_ENC_LEFT:
			inc_step_value(step, -1, fine, layer);
			break;
		case ACTION_ENC_RIGHT:
			inc_step_value(step, +1, fine, layer);
			break;
		default:
			break;
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	byte encoder_action(ACTION what, int& value, int min, int max, int wrap) {
		switch(what) {
		case ACTION_ENC_RIGHT:
			if(value < max) {
				++value;
				return 1;
			}
			else if(wrap) {
				value = min;
				return 1;
			}
			break;
		case ACTION_ENC_LEFT:
			if(value > min) {
				--value;
				return 1;
			}
			else if(wrap) {
				value = max;
				return 1;
			}
		}
		return 0;
	}
	///////////////////////////////////////////////////////////////////////////////
	int inc_value(ACTION what, int value, int min, int max, int wrap) {
		int x = value;
		encoder_action(what, x, min, max, wrap);
		return x;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Move cursor left / right for encoder event
	inline void cursor_action(ACTION what, byte wrap) {
		encoder_action(what, m_cursor, 0, GRID_WIDTH-1, wrap);
	}


	////////////////////////////////////////////////
	void command_prompt() {
		if(m_cmd_prompt) {
			g_popup.text(m_cmd_prompt);
			g_popup.text_value(m_cmd_values,m_edit_value,1);
		}
		else {
			g_popup.text_value(m_cmd_values,m_edit_value);
		}
		g_popup.no_hide();
	}

	////////////////////////////////////////////////
	void command_mode(COMMAND cmd) {
		m_command = cmd;
		m_cmd_values = NULL;
		switch(cmd) {
		case CMD_CLEAR_PAGE:
		case CMD_CLEAR_LAYER:
			m_edit_value = 0;
			m_num_values = 2;
			m_cmd_prompt = "SURE?";
			m_cmd_values = " NO|YES";
			break;
		case CMD_CLONE_PAGE:
			m_edit_value = 1 + 4 * m_cur_layer + m_cur_page;
			m_num_values = 17;
			m_cmd_prompt = get_layer_page();
			m_cmd_values = ">??|>1A|>1B|>1C|>1D|>2A|>2B|>2C|>2D|>3A|>3B|>3C|>3D|>4A|>4B|>4C|>4D";
			break;
		case CMD_CLONE_LAYER:
			m_edit_value = 1 + m_cur_layer;
			m_num_values = 5;
			m_cmd_prompt = get_layer();
			m_cmd_values = ">??|>L1|>L2|>L3|>L4";
			break;
		case CMD_MEMORY:
			m_edit_value = 1;
			m_num_values = 3;
			m_cmd_prompt = get_memo_slot();
			m_cmd_values = "EXIT|LOAD|SAVE";
			break;
		}
		command_prompt();
	}

	////////////////////////////////////////////////
	void command_action(CSequenceLayer& layer, ACTION what) {
		switch(what) {
			////////////////////////////////////////////////
		case ACTION_ENC_LEFT:
		case ACTION_ENC_RIGHT:
			if(encoder_action(what, m_edit_value, 0, m_num_values-1, 0)) {
				command_prompt();
			}
			break;
		case ACTION_END:
			if(exec_command(layer, m_command, m_edit_value)) {
				g_popup.text("DONE");
			}
			else {
				g_popup.hide();
			}
			m_command = CMD_NONE;
			break;
		default:
			break;
		}

	}

	///////////////////////////////////////////////////////////////////////////////
	void toggle_init() {
		m_edit_param = P_NONE;
	}

	///////////////////////////////////////////////////////////////////////////////
	void toggle(PARAM_ID param, const char* prompt, const char* values, int num_values = 2) {
		if(m_edit_param != param) {
			m_edit_param = param;
		}
		else {
			int value = get(param)+1;
			if(value > num_values-1) {
				value = 0;
			}
			set(param, value);
		}
		m_cmd_prompt = prompt;
		m_cmd_values = values;
		m_edit_value = get(param);
		command_prompt();
	}

	///////////////////////////////////////////////////////////////////////////////
	void toggle_done() {
		m_edit_param = P_NONE;
		m_cmd_prompt = NULL;
		m_cmd_values = NULL;
		g_popup.hide_after_timeout();
	}


	////////////////////////////////////////////////
	byte exec_command(CSequenceLayer& layer, COMMAND cmd, int value) {
		switch(cmd) {
		case CMD_CLEAR_PAGE:
			if(value) {
				layer.clear_page(m_cur_page);
				return 1;
			}
			break;
		case CMD_CLEAR_LAYER:
			if(value) {
				layer.clear();
				m_cur_page = 0;
				return 1;
			}
			break;
		case CMD_CLONE_PAGE:
			if(value) {
				byte target_layer = (value-1)/4;
				byte target_page = (value-1)%4;
				if(target_layer != m_cur_layer || target_page != m_cur_page) {
					CSequencePage page;
					layer.get_page_content(m_cur_page, &page);
					g_sequence.get_layer(target_layer).set_page_content(target_page, &page);
					return 1;
				}
			}
			break;
		case CMD_CLONE_LAYER:
			if(value) {
				byte target_layer = value - 1;
				if(target_layer != m_cur_layer) {
					g_sequence.get_layer(target_layer).set_content(g_sequence.get_layer(m_cur_layer));
 					return 1;
				}
			}
			break;
		case CMD_MEMORY:
			if(value == 1) {
				g_sequence.load_patch(m_memo_slot);
			}
			else if(value == 2) {
				g_sequence.save_patch(m_memo_slot);
			}
		}
		return 0;
	}
	///////////////////////////////////////////////////////////////////////////////
	// STUFF WHAT THE CV BUTTON DOES...
	byte cv_action(CSequenceLayer& layer, ACTION what) {
		CSequenceStep step = layer.get_step(m_cur_page, m_cursor);
		switch(what) {
		////////////////////////////////////////////////
		case ACTION_BEGIN:
			// when button first pressed, current step
			// scrolled into view and described
			//layer.set_scroll_for(step.get_value());
			show_step_value(layer, step.get_value());
			break;
		////////////////////////////////////////////////
		case ACTION_HOLD:
			// holding the button down shows the layer id
			show_layer_page();
			break;
		////////////////////////////////////////////////
		case ACTION_CLICK:
			layer.set_scroll_for(step.get_value());
			break;
		////////////////////////////////////////////////
		case ACTION_ENC_LEFT:
		case ACTION_ENC_RIGHT:
			switch(m_key_combo) {
			case KEY_CV|KEY2_CV_SCROLL:
				// action to shift all points up or down
				if(what == ACTION_ENC_LEFT) {
					scroll(layer,-1);
				}
				else {
					scroll(layer,+1);
				}
				break;
			case KEY_CV|KEY2_CV_MOVE_VERT:
				// action to shift all points up or down
				if(what == ACTION_ENC_LEFT) {
					if(layer.shift_vertical(m_cur_page, -1)) {
						--m_edit_value;
					}
				}
				else {
					if(layer.shift_vertical(m_cur_page, +1)) {
						++m_edit_value;
					}
				}
				g_popup.show_offset(m_edit_value);
				break;
			case KEY_CV|KEY2_CV_MOVE_HORZ:
				// action to shift all points left or right
				if(encoder_action(what, m_edit_value, -GRID_WIDTH, GRID_WIDTH, 0)) {
					if(what == ACTION_ENC_LEFT) {
						layer.shift_horizontal(m_cur_page, -1);
					}
					else {
						layer.shift_horizontal(m_cur_page, +1);
					}
				}
				g_popup.show_offset(m_edit_value);
				break;
			case KEY_CV|KEY2_CV_FINE:
			default:
				// the first turn sets as a data point
				if(!step.is_data_point()) {
					step.set_data_point(1);
					if(m_cfg.m_auto_gate) {
						step.set_gate(1);
					}
				}
				else {
					// change the value
					value_action(layer, step, what, (m_key_combo&KEY2_CV_FINE));
				}
				layer.set_step(m_cur_page, m_cursor, step);
				layer.set_scroll_for(step.get_value());
				show_step_value(layer, step.get_value());
				break;
			}
			break;
		////////////////////////////////////////////////
		case ACTION_KEY_COMBO:
			switch(m_key_combo) {
			case KEY_CV|KEY2_CV_MOVE_VERT:
				m_edit_value = 0;
				g_popup.text("VERT");
				break;
			case KEY_CV|KEY2_CV_MOVE_HORZ:
				m_edit_value = 0;
				g_popup.text("HORZ");
				break;
			case KEY_CV|KEY2_CV_AUTO_SCROLL:
				layer.set_scroll_for_page(m_cur_page);
				break;
			}
			break;
		default:
			break;
		}
		return 0;
	}

	///////////////////////////////////////////////////////////////////////////////
	// GATE BUTTON
	void gate_action(CSequenceLayer& layer, ACTION what) {
		CSequenceStep step = layer.get_step(m_cur_page, m_cursor);
		switch(what) {
		////////////////////////////////////////////////
		case ACTION_BEGIN:
			m_edit_value = 0;
			break;
		case ACTION_END:
			m_gate_view = GATE_VIEW_GATE_TIE;
			break;
		case ACTION_KEY_COMBO:
			switch(m_key_combo) {
			case KEY_GATE|KEY2_GATE_PROB:
				m_gate_view = GATE_VIEW_PROB;
				show_gate_prob(step);
				break;
			case KEY_GATE|KEY2_GATE_RETRIG:
				m_gate_view = GATE_VIEW_RETRIG;
				show_gate_retrig(step);
				break;
			case KEY_GATE|KEY2_GATE_VEL:
				m_gate_view = GATE_VIEW_VELOCITY;
				show_gate_accent(step);
				break;
			}
			break;
		case ACTION_ENC_LEFT:
		case ACTION_ENC_RIGHT:
			switch(m_key_combo) {
			case KEY_GATE|KEY2_GATE_PROB:
				step.set_prob(inc_value(what, step.get_prob(), 0, CSequenceStep::PROB_MAX, 0));
				layer.set_step(m_cur_page, m_cursor, step);
				show_gate_prob(step);
				m_gate_view = GATE_VIEW_PROB;
				break;
			case KEY_GATE|KEY2_GATE_RETRIG:
				step.set_retrig(inc_value(what, step.get_retrig(), 0, CSequenceStep::RETRIG_MAX, 0));
				layer.set_step(m_cur_page, m_cursor, step);
				show_gate_retrig(step);
				m_gate_view = GATE_VIEW_RETRIG;
				break;
			case KEY_GATE|KEY2_GATE_VEL:
				step.set_accent(inc_value(what, step.get_accent(), 0, CSequenceStep::ACCENT_MAX, 0));
				layer.set_step(m_cur_page, m_cursor, step);
				show_gate_accent(step);
				m_gate_view = GATE_VIEW_VELOCITY;
				break;
			default:
				if(what == ACTION_ENC_RIGHT) {
					// extend gate tie from current position
					byte pos = m_cursor+m_edit_value;
					if(pos < CSequencePage::MAX_STEPS-1) {
						step = layer.get_step(m_cur_page, pos);
						step.set_tie(1);
						layer.set_step(m_cur_page, pos, step);
					}
					++m_edit_value;
				}
				else {
					// remove gate ties
					if(m_edit_value>=0) {
						byte pos = m_cursor+m_edit_value;
						step = layer.get_step(m_cur_page, pos);
						step.set_tie(0);
						layer.set_step(m_cur_page, pos, step);
						if(m_edit_value>0) {
							--m_edit_value;
						}
					}
				}
				m_gate_view = GATE_VIEW_GATE_TIE;
				break;
			}
			break;
		////////////////////////////////////////////////
		case ACTION_CLICK:
			//CSequencePage& page = layer.get_page(m_cur_page);
			if(m_gate_view == GATE_VIEW_GATE_TIE) {
				CSequenceStep step = layer.get_step(m_cur_page, m_cursor);
				step.set_gate(!step.get_gate());
				layer.set_step(m_cur_page, m_cursor, step);
			}
			break;
		default:
			break;
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// CLONE BUTTON
	void clone_action(CSequenceLayer& layer, ACTION what) {
		CSequenceStep::DATA data_type;
		switch(what) {
		////////////////////////////////////////////////
		case ACTION_BEGIN:
			m_clone_step = layer.get_step(m_cur_page, m_cursor);
			break;
		////////////////////////////////////////////////
		case ACTION_CLICK:
			if(m_clone_status == CLONE_NONE) {
				m_clone_source = m_cursor;
				m_clone_status = CLONE_MARKED;
			}
			else {
				m_clone_status = CLONE_NONE;
			}
			break;
		case ACTION_KEY_COMBO:
			switch(m_key_combo) {
			case KEY_CLONE|KEY2_CLONE_PAGE:
				command_mode(CMD_CLONE_PAGE);
				break;
			case KEY_CLONE|KEY2_CLONE_LAYER:
				command_mode(CMD_CLONE_LAYER);
				break;
			}
			break;
			////////////////////////////////////////////////
		case ACTION_ENC_LEFT:
		case ACTION_ENC_RIGHT:
			switch(m_key_combo) {
			case KEY_CLONE|KEY2_CLONE_CV:
				data_type = CSequenceStep::CV_DATA;
				break;
			case KEY_CLONE|KEY2_CLONE_GATE:
				data_type = CSequenceStep::GATE_DATA;
				break;
			default:
				data_type = CSequenceStep::ALL_DATA;
				break;
			}
			if(m_clone_status == CLONE_NONE) {
				cursor_action(what, 1);
				layer.set_step(m_cur_page, m_cursor, m_clone_step, data_type, 1);
			}
			else {
				CSequenceStep source = layer.get_step(m_cur_page, m_clone_source);
				layer.set_step(m_cur_page, m_cursor, source, data_type, 1);
				cursor_action(what, 1);
				encoder_action(what, m_clone_source, 0, GRID_WIDTH-1, 1);
				m_clone_status = CLONE_ACTIONED;
			}
			break;
		case ACTION_END:
			if(m_clone_status == CLONE_ACTIONED) {
				m_clone_status = CLONE_NONE;
			}
			break;
		default:
			break;
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// CLEAR BUTTON
	void clear_action(CSequenceLayer& layer, ACTION what) {
		switch(what) {
		////////////////////////////////////////////////
		case ACTION_CLICK:
			layer.clear_step(m_cur_page, m_cursor);
			break;
			////////////////////////////////////////////////
		case ACTION_KEY_COMBO:
			switch(m_key_combo) {
			case KEY_CLEAR|KEY2_CLEAR_PAGE:
				command_mode(CMD_CLEAR_PAGE);
				break;
			case KEY_CLEAR|KEY2_CLEAR_LAYER:
				command_mode(CMD_CLEAR_LAYER);
				break;
			}
			break;
		case ACTION_ENC_LEFT:
		case ACTION_ENC_RIGHT:
			switch(m_key_combo) {
			case KEY_CLEAR|KEY2_CLEAR_CV:
				layer.clear_step(m_cur_page, m_cursor, CSequenceStep::CV_DATA);
				cursor_action(what, 1);
				break;
			case KEY_CLEAR|KEY2_CLEAR_GATE:
				layer.clear_step(m_cur_page, m_cursor, CSequenceStep::GATE_DATA);
				cursor_action(what, 1);
				break;
			default:
				layer.clear_step(m_cur_page, m_cursor);
				cursor_action(what, 1);
				break;
			}
			break;
		case ACTION_END:
			if(!m_encoder_moved) {
				layer.clear_step(m_cur_page, m_cursor);
				cursor_action(what, 1);
			}
			break;
		default:
			break;
		}
	}



	///////////////////////////////////////////////////////////////////////////////
	void loop_action(CSequenceLayer& layer, ACTION what) {
		switch(what) {
		case ACTION_BEGIN:
			m_sel_from = m_cursor;
			break;
		case ACTION_CLICK:
			layer.set_play_page(m_cur_page);
			layer.set_pos(m_cursor);
			break;
		////////////////////////////////////////////////
		case ACTION_ENC_LEFT:
		case ACTION_ENC_RIGHT:
			cursor_action(what, 0);
			m_sel_to = m_cursor;
			if(m_sel_to > m_sel_from) {
				g_popup.num2digits(m_sel_to - m_sel_from + 1);
			}
			else {
				g_popup.num2digits(m_sel_from - m_sel_to + 1);
			}
			break;
			////////////////////////////////////////////////
		case ACTION_KEY_COMBO:
			{
				int page = -1;
				switch(m_key_combo) {
				case KEY_LOOP|KEY2_LOOP_CUE_A:
					page = 0;
					break;
				case KEY_LOOP|KEY2_LOOP_CUE_B:
					page = 1;
					break;
				case KEY_LOOP|KEY2_LOOP_CUE_C:
					page = 2;
					break;
				case KEY_LOOP|KEY2_LOOP_CUE_D:
					page = 3;
					break;
				case KEY_LOOP|KEY2_LOOP_CUE_RANDOM:
					layer.cue_random();
					g_popup.text("=RND");
					break;
				case KEY_LOOP|KEY2_LOOP_CUE_FOREGROUND:
					layer.cue_cancel();
					layer.set_play_page(m_cur_page);
					g_popup.text("=FGD");
					break;
				case KEY_LOOP|KEY2_LOOP_CUE_ALL:
					layer.cue_all();
					g_popup.text("=ALL");
					break;
				}
				if(page>=0) {
					byte res;
					if(m_combo_clicks<2) {
						res = layer.cue_first(page);
					}
					else {
						res = layer.cue_next(page);
					}
					if(res) {
						g_popup.text("=");
						g_popup.num2digits(layer.get_cue_list_count(),1);
					}
					else {
						g_popup.text("=IVL");
					}
				}
				break;
			}
		////////////////////////////////////////////////
		case ACTION_END:
			if(m_sel_to >= 0) {
				layer.set_loop_from(m_cur_page, m_sel_from);
				layer.set_loop_to(m_cur_page, m_sel_to);
			}
			m_sel_to = -1;
			m_sel_from = -1;
			break;
		default:
			break;
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	void page_action(CSequenceLayer& layer, ACTION what) {
		switch(what) {
		////////////////////////////////////////////////
		case ACTION_BEGIN:
			m_edit_value = layer.get_max_page_no();
			break;
		case ACTION_CLICK:
			show_layer_page();
			break;
		case ACTION_ENC_LEFT:
		case ACTION_ENC_RIGHT:
			if(m_encoder_moved) {
				encoder_action(what, m_edit_value, 0, CSequenceLayer::NUM_PAGES-1, 0);
			}
			show_page_list(m_edit_value);
			break;
		case ACTION_KEY_COMBO: {

				int new_page = -1;
				switch(m_key_combo) {
				case KEY_PAGE|KEY2_PAGE_A:
					new_page = 0;
					break;
				case KEY_PAGE|KEY2_PAGE_B:
					new_page = 1;
					break;
				case KEY_PAGE|KEY2_PAGE_C:
					new_page = 2;
					break;
				case KEY_PAGE|KEY2_PAGE_D:
					new_page = 3;
					break;
				}
				if(new_page >= 0) {
					layer.prepare_page(new_page, CSequenceLayer::INIT_BLANK);
					m_cur_page = new_page;
					m_edit_value = layer.get_max_page_no(); // we might have added new pages above...
					if(!layer.is_cue_mode()) {
						layer.set_play_page(m_cur_page);
					}
					show_layer_page();
				}
			}
			break;
		case ACTION_END:
			layer.set_max_page_no(m_edit_value);
			if(m_edit_value < m_cur_page) {
				m_cur_page = m_edit_value;
			}
			break;
		default:
			break;
		}
	}


	///////////////////////////////////////////////////////////////////////////////
	// MENU BUTTON
	void layer_action(CSequenceLayer& layer, ACTION what) {
		switch(what) {
			////////////////////////////////////////////////
		case ACTION_ENC_LEFT:
			scroll(layer, -1);
			break;
		case ACTION_ENC_RIGHT:
			scroll(layer, +1);
			break;
		case ACTION_KEY_COMBO:
			switch(m_key_combo) {
				case KEY_LAYER|KEY2_LAYER_1:
					m_cur_layer = 0;
					m_cur_page = 0;
					show_layer_page();
					break;
				case KEY_LAYER|KEY2_LAYER_2:
					m_cur_layer = 1;
					m_cur_page = 0;
					show_layer_page();
					break;
				case KEY_LAYER|KEY2_LAYER_3:
					m_cur_layer = 2;
					m_cur_page = 0;
					show_layer_page();
					break;
				case KEY_LAYER|KEY2_LAYER_4:
					m_cur_layer = 3;
					m_cur_page = 0;
					show_layer_page();
					break;
				case KEY_LAYER|KEY2_LAYER_MUTE:
					layer.set_enabled(!layer.get_enabled());
					show_layer_page();
					break;
			}
			break;
		default:
			break;
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	void func_action(CSequenceLayer& layer, ACTION what) {
		switch(what) {
		case ACTION_BEGIN:
			toggle_init();
			break;
		case ACTION_KEY_COMBO:
			switch(m_key_combo) {
			case KEY_FUNC|KEY2_FUNC_SCALE_MODE:
				toggle(P_SQL_SCALED_VIEW, "ROWS:", "CHR|SCA");
				break;
			case KEY_FUNC|KEY2_FUNC_AUTO_GATE:
				toggle(P_EDIT_AUTO_GATE_INSERT, "TRIG:", "MAN|AUT");
				break;
			case KEY_FUNC|KEY2_FUNC_INTERPOLATE:
				toggle(P_SQL_FILL_MODE, "FILL:", "OFF|PAD|INT",3);
				break;
			case KEY_FUNC|KEY2_FUNC_GRID:
				toggle(P_EDIT_SHOW_GRID, "GRID:", "HID|SHO");
				break;
			case KEY_FUNC|KEY2_FUNC_LOOP_MODE:
				toggle(P_SQL_LOOP_PER_PAGE, "LOOP:", "LAY|PAG");
				break;
			case KEY_FUNC|KEY2_FUNC_PAGE_ADV:
				//toggle(P_SQL_CUE_MODE, "PAGE:", "FGD|BKG");
				break;
			}
			break;
		case ACTION_END:
			toggle_done();
		default:
			break;
		}
	}
	///////////////////////////////////////////////////////////////////////////////
	void memo_action(CSequenceLayer& layer, ACTION what) {
		switch(what) {
		case ACTION_KEY_COMBO:
			m_memo_slot = 0;
			switch(m_key_combo) {
			case KEY_MEMO|KEY2_MEMO_1: m_memo_slot = 1; break;
			case KEY_MEMO|KEY2_MEMO_2: m_memo_slot = 2; break;
			case KEY_MEMO|KEY2_MEMO_3: m_memo_slot = 3; break;
			case KEY_MEMO|KEY2_MEMO_4: m_memo_slot = 4; break;
			case KEY_MEMO|KEY2_MEMO_5: m_memo_slot = 5; break;
			case KEY_MEMO|KEY2_MEMO_6: m_memo_slot = 6; break;
			case KEY_MEMO|KEY2_MEMO_7: m_memo_slot = 7; break;
			case KEY_MEMO|KEY2_MEMO_8: m_memo_slot = 8; break;
			}
			if(m_memo_slot) {
				command_mode(CMD_MEMORY);
			}
			break;
		default:
			break;
		}
	}


	///////////////////////////////////////////////////////////////////////////////
	// function to dispatch an action to the correct handler based on which
	// key has been pressed
	void action(CSequenceLayer& layer, ACTION what) {
		if(m_command) { // a command is in progress, so until
			command_action(layer, what);
		}
		else {
			switch(m_action_key) {
			case KEY_CV:
				cv_action(layer, what);
				break;
			case KEY_CLONE:
				clone_action(layer, what);
				break;
			case KEY_CLEAR:
				clear_action(layer, what);
				break;
			case KEY_GATE:
				gate_action(layer, what);
				break;
			case KEY_LOOP:
				loop_action(layer, what);
				break;
			case KEY_PAGE:
				page_action(layer, what);
				break;
			case KEY_LAYER:
				layer_action(layer, what);
				break;
			case KEY_FUNC:
				func_action(layer, what);
				break;
			case KEY_MEMO:
				memo_action(layer, what);
				break;
			}
		}
	}



public:

	//
	// PUBLIC METHODS
	//

	///////////////////////////////////////////////////////////////////////////////
	// constructor
	CSequenceEditor() {
		init_config();
		init_state();
	}

	///////////////////////////////////////////////////////////////////////////////
	int get(PARAM_ID param) {
		switch(param) {
		case P_EDIT_AUTO_GATE_INSERT: return !!m_cfg.m_auto_gate;
		case P_EDIT_SHOW_GRID: return !!m_cfg.m_show_grid;
		default:
			return g_sequence.get_layer(m_cur_layer).get(param);
		}
	}
	///////////////////////////////////////////////////////////////////////////////
	void set(PARAM_ID param, int value) {
		switch(param) {
		case P_EDIT_AUTO_GATE_INSERT: m_cfg.m_auto_gate = !!value; break;
		case P_EDIT_SHOW_GRID: m_cfg.m_show_grid = !!value; break;
		default:
			g_sequence.get_layer(m_cur_layer).set(param, value);
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	int is_valid_param(PARAM_ID param) {
		switch(param) {
			case P_EDIT_AUTO_GATE_INSERT:
			case P_EDIT_SHOW_GRID:
				return 1;
			default:
				return g_sequence.get_layer(m_cur_layer).is_valid_param(param);
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////////
	// handle an event
	void event(int evt, uint32_t param) {
		CSequenceLayer& layer = g_sequence.get_layer(m_cur_layer);
		switch(evt) {
		case EV_KEY_PRESS:
			if(!m_action_key) {
				if(param) {
					m_action_key = param;
					m_encoder_moved = 0;
					m_combo_clicks = 0;
					action(layer, ACTION_BEGIN);
				}
			}
			else {
				m_key_combo = param;
				++m_combo_clicks;
				action(layer, ACTION_KEY_COMBO);
			}
			break;
		case EV_KEY_RELEASE:
			if(param & m_key_combo) {
				m_key_combo = 0;
			}
			if(param == m_action_key) {
				action(layer, ACTION_END);
				m_action_key = 0;
			}
			break;
		case EV_KEY_HOLD:
			if(param == m_action_key) {
				action(layer, ACTION_HOLD);
			}
			break;
		case EV_KEY_CLICK:
			if(param == m_action_key) {
				action(layer, ACTION_CLICK);
			}
			break;
		case EV_ENCODER:
			{
				ACTION what = (((int)param)<0)? ACTION_ENC_LEFT: ACTION_ENC_RIGHT;
				if(m_action_key) {
					action(layer, what);
					m_encoder_moved = 1;
				}
				else {
					cursor_action(what, 1);
					g_popup.avoid(m_cursor);
				}
				break;
			}
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////////
	// draw the display
	void repaint() {
		CSequenceLayer& layer = g_sequence.get_layer(m_cur_layer);
		int i;
		uint32_t mask;


		// Clear the display
		g_ui.clear();

		// add grid lines
		if(m_cfg.m_show_grid) {
			show_grid(layer);
		}

		// displaying the cursor
		mask = g_ui.bit(m_cursor);
		for(i=0; i<=12; ++i) {
			g_ui.raster(i) &= ~mask;
			g_ui.hilite(i) |= mask;
		}

		// decide if clone source will be shown on row 13
		if(m_clone_status != CLONE_NONE) {
			mask = g_ui.bit(m_clone_source);
			g_ui.raster(13) |= mask;
			g_ui.hilite(13) |= mask;
		}

		// show the play position on the lowest row
		mask = g_ui.bit(layer.get_pos());
		g_ui.raster(15) |= mask;
		g_ui.hilite(15) |= mask;


		// determine where the "ruler" will be drawn
		int ruler_from;
		int ruler_to;
		if(m_sel_to >= 0) {
			ruler_from = m_sel_from;
			ruler_to = m_sel_to;
		}
		else {
			ruler_from = layer.get_loop_from(m_cur_page);
			ruler_to = layer.get_loop_to(m_cur_page);
		}

		// scan over the full 32 columns
		mask = g_ui.bit(0);
		int c = 0;
		for(i=0; i<32; ++i) {

			// show the "ruler" at the bottom of screen
			if(ruler_from > ruler_to) { // playing backwards
				if(i >= ruler_to && i <= ruler_from) {
					if(!(c & 0x03)) { // steps 0, 4, 8 etc
						g_ui.raster(15) |= mask;
					}
					else {
						g_ui.hilite(15) |= mask;
					}
					--c;
				}
			}
			else {
				if(i >= ruler_from && i <= ruler_to) {
					if(!(c & 0x03)) { // steps 0, 4, 8 etc
						g_ui.raster(15) |= mask;
					}
					else {
						g_ui.hilite(15) |= mask;
					}
					++c;
				}
			}

			// get the current step value and map to display row
			CSequenceStep step = layer.get_step(m_cur_page,i);
			int n = step.get_value();
			if(layer.get_mode() == V_SQL_SEQ_MODE_MOD) {
				n = 12 - step.get_value()/10;
				if(n<0) {
					n=0;
				}
			}
			else {
				if(layer.get_mode() == V_SQL_SEQ_MODE_PITCH && layer.is_scaled_view()) {
					n = CScale::instance().note_to_index(n);
				}
				n = 12 - n + layer.get_scroll_ofs();
			}



			// should this step be shown as active?
			byte show_active_pos =
					(i == layer.get_pos()) &&
					(g_sequence.is_running()) &&
					(layer.get_play_page() == m_cur_page);

			// plot the value point for the step
			if(n >= 0 && n <= 12) {
				if(show_active_pos) {
					g_ui.hilite(n) |= mask;
					g_ui.raster(n) |= mask;
				}
				else if(step.is_data_point()) {
					g_ui.raster(n) |= mask;
					g_ui.hilite(n) &= ~mask;
				}
				else {
					g_ui.hilite(n) |= mask;
					g_ui.raster(n) &= ~mask;
				}
			}

			// determine how the gate point should be displayed
			byte bri = BRIGHT_OFF;
			byte gate_or_tie = step.get_gate()||step.get_tie();
			switch(m_gate_view) {
				case GATE_VIEW_GATE_TIE:
					if(step.get_gate()) {
						bri = step.get_tie()? BRIGHT_HIGH : BRIGHT_MED;
					}
					else if(step.get_tie()){
						bri = BRIGHT_LOW;
					}
					break;
				case GATE_VIEW_PROB:
					if(!!step.get_prob()) {
						bri = gate_or_tie? BRIGHT_HIGH : BRIGHT_MED;
					}
					else if(gate_or_tie){
						bri = BRIGHT_LOW;
					}
					break;
				case GATE_VIEW_RETRIG:
					if(!!step.get_retrig()) {
						bri = gate_or_tie? BRIGHT_HIGH : BRIGHT_MED;
					}
					else if(gate_or_tie){
						bri = BRIGHT_LOW;
					}
					break;
				case GATE_VIEW_VELOCITY:
					if(!!step.get_accent()) {
						bri = gate_or_tie? BRIGHT_HIGH : BRIGHT_MED;
					}
					else if(gate_or_tie){
						bri = BRIGHT_LOW;
					}
					break;
			}

			// gates at current position are highlighted
			if(bri != BRIGHT_OFF && show_active_pos) {
				bri = BRIGHT_HIGH;
			}

			// plot the gate info
			switch(bri) {
			case BRIGHT_LOW:
				g_ui.raster(14) &= ~mask;
				g_ui.hilite(14) |= mask;
				break;
			case BRIGHT_MED:
				g_ui.raster(14) |= mask;
				g_ui.hilite(14) &= ~mask;
				break;
			case BRIGHT_HIGH:
				g_ui.raster(14) |= mask;
				g_ui.hilite(14) |= mask;
				break;
			}

			mask>>=1;
		}


		if(m_ppi_timeout) {
			g_ui.hilite(14) |= 0b11;
			g_ui.hilite(15) |= 0b11;
			g_ui.raster(14) &= ~0b11;
			g_ui.raster(15) &= ~0b11;
			switch(layer.get_play_page()) {
				case 0:
					g_ui.raster(14) |= 0b10;
					break;
				case 1:
					g_ui.raster(14) |= 0b01;
					break;
				case 2:
					g_ui.raster(15) |= 0b10;
					break;
				case 3:
					g_ui.raster(15) |= 0b01;
					break;
			}
		}
	}


	void run() {
		CSequenceLayer& layer = g_sequence.get_layer(m_cur_layer);
		if(layer.is_cue_mode()) {
			if(layer.is_page_advanced()) {
				m_ppi_timeout = PPI_MS;
			}
			else if(m_ppi_timeout) {
				--m_ppi_timeout;
			}
		}
	}
/*
	int get_cfg(byte *dest, int size) {
		int len = sizeof(CONFIG);
		ASSERT(size >= len);
		memcpy(dest, &m_cfg, len);
		return len;
	}
	void set_cfg(byte *src, int size) {
		int len = sizeof(CONFIG);
		ASSERT(size >= len);
		memcpy(&m_cfg, src, len);
		return;
	}*/

};
CSequenceEditor g_sequence_editor;


#endif /* SEQUENCE_EDITOR_H_ */
