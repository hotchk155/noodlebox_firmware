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
		PPI_MS = 100,		// play page indicator timeout
		SCROLL_MARGIN = 3
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
		CMD_CLEAR_LAYER
	} COMMAND;

	enum {
		GATE_VIEW_GATE,
		GATE_VIEW_TIE,
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
		int m_scaled_pitch:1;
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
	int m_num_values;

	int m_sel_from;				// start of selection range
	int m_sel_to;				// end of selection range
	byte m_gate_view;			// which gate layer is being viewed
	int m_clone_source;			// column from which to clone data
	byte m_clone_status;
	//byte m_confirm_pending;
	byte m_cur_layer;			// the layer number that is being viewed
	byte m_cur_page;			// the page within the layer that is being viewed

	byte m_ppi_timeout;			// play page indicator timeout
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
		m_gate_view = GATE_VIEW_GATE;
		m_clone_source = 0;
		m_clone_status = CLONE_NONE;
		m_cur_layer = 0;
		m_cur_page = 0;
		//m_confirm_pending = 0;
		m_command = CMD_NONE;
		m_cmd_prompt = NULL;
		m_num_values = 0;
		m_edit_param = P_NONE;
	}

	///////////////////////////////////////////////////////////////////////////////
	void init_config() {
		m_cfg.m_show_grid = 1;
		m_cfg.m_auto_gate = 1;
	}

	///////////////////////////////////////////////////////////////////////////////
	void show_gate_view() {
		switch(m_gate_view) {
		case GATE_VIEW_GATE:
			g_popup.text("TRIG", 4);
			break;
		case GATE_VIEW_TIE:
			g_popup.text("TIES", 4);
			break;
		case GATE_VIEW_PROB:
			g_popup.text("RAND", 4);
			break;
		case GATE_VIEW_RETRIG:
			g_popup.text("RTRG", 4);
			break;
		}
		g_popup.avoid(m_cursor);
	}

	///////////////////////////////////////////////////////////////////////////////
	void show_gate_prob(int value) {
		switch(value) {
		case CSequenceStep::PROB_OFF:
			g_popup.text("ALL", 3);
			break;
		case CSequenceStep::PROB_HIGH:
			g_popup.text("HI", 2);
			break;
		case CSequenceStep::PROB_MED:
			g_popup.text("MED", 3);
			break;
		case CSequenceStep::PROB_LOW:
			g_popup.text("LOW", 3);
			break;
		}
		g_popup.avoid(m_cursor);
	}

	///////////////////////////////////////////////////////////////////////////////
	void show_gate_retrig(int value) {
		switch(value) {
		case CSequenceStep::RETRIG_OFF:
			g_popup.text("OFF", 3);
			break;
		case CSequenceStep::RETRIG_SLOW:
			g_popup.text("SLOW", 4);
			break;
		case CSequenceStep::RETRIG_MED:
			g_popup.text("MED", 3);
			break;
		case CSequenceStep::RETRIG_FAST:
			g_popup.text("FAST", 4);
			break;
		}
		g_popup.avoid(m_cursor);
	}

	///////////////////////////////////////////////////////////////////////////////
	void show_layer_page() {
		char text[3];
		text[0] = '1' + m_cur_layer;
		text[1] = 'A' + m_cur_page;
		if(!g_sequence.get_layer(m_cur_layer).get_enabled()) {
			text[2] = '$';
			g_popup.text(text, 3);
		}
		else {
			g_popup.text(text, 2);
		}

		g_popup.avoid(m_cursor);
	}

	///////////////////////////////////////////////////////////////////////////////
	void show_page_list(int value) {
		switch(value) {
		case 3:
			g_popup.text("ABCD", 4);
			break;
		case 2:
			g_popup.text("ABC", 4);
			break;
		case 1:
			g_popup.text("AB", 4);
			break;
		case 0:
			g_popup.text("A", 4);
			break;
		case -1:
			g_popup.text(" ", 4);
			break;
		}
		g_popup.avoid(m_cursor);
	}

	///////////////////////////////////////////////////////////////////////////////
	void show_grid(CSequenceLayer& layer) {
		int n;
		int notes_per_octave;
		int row;
		int spacing = 0;
		switch (layer.get_view()) {
		case CSequenceLayer::VIEW_PITCH:
			if(m_cfg.m_scaled_pitch) {
				notes_per_octave = layer.get_scale().get_notes_per_octave();	// e.g. 7
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
		case CSequenceLayer::VIEW_PITCH_OFFSET:
			row = CSequenceLayer::OFFSET_ZERO;
			break;
		case CSequenceLayer::VIEW_MODULATION:
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
	void set_scroll_for(CSequenceLayer& layer, int value, int margin = SCROLL_MARGIN) {

		if(layer.get_view() == CSequenceLayer::VIEW_PITCH && m_cfg.m_scaled_pitch) {
			value = g_scale.note_to_index(value);
		}

		int scroll_ofs = layer.get_scroll_ofs();
		if((value-margin)<scroll_ofs) {
			scroll_ofs = value-margin;
		}
		else if((value+margin)>scroll_ofs+12) {
			scroll_ofs = value-12+margin;
		}

		if(scroll_ofs < 0) {
			scroll_ofs = 0;
		}
		layer.set_scroll_ofs(scroll_ofs);
	}

	///////////////////////////////////////////////////////////////////////////////
	// display a popup window with info about the current step
	void show_step_value(CSequenceLayer& layer, int value) {
		switch(layer.get_view()) {
		case CSequenceLayer::VIEW_PITCH:
			g_popup.note_name(value);
			break;
		case CSequenceLayer::VIEW_PITCH_OFFSET:
			g_popup.show_offset(value-CSequenceLayer::OFFSET_ZERO);
			break;
		case CSequenceLayer::VIEW_MODULATION:
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
		switch(layer.get_view()) {
		case CSequenceLayer::VIEW_MODULATION:
			if(fine) {
				value += delta;
			}
			else {
				value = 10*(value/10 + delta);
			}
			break;
		case CSequenceLayer::VIEW_PITCH:
			if(m_cfg.m_scaled_pitch && !fine) {
				value = g_scale.note_to_index(value);
				value += delta;
				value = g_scale.index_to_note(value);
				max_value = g_scale.max_index();
				break;
			} // else fall thru
		case CSequenceLayer::VIEW_PITCH_OFFSET:
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
		if(m_cfg.m_auto_gate) {
			step.set_gate(1);
		}
		step.set_data_point(1);	// the data point has been set by user
	}

	///////////////////////////////////////////////////////////////////////////////
	// scroll display up or down
	void scroll(CSequenceLayer& layer, int dir) {
		switch(layer.get_view()) {
		case CSequenceLayer::VIEW_PITCH:
		case CSequenceLayer::VIEW_PITCH_OFFSET:
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
		case CSequenceLayer::VIEW_MODULATION:
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
	// Move cursor left / right for encoder event
	void cursor_action(CSequenceLayer& layer, ACTION what, int& cursor, byte wrap=0) {
		switch(what) {
		case ACTION_ENC_RIGHT:
			if(cursor < GRID_WIDTH-1) {
				++cursor;
			}
			else if(wrap) {
				cursor = 0;
			}
			break;
		case ACTION_ENC_LEFT:
			if(cursor > 0) {
				--cursor;
			}
			else if(wrap) {
				cursor = GRID_WIDTH-1;
			}
			break;
		default:
			break;
		}
	}


	////////////////////////////////////////////////
	void command_prompt() {
		if(m_cmd_prompt) {
			int count = m_edit_value;
			const char *ch = m_cmd_prompt;
			while(*ch) {
				char text[8];
				int len = 0;
				while(*ch && *ch != '|' && len<8) {
					text[len++] = *ch++;
				}
				if(!count) {
					g_popup.text(text, len);
					g_popup.no_hide();
					break;
				}
				if(!*ch) {
					break;
				}
				++ch;
				--count;
			}
		}
	}

	////////////////////////////////////////////////
	void command_mode(COMMAND cmd) {
		m_command = cmd;
		m_cmd_prompt = NULL;
		switch(cmd) {
		case CMD_CLEAR_PAGE:
		case CMD_CLEAR_LAYER:
			m_edit_value = 0;
			m_num_values = 2;
			m_cmd_prompt = "SURE? NO|SURE?YES";
			break;
		}
		command_prompt();
	}

	////////////////////////////////////////////////
	void command_action(CSequenceLayer& layer, ACTION what) {
		switch(what) {
			////////////////////////////////////////////////
		case ACTION_ENC_LEFT:
			if(m_edit_value>0) {
				--m_edit_value;
				command_prompt();
			}
			break;
		case ACTION_ENC_RIGHT:
			if(m_edit_value<m_num_values-1) {
				++m_edit_value;
				command_prompt();
			}
			break;
		case ACTION_END:
			if(exec_command(layer, m_command, m_edit_value)) {
				g_popup.text("DONE",4);
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
	}

	///////////////////////////////////////////////////////////////////////////////
	void toggle(PARAM_ID param, const char*text) {
		if(m_combo_clicks>1) {
			set(param, !get(param));
		}
		m_cmd_prompt = text;
		m_edit_value = get(param)?1:0;
		command_prompt();
	}

	///////////////////////////////////////////////////////////////////////////////
	void toggle_done() {
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
			set_scroll_for(layer, step.get_value());
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
				// fine edit in mod mode
			case KEY_CV|KEY2_CV_FINE:
				// fine adjustment of value. show the new value and copy
				value_action(layer, step, what, 1);
				layer.set_step(m_cur_page, m_cursor, step);
				set_scroll_for(layer, step.get_value());
				show_step_value(layer, step.get_value());
				break;
			case KEY_CV|KEY2_CV_MOVE_VERT:
				// action to shift all points up or down
				if(what == ACTION_ENC_LEFT) {
					if(layer.shift_vertical(m_cur_page, -1, m_cfg.m_scaled_pitch)) {
						--m_edit_value;
					}
				}
				else {
					if(layer.shift_vertical(m_cur_page, +1, m_cfg.m_scaled_pitch)) {
						++m_edit_value;
					}
				}
				g_popup.show_offset(m_edit_value);
				break;
			case KEY_CV|KEY2_CV_MOVE_HORZ:
				// action to shift all points left or right
				if(what == ACTION_ENC_LEFT) {
					if(--m_edit_value <= -(GRID_WIDTH-1)) {
						m_edit_value = -(GRID_WIDTH-1);
					}
					layer.shift_horizontal(m_cur_page, -1);
				}
				else {
					if(++m_edit_value >= GRID_WIDTH-1) {
						m_edit_value = 0;
					}
					layer.shift_horizontal(m_cur_page, +1);
				}
				g_popup.show_offset(m_edit_value);
				break;
			default:
				if(!step.is_data_point()) {
					// the first turn sets as a data point
					step.set_data_point(1);
				}
				else {
					value_action(layer, step, what, 0);				// change the value
				}
				// editing a step value
				layer.set_step(m_cur_page, m_cursor, step);
				set_scroll_for(layer, step.get_value());
				show_step_value(layer, step.get_value());
				break;
			}
			break;
		////////////////////////////////////////////////
		case ACTION_KEY_COMBO:
			switch(m_key_combo) {
			case KEY_CV|KEY2_CV_MOVE_VERT:
				m_edit_value = 0;
				g_popup.text("VERT", 4);
				g_popup.align(CPopup::ALIGN_RIGHT);
				break;
			case KEY_CV|KEY2_CV_MOVE_HORZ:
				m_edit_value = 0;
				g_popup.text("HORZ", 4);
				g_popup.align(CPopup::ALIGN_RIGHT);
				break;
			}
			break;
		default:
			break;
		}
		return 0;
	}

	///////////////////////////////////////////////////////////////////////////////
	// PASTE BUTTON
	void clone_action(CSequenceLayer& layer, ACTION what) {
		//CSequencePage& page = layer.get_page(m_cur_page);
		switch(what) {
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
			////////////////////////////////////////////////
		case ACTION_ENC_LEFT:
		case ACTION_ENC_RIGHT:
			if(m_clone_status == CLONE_NONE) {
				CSequenceStep source =	layer.get_step(m_cur_page, m_cursor);
				cursor_action(layer, what, m_cursor, 1);
				layer.set_step(m_cur_page, m_cursor, source);
			}
			else {
				CSequenceStep source =	layer.get_step(m_cur_page, m_clone_source);
				layer.set_step(m_cur_page, m_cursor, source);
				cursor_action(layer, what, m_cursor, 1);
				cursor_action(layer, what, m_clone_source, 1);
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
			layer.clear_step(m_cur_page, m_cursor);
			cursor_action(layer, what, m_cursor);
			break;
		case ACTION_END:
			if(!m_encoder_moved) {
				layer.clear_step(m_cur_page, m_cursor);
				cursor_action(layer, what, m_cursor);
			}
			break;
		default:
			break;
		}
	}


	///////////////////////////////////////////////////////////////////////////////
	// GATE BUTTON
	void gate_action(CSequenceLayer& layer, ACTION what) {
		switch(what) {
		////////////////////////////////////////////////
		case ACTION_ENC_LEFT:
			if(m_gate_view) {
				--m_gate_view;
			}
			show_gate_view();
			break;
		////////////////////////////////////////////////
		case ACTION_ENC_RIGHT:
			if(m_gate_view < GATE_VIEW_MAX) {
				++m_gate_view;
			}
			show_gate_view();
			break;
		////////////////////////////////////////////////
		case ACTION_CLICK:
		{
			//CSequencePage& page = layer.get_page(m_cur_page);
			CSequenceStep step = layer.get_step(m_cur_page, m_cursor);
			switch(m_gate_view) {
			case GATE_VIEW_GATE:
				step.toggle_gate();
				break;
			case GATE_VIEW_TIE:
				step.toggle_tie();
				break;
			case GATE_VIEW_PROB:
				step.inc_prob();
				show_gate_prob(step.get_prob());
				break;
			case GATE_VIEW_RETRIG:
				step.inc_retrig();
				show_gate_retrig(step.get_retrig());
				break;
			}
			layer.set_step(m_cur_page, m_cursor, step);
			break;
		}
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
			cursor_action(layer, what, m_cursor);
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
				case KEY_LOOP|KEY2_PAGE_A:
					page = 0;
					break;
				case KEY_LOOP|KEY2_PAGE_B:
					page = 1;
					break;
				case KEY_LOOP|KEY2_PAGE_C:
					page = 2;
					break;
				case KEY_LOOP|KEY2_PAGE_D:
					page = 3;
					break;
				}
				if(page>=0) {
					if(m_combo_clicks<2) {
						layer.clear_page_list();
					}
					if(layer.add_to_page_list(page)) {
						g_popup.num2digits(layer.get_page_list_count());
						g_popup.align(CPopup::ALIGN_RIGHT);
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
			if(m_encoder_moved && m_edit_value > 0) {
				--m_edit_value;
			}
			show_page_list(m_edit_value);
			break;
		case ACTION_ENC_RIGHT:
			if(m_encoder_moved && m_edit_value < CSequenceLayer::MAX_PAGES-1) {
				++m_edit_value;
			}
			show_page_list(m_edit_value);
			break;
		case ACTION_KEY_COMBO: {

				int cur_page = -1;
				switch(m_key_combo) {
				case KEY_PAGE|KEY2_PAGE_A:
					cur_page = 0;
					break;
				case KEY_PAGE|KEY2_PAGE_B:
					cur_page = 1;
					break;
				case KEY_PAGE|KEY2_PAGE_C:
					cur_page = 2;
					break;
				case KEY_PAGE|KEY2_PAGE_D:
					cur_page = 3;
					break;
				case KEY_PAGE|KEY2_PAGE_ADVANCE:
					break;
				}
				if(cur_page >= 0) {
					m_cur_page = cur_page;
					layer.prepare_page(m_cur_page);
					m_edit_value = layer.get_max_page_no(); // we might have added new pages above...
					if(!layer.get(P_SQL_AUTO_PAGE_ADVANCE)) {
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
	void run_action(CSequenceLayer& layer, ACTION what) {
		switch(what) {
		case ACTION_BEGIN:
			toggle_init();
			break;
		case ACTION_KEY_COMBO:
			switch(m_key_combo) {
			case KEY_RUN|KEY_RUN_SCALE_MODE:
				toggle(P_EDIT_SCALE_GRID, "CHRO EDT|SCAL EDT");
				break;
			case KEY_RUN|KEY_RUN_AUTO_GATE:
				toggle(P_EDIT_AUTO_GATE_INSERT, "MAN GATE|AUTO GAT");
				break;
			case KEY_RUN|KEY_RUN_INTERPOLATE:
				toggle(P_SQL_INTERPOLATE, "FILL STP|FILL CUR");
				break;
			case KEY_RUN|KEY_RUN_GRID:
				toggle(P_EDIT_SHOW_GRID, "GRID OFF|GRID ON");
				break;
			case KEY_RUN|KEY_RUN_LOOP_MODE:
				toggle(P_SQL_LOOP_PER_PAGE, "LOOP COM|LOOP IND");
				break;
			case KEY_RUN|KEY_RUN_PAGE_ADV:
				toggle(P_SQL_AUTO_PAGE_ADVANCE, "PAG FGD|PAG SEQ");
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
	// function to dispatch an action to the correct handler based on which
	// key has been pressed
	void action(CSequenceLayer& layer, ACTION what) {
		if(m_command) { // a command is in progress, so until
			command_action(layer, what);
		}
		else {
			switch(m_action_key) {
			case KEY_CV: cv_action(layer, what); break;
			case KEY_CLONE: clone_action(layer, what); break;
			case KEY_CLEAR: clear_action(layer, what); break;
			case KEY_GATE: gate_action(layer, what); break;
			case KEY_LOOP: loop_action(layer, what); break;
			case KEY_PAGE: page_action(layer, what); break;
			case KEY_LAYER: layer_action(layer, what); break;
			case KEY_RUN: run_action(layer, what); break;
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
		case P_EDIT_SCALE_GRID: return !!m_cfg.m_scaled_pitch;
		default:
			return g_sequence.get_layer(m_cur_layer).get(param);
		}
	}
	///////////////////////////////////////////////////////////////////////////////
	void set(PARAM_ID param, int value) {
		switch(param) {
		case P_EDIT_AUTO_GATE_INSERT: m_cfg.m_auto_gate = !!value; break;
		case P_EDIT_SHOW_GRID: m_cfg.m_show_grid = !!value; break;
		case P_EDIT_SCALE_GRID: m_cfg.m_scaled_pitch = !!value; break;
		default:
			g_sequence.get_layer(m_cur_layer).set(param, value);
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	int is_valid_param(PARAM_ID param) {
		switch(param) {
			case P_EDIT_AUTO_GATE_INSERT:
			case P_EDIT_SHOW_GRID:
			case P_EDIT_SCALE_GRID:
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
				switch(param) {
				case KEY_CV:
				case KEY_CLONE:
				case KEY_CLEAR:
				case KEY_GATE:
				case KEY_LOOP:
				case KEY_PAGE:
				case KEY_LAYER:
				case KEY_RUN:
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
					cursor_action(layer, what, m_cursor);
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
			if(layer.get_view() == CSequenceLayer::VIEW_MODULATION) {
				n = 12 - step.get_value()/10;
				if(n<0) {
					n=0;
				}
			}
			else {
				if(layer.get_view() == CSequenceLayer::VIEW_PITCH && m_cfg.m_scaled_pitch) {
					n = layer.get_scale().note_to_index(n);
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
			switch(m_gate_view) {
				case GATE_VIEW_GATE:
					if(step.is_gate()) {
						bri = BRIGHT_MED;
					}
					else if(step.is_tied()){
						bri = BRIGHT_LOW;
					}
					break;
				case GATE_VIEW_TIE:
					if(step.is_tied()){
						bri = BRIGHT_MED;
					}
					else if(step.is_gate()) {
						bri = BRIGHT_LOW;
					}
					break;
				case GATE_VIEW_PROB:
					if(step.get_prob() != CSequenceStep::PROB_OFF) {
						bri = BRIGHT_MED;
					}
					else if(step.is_gate()){
						bri = BRIGHT_LOW;
					}
					break;
				case GATE_VIEW_RETRIG:
					if(step.get_retrig() != CSequenceStep::RETRIG_OFF) {
						bri = BRIGHT_MED;
					}
					else if(step.is_gate()){
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
					g_ui.raster(15) |= 0b01;
					break;
				case 3:
					g_ui.raster(15) |= 0b10;
					break;
			}
		}
	}


	void run() {
		CSequenceLayer& layer = g_sequence.get_layer(m_cur_layer);
		if(layer.is_page_advanced() && layer.get_page_list_count()!=1) {
			m_ppi_timeout = PPI_MS;
		}
		else if(m_ppi_timeout) {
			--m_ppi_timeout;
		}
	}
};
CSequenceEditor g_sequence_editor;


#endif /* SEQUENCE_EDITOR_H_ */
