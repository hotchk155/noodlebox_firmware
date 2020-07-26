//////////////////////////////////////////////////////////////////////////////
// sixty four pixels 2020                                       CC-NC-BY-SA //
//                                //  //          //                        //
//   //////   /////   /////   //////  //   /////  //////   /////  //   //   //
//   //   // //   // //   // //   //  //  //   // //   // //   //  // //    //
//   //   // //   // //   // //   //  //  /////// //   // //   //   ///     //
//   //   // //   // //   // //   //  //  //      //   // //   //  // //    //
//   //   //  /////   /////   //////   //  /////  //////   /////  //   //   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// SEQUENCE EDITOR VIEW
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
#ifndef SEQUENCE_EDITOR_H_
#define SEQUENCE_EDITOR_H_

// This class provides the user interface for editing one layer of the sequence
class CSequenceEditor {

	// misc constants
	enum {
		GRID_WIDTH = 32,		// number of columns in the grid
		GRID_HEIGHT = 16,		// number of rows in the grid
		POPUP_MS = 2000,		// how long popup window is to be displayed
		PPI_MS = 100,			// play page indicator timeout
		MAX_MIDI_IN_NOTES = 10, 	// max number of held midi notes that we can track
	};

	// enumeration of the "gestures" or actions that the user can perform
	typedef enum:byte {
		ACTION_NONE,			// no action in progress
		ACTION_BEGIN,			// start of an action, when PRIMARY button is pressed
		ACTION_ENC_LEFT,		// encoder is turned anticlockwise
		ACTION_ENC_RIGHT,   	// encoder is turned clockwise
		ACTION_HOLD,			// button has been held down for a certain period with no encoder turn
		ACTION_CLICK,			// button pressed and release without encoder turn
		ACTION_COMBO_BEGIN,		// SECONDARY button pressed while PRIMARY button held
		ACTION_COMBO_END,		// SECONDARY button released while PRIMARY button still  held
		ACTION_KEY_DOWN_RAW,		// Additional keys pressed while PRIMARY and SECONDARY keys already pressed
		ACTION_END				// end of an action, when PRIMARY button is released
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
		CMD_MOVE_LAYER,
		CMD_MEMORY
	} COMMAND;

	enum {
		GATE_VIEW_GATE_TIE,
		GATE_VIEW_ACCENT,
		GATE_VIEW_PROB,
		GATE_VIEW_RETRIG,
		GATE_VIEW_HOLD,
		GATE_VIEW_MAX = GATE_VIEW_RETRIG
	};

	enum {
		BRIGHT_OFF,
		BRIGHT_LOW,
		BRIGHT_MED,
		BRIGHT_HIGH
	};

	enum {
		CLONE_MARKED	= 0x01,				// a clone point has been marked
		CLONE_ACTIONED  = 0x02,				// data has been cloned from clone point
		CLONE_RETAIN	= 0x04				// the clone point can be retained when current action ends
	};

	typedef struct {
		//V_SQL_MIDI_IN_MODE m_midi_in_mode;
		V_SQL_MIDI_IN_CHAN m_midi_in_chan;
		int m_show_grid:1;
		int m_auto_gate:1;
	} CONFIG;
	CONFIG m_cfg;

	typedef struct {
		int m_layer;
		int m_page;
		int m_step;
	} LOCATION;

	//
	// MEMBER VARIABLES
	//
	ACTION m_action;			// the action being performed by the user
	uint32_t m_action_key;		// the key to which the action applies
	uint32_t m_last_action_key;
	uint32_t m_key_combo;		// keys pressed in conjunction with edit shift
	uint32_t m_key_down_raw;
	byte m_combo_clicks;
	int m_cursor;				// position of the vertical cursor bar
	int m_edit_value;			// the value being edited (e.g. shift offset)
	PARAM_ID m_edit_param;
	COMMAND m_command;
	const char *m_cmd_prompt;
	const char *m_cmd_values;
	int m_num_values;
	int m_rand_seed;

	int m_sel_from;				// start of selection range
	int m_sel_to;				// end of selection range
	int m_gate_view;			// which gate layer is being viewed
	int m_gate_edit_step;		// the starting column for GATE+encoder operation
	LOCATION m_clone_source;	// the clone source point
	byte m_clone_flags;			// flags controlling state of clone source point
	byte m_cur_layer;			// the layer number that is being viewed
	byte m_cur_page;			// the page within the layer that is being viewed
	byte m_memo_slot;
	byte m_ppi_timeout;			// play page indicator timeout

	int m_edit_mutes:1;		// are we currently editing mutes
	int m_encoder_moved:1;		// whether encoder has been previously moved since action was in progress
	int m_rec_arm:1;
	byte m_midi_in_note[MAX_MIDI_IN_NOTES];
	int m_num_midi_in_notes;



	CSequenceStep m_clone_step;	// during clone operation..
	CSequencePage m_save_page; // during randomization
	//
	// PRIVATE METHODS
	//

	///////////////////////////////////////////////////////////////////////////////
	// initialise everything
	void init_state() {
		m_cursor = 0;
		m_sel_from = -1;
		m_sel_to = -1;
		m_gate_view = GATE_VIEW_GATE_TIE;
		m_clone_flags = 0;
		m_cur_layer = 0;
		m_cur_page = 0;
		m_memo_slot = 0;
		m_rand_seed = 0;
		m_rec_arm = 0;
		m_num_midi_in_notes = 0;
	}

	///////////////////////////////////////////////////////////////////////////////
	void init_config() {
		//m_cfg.m_midi_in_mode = V_SQL_MIDI_IN_MODE_NONE;
		m_cfg.m_midi_in_chan = V_SQL_MIDI_IN_CHAN_OMNI;
		m_cfg.m_show_grid = 1;
		m_cfg.m_auto_gate = 1;
	}

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
	void show_gate_hold(CSequenceStep& step) {
		g_popup.text("HLD.");
		g_popup.num2digits(step.get_step_count(),1);
		g_popup.avoid(m_cursor);
	}

	///////////////////////////////////////////////////////////////////////////////
	void show_gate_accent(CSequenceStep& step) {
		g_popup.text(step.is(CSequenceStep::ACCENT_POINT) ? "ACC:ON" : "ACC:OFF");
		g_popup.avoid(m_cursor);
	}

	///////////////////////////////////////////////////////////////////////////////
	void show_layer_mutes() {
		g_popup.text("L");
		g_popup.text(g_sequence.get_layer(0).is_muted()? "$":"1",1);
		g_popup.text(g_sequence.get_layer(1).is_muted()? "$":"2",1);
		g_popup.text(g_sequence.get_layer(2).is_muted()? "$":"3",1);
		g_popup.text(g_sequence.get_layer(3).is_muted()? "$":"4",1);
		g_popup.avoid(m_cursor);
		g_popup.no_hide();
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
		static char text[3];
		switch(m_memo_slot) {
			case SLOT_TEMPLATE:
				text[0] = 'T';
				text[1] = 'M';
				break;
			default:
				text[0] = 'M' ;
				text[1] = '1' + (m_memo_slot - SLOT_PATCH1);
		}
		text[2] = '\0';
		return text;
	}

	///////////////////////////////////////////////////////////////////////////////
	void show_layer() {
		g_popup.text(get_layer());
		g_popup.avoid(m_cursor);
	}

	///////////////////////////////////////////////////////////////////////////////
	void show_layer_page() {
		g_popup.text(get_layer_page());
		if(g_sequence.get_layer(m_cur_layer).is_muted()) {
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
	void show_trig_density(int count, int steps) {
		g_popup.num2digits(count);
		g_popup.text(":",1);
		g_popup.num2digits(steps,1);
	}

	///////////////////////////////////////////////////////////////////////////////
	void show_swing(int value) {
		g_popup.num2digits(value);
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
				g_ui.hilite(row) = 0x88888888U;
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
		int min_value = 0;
		int max_value = 127;
		int value = step.get_value();
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
				if(value < 0) {
					value = 0;
				}
				if(value > CScale::instance().max_index()) {
					value = CScale::instance().max_index();
				}
				value = CScale::instance().index_to_note(value);
				break;
			} // else fall thru
		case V_SQL_SEQ_MODE_OFFSET:
		default:
			min_value = CSequenceLayer::OFFSET_MIN;
			max_value = CSequenceLayer::OFFSET_MAX;
			value += delta;
			break;
		}
		if(value<min_value) {
			value = min_value;
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
		case CMD_MOVE_LAYER:
			m_edit_value = 1 + m_cur_layer;
			m_num_values = 5;
			m_cmd_prompt = get_layer();
			switch(m_cur_layer) {
			case 1:
				m_cmd_values = ">CXL|>.134|>1.34|>13.4|>134.";
				break;
			case 2:
				m_cmd_values = ">CXL|>.124|>1.24|>12.4|>124.";
				break;
			case 3:
				m_cmd_values = ">CXL|>.123|>1.23|>12.3|>123.";
				break;
			default:
			case 0:
				m_cmd_values = ">CXL|>.234|>2.34|>23.4|>234.";
				break;
			}
			break;
		case CMD_MEMORY:
			m_edit_value = 1;
			m_num_values = 3;
			m_cmd_prompt = get_memo_slot();
			m_cmd_values = ":CXL|:LOAD|:SAVE";
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
			int value = ::get(param)+1;
			if(value > num_values-1) {
				value = 0;
			}
			::set(param, value);
		}
		m_cmd_prompt = prompt;
		m_cmd_values = values;
		m_edit_value = ::get(param);
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
				layer.silence();
				layer.init();
				m_cur_page = 0;
				return 1;
			}
			break;
		case CMD_CLONE_PAGE:
			if(value) {
				byte target_layer = (value-1)/4;
				byte target_page = (value-1)%4;
				if(target_layer != m_cur_layer || target_page != m_cur_page) {
					layer.get_page_content(m_cur_page, m_save_page);
					g_sequence.get_layer(target_layer).set_page_content(target_page, m_save_page);
					return 1;
				}
			}
			break;
		case CMD_CLONE_LAYER:
			if(value) {
				byte target_layer = value - 1;
				if(target_layer != m_cur_layer) {
					g_sequence.get_layer(target_layer).silence();
					g_sequence.get_layer(target_layer).set_content(g_sequence.get_layer(m_cur_layer));
 					return 1;
				}
			}
			break;
		case CMD_MOVE_LAYER:
			if(value) {
				byte target_layer = value - 1;
				if(target_layer != m_cur_layer) {
					g_sequence.get_layer(m_cur_layer).silence();
					g_sequence.get_layer(target_layer).silence();
					g_sequence.move_layer(m_cur_layer, target_layer);
					m_cur_layer = target_layer;
 					return 1;
				}
			}
			break;
		case CMD_MEMORY:
			if(m_memo_slot) {
				if(value == 1) {
					g_sequence.silence();
					g_sequence.load_patch(m_memo_slot);
				}
				else if(value == 2) {
					g_sequence.save_patch(m_memo_slot);
				}
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
			if(layer.get_mode() != V_SQL_SEQ_MODE_PITCH
				|| layer.any_data_points(m_cur_page)) {
				layer.set_scroll_for(step.get_value());
				show_step_value(layer, step.get_value());
			}
			// when button first pressed, current step
			// scrolled into view and described
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
				if(!step.is(CSequenceStep::DATA_POINT)) {
					if(!layer.any_data_points(m_cur_page)) {
						step.set_value(layer.get_default_value());
					}
					step.set(CSequenceStep::DATA_POINT,1);
					if(m_cfg.m_auto_gate) {
						step.set(CSequenceStep::TRIG_POINT,1);
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
		case ACTION_COMBO_BEGIN:
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
		int loop_min, loop_max, loop_span;
		CSequenceStep step = layer.get_step(m_cur_page, m_cursor);
		switch(what) {
		////////////////////////////////////////////////
		case ACTION_BEGIN:
			m_edit_value = 0;
			m_gate_edit_step = m_cursor;
			break;
		case ACTION_COMBO_BEGIN:
			switch(m_key_combo) {
			case KEY_GATE|KEY2_GATE_PROB:
				m_gate_view = GATE_VIEW_PROB;
				show_gate_prob(step);
				break;
			case KEY_GATE|KEY2_GATE_RETRIG:
				m_gate_view = GATE_VIEW_RETRIG;
				show_gate_retrig(step);
				break;
			case KEY_GATE|KEY2_GATE_HOLD:
				m_gate_view = GATE_VIEW_HOLD;
				show_gate_hold(step);
				break;
			case KEY_GATE|KEY2_GATE_VEL:
				m_gate_view = GATE_VIEW_ACCENT;
				show_gate_accent(step);
				break;
			case KEY_GATE|KEY2_GATE_REPLACE:
				loop_span = layer.get_loop_span(m_cur_page, &loop_min, &loop_max);
				m_edit_value = layer.count_of(m_cur_page, CSequenceStep::TRIG_POINT, loop_min, loop_max);
				show_trig_density(m_edit_value, loop_span);
				break;
			case KEY_GATE|KEY2_GATE_SWING:
				m_edit_value = layer.get(P_SQL_OFF_GRID_AMOUNT);
				show_swing(m_edit_value);
				break;
			}
			break;
		case ACTION_END:
		case ACTION_COMBO_END:
			m_gate_view = GATE_VIEW_GATE_TIE;
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
			case KEY_GATE|KEY2_GATE_HOLD:
				step.set_hold(inc_value(what, step.get_hold(), 0, CSequenceStep::HOLD_MAX, 0));
				layer.set_step(m_cur_page, m_cursor, step);
				show_gate_hold(step);
				m_gate_view = GATE_VIEW_HOLD;
				break;
			case KEY_GATE|KEY2_GATE_VEL:
				step.set(CSequenceStep::ACCENT_POINT, (what == ACTION_ENC_RIGHT));
				layer.set_step(m_cur_page, m_cursor, step);
				show_gate_accent(step);
				m_gate_view = GATE_VIEW_ACCENT;
				break;
			case KEY_GATE|KEY2_GATE_REPLACE:
				loop_span = layer.get_loop_span(m_cur_page, &loop_min, &loop_max);
				if(encoder_action(what, m_edit_value, 0, loop_span, 0)) {
					layer.replace_gates(m_cur_page, m_edit_value, loop_span, loop_min);
					show_trig_density(m_edit_value, loop_span);
				}
				break;
			case KEY_GATE|KEY2_GATE_SWING:
				if(encoder_action(what, m_edit_value, CSequenceLayer::MOD_AMOUNT_MIN, CSequenceLayer::MOD_AMOUNT_MAX, 0)) {
					layer.set(P_SQL_OFF_GRID_AMOUNT, m_edit_value);
					show_swing(m_edit_value);
				}
				break;
			default:
				if(what == ACTION_ENC_RIGHT) {
					if(m_edit_value > 0) {
						if(m_cursor < GRID_WIDTH-1) {
							++m_cursor;
							step = layer.get_step(m_cur_page, m_cursor);
							step.set(CSequenceStep::TIE_POINT, 1);
							layer.set_step(m_cur_page, m_cursor, step);
						}
					}
					else if(m_cursor == m_gate_edit_step) {
						step = layer.get_step(m_cur_page, m_cursor);
						step.set(CSequenceStep::TIE_POINT, 1);
						layer.set_step(m_cur_page, m_cursor, step);
						m_edit_value = 1; // working to right of start position
					}
					else {
						++m_cursor;
					}
				}
				else {
					if(m_edit_value < 0) {
						if(m_cursor > 0) {
							step = layer.get_step(m_cur_page, m_cursor);
							step.set(CSequenceStep::TIE_POINT, 0);
							layer.set_step(m_cur_page, m_cursor, step);
							--m_cursor;
						}
					}
					else if(m_cursor == m_gate_edit_step) {
						step = layer.get_step(m_cur_page, m_cursor);
						step.set(CSequenceStep::TIE_POINT, 0);
						layer.set_step(m_cur_page, m_cursor, step);
						m_edit_value = -1;
					}
					else {
						step = layer.get_step(m_cur_page, m_cursor);
						step.set(CSequenceStep::TIE_POINT, 0);
						layer.set_step(m_cur_page, m_cursor, step);
						--m_cursor;
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
				step.set(CSequenceStep::TRIG_POINT, !step.is(CSequenceStep::TRIG_POINT));
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
			if(!(m_clone_flags & CLONE_MARKED)) {
				// turn on marking of the clone point
				m_clone_source.m_layer = m_cur_layer;
				m_clone_source.m_page = m_cur_page;
				m_clone_source.m_step = m_cursor;
				m_clone_flags = CLONE_MARKED|CLONE_RETAIN;
			}
			else {
				// turn off clone point marking
				m_clone_flags = 0;
			}
			break;
		case ACTION_COMBO_BEGIN:
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
			if(!(m_clone_flags & CLONE_MARKED)) {
				// the clone point has not been marked
				cursor_action(what, 1);
				layer.set_step(m_cur_page, m_cursor, m_clone_step, data_type, 1);
			}
			else {
				CSequenceStep source = g_sequence.get_layer(m_clone_source.m_layer).get_step(m_clone_source.m_page, m_clone_source.m_step);
				layer.set_step(m_cur_page, m_cursor, source, data_type, 1);
				cursor_action(what, 1);
				encoder_action(what, m_clone_source.m_step, 0, GRID_WIDTH-1, 1);
				m_clone_flags |= CLONE_ACTIONED;
			}
			break;
		case ACTION_END:
			// if we have not cloned data then we can retain the clone source
			// points, otherwise it will be cleared  (at the end of the clone)
			if(!(m_clone_flags & CLONE_ACTIONED)) {
				m_clone_flags |= CLONE_RETAIN;
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
		case ACTION_COMBO_BEGIN:
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
	void rand_action(CSequenceLayer& layer, ACTION what) {
		switch(what) {
		case ACTION_BEGIN:
			// capture the page state so we can revert back to it
			layer.get_page_content(m_cur_page, m_save_page);
			m_rand_seed = g_clock.get_ms();
			m_edit_value = 0;
			break;
		////////////////////////////////////////////////
		case ACTION_ENC_LEFT:
		case ACTION_ENC_RIGHT:
			switch(m_key_combo) {
			// creating a new random page?
			case KEY_RAND|KEY2_RAND_CREATE:
				if(encoder_action(what, m_edit_value, 0, 100, 0)) {
					layer.randomise_page(m_cur_page, m_rand_seed + m_edit_value);
				}
				g_popup.num3digits(m_edit_value);
				break;
			// adding noise to an existing page
			default:
				if(encoder_action(what, m_edit_value, -100, 100, 0)) {
					layer.set_page_content(m_cur_page, m_save_page);
					layer.add_noise_to_page(m_cur_page, m_rand_seed, m_edit_value);
				}
				g_popup.num3digits(m_edit_value);
				break;
			}
			break;
			////////////////////////////////////////////////
		case ACTION_COMBO_BEGIN:
			{
				int page_no = -1;
				switch(m_key_combo) {
					// commit randomness to page slot
					case KEY_RAND|KEY2_RAND_SAVE_A:
						page_no = 0;
						break;
					case KEY_RAND|KEY2_RAND_SAVE_B:
						page_no = 1;
						break;
					case KEY_RAND|KEY2_RAND_SAVE_C:
						page_no = 2;
						break;
					case KEY_RAND|KEY2_RAND_SAVE_D:
						page_no = 3;
						break;
					case KEY_RAND|KEY2_RAND_SAVE_CUR:
						// commit to current page slot. This needs to update the saved
						// page image and reset the level parameter
						layer.get_page_content(m_cur_page, m_save_page);
						m_edit_value = 0;
						g_popup.text("DONE");
						g_popup.avoid(m_cursor);
						break;
				}
				if(page_no >= 0) {
					// we cannot use this method to save to the current page because
					// the changes will be overwritten by the saved page image
					if(page_no != m_cur_page) {
						CSequencePage this_page;
						layer.get_page_content(m_cur_page, this_page);
						layer.set_page_content(page_no, this_page);
						g_popup.text("DONE");
						g_popup.avoid(m_cursor);
					}
				}
			}
			break;
		////////////////////////////////////////////////
		case ACTION_END:
			// restore the saved page image
			layer.set_page_content(m_cur_page, m_save_page);
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
		case ACTION_COMBO_BEGIN:
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
					if(layer.is_cue_mode()) {
						layer.cue_cancel();
						layer.set_play_page(m_cur_page);
						g_popup.text("=OFF");
					}
					else {
						layer.cue_first(m_cur_page);
						g_popup.text("=BKG");
					}
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
		case ACTION_COMBO_BEGIN: {
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
				case KEY_PAGE|KEY2_PAGE_MOVE_LAYER:
					command_mode(CMD_MOVE_LAYER);
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
				m_clone_flags |= CLONE_RETAIN;
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
		case ACTION_BEGIN:
			m_edit_mutes = 0;
			break;
		case ACTION_END:
			if(m_edit_mutes) {
				g_popup.hide();
			}
			break;
		case ACTION_ENC_LEFT:
			scroll(layer, -1);
			break;
		case ACTION_ENC_RIGHT:
			scroll(layer, +1);
			break;
		case ACTION_KEY_DOWN_RAW:
			if(m_edit_mutes) {
				switch(m_key_down_raw) {
				case KEY2_LAYER_1:
					g_sequence.get_layer(0).toggle_mute();
					break;
				case KEY2_LAYER_2:
					g_sequence.get_layer(1).toggle_mute();
					break;
				case KEY2_LAYER_3:
					g_sequence.get_layer(2).toggle_mute();
					break;
				case KEY2_LAYER_4:
					g_sequence.get_layer(3).toggle_mute();
					break;
				case KEY2_LAYER_MUTE:
					g_sequence.get_layer(m_cur_layer).toggle_mute();
					break;
				}
				show_layer_mutes();
			}
			break;

		case ACTION_COMBO_BEGIN:
			if(!m_edit_mutes) {
				m_clone_flags |= CLONE_RETAIN;
				switch(m_key_combo) {
					case KEY_LAYER|KEY2_LAYER_1:
						fire_event(EV_CHANGE_LAYER, 0);
						break;
					case KEY_LAYER|KEY2_LAYER_2:
						fire_event(EV_CHANGE_LAYER, 1);
						break;
					case KEY_LAYER|KEY2_LAYER_3:
						fire_event(EV_CHANGE_LAYER, 2);
						break;
					case KEY_LAYER|KEY2_LAYER_4:
						fire_event(EV_CHANGE_LAYER, 3);
						break;
					case KEY_LAYER|KEY2_LAYER_MUTE:
						m_clone_flags &= ~CLONE_RETAIN;
						m_edit_mutes = 1;
						show_layer_mutes();
						break;
				}
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
		case ACTION_COMBO_BEGIN:
			switch(m_key_combo) {
			case KEY_FUNC|KEY2_FUNC_SCALE_MODE:
				toggle(P_SQL_SCALED_VIEW, "ROWS:", "CHR|SCA");
				break;
			case KEY_FUNC|KEY2_FUNC_AUTO_TRIG:
				toggle(P_SQL_AUTO_GATE_INSERT, "TRIG:", "MAN|AUT");
				break;
			case KEY_FUNC|KEY2_FUNC_INTERPOLATE:
				toggle(P_SQL_FILL_MODE, "FILL:", "OFF|PAD|INT",3);
				break;
			case KEY_FUNC|KEY2_FUNC_GRID:
				toggle(P_SQL_SHOW_GRID, "GRID:", "HID|SHO");
				break;
			case KEY_FUNC|KEY2_FUNC_REC_MODE:
				toggle(P_SEQ_REC_MODE, "REC:", "NONE|CV|TRAN",3);
				break;
			case KEY_FUNC|KEY2_FUNC_LOOP_POINTS:
				toggle(P_SQL_LOOP_PER_PAGE, "LOOP:", "LAY|PAG",2);
				break;
			case KEY_FUNC|KEY2_FUNC_REC_ARM:
				toggle(P_SEQ_REC_ARM, "ARM:", "OFF|ON");
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
		case ACTION_HOLD:
			if(m_memo_slot >= SLOT_PATCH1) {
				g_popup.text(get_memo_slot());
			}
			break;
		case ACTION_COMBO_BEGIN:
			m_memo_slot = 0;
			switch(m_key_combo) {
			case KEY_MEMO|KEY2_MEMO_1: m_memo_slot = SLOT_PATCH1; break;
			case KEY_MEMO|KEY2_MEMO_2: m_memo_slot = SLOT_PATCH2; break;
			case KEY_MEMO|KEY2_MEMO_3: m_memo_slot = SLOT_PATCH3; break;
			case KEY_MEMO|KEY2_MEMO_4: m_memo_slot = SLOT_PATCH4; break;
			case KEY_MEMO|KEY2_MEMO_5: m_memo_slot = SLOT_PATCH5; break;
			case KEY_MEMO|KEY2_MEMO_6: m_memo_slot = SLOT_PATCH6; break;
			case KEY_MEMO|KEY2_MEMO_7: m_memo_slot = SLOT_PATCH7; break;
			case KEY_MEMO|KEY2_MEMO_8: m_memo_slot = SLOT_PATCH8; break;
			case KEY_MEMO|KEY2_MEMO_TEMPLATE: m_memo_slot = SLOT_TEMPLATE; break;
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

		if(what == ACTION_BEGIN) {
			m_clone_flags &= ~CLONE_RETAIN;
		}

		if(m_command) { // a command is in progress, so until
			command_action(layer, what);
		}
		else {
			switch(m_action_key) {
			case KEY_CV:
				cv_action(layer, what);
				break;
			case KEY_GATE:
				gate_action(layer, what);
				break;
			case KEY_CLONE:
				clone_action(layer, what);
				break;
			case KEY_CLEAR:
				clear_action(layer, what);
				break;
			case KEY_RAND:
				rand_action(layer, what);
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

		// cancel clone status
		if(what == ACTION_END && !(m_clone_flags & CLONE_RETAIN)) {
			m_clone_flags = 0;
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////////
	byte process_midi_in_note(byte note, byte vel) {
		byte result = 0;
		int pos;
		if (vel) {
			// determine the insertion point for the new note
			for (pos = 0; pos < m_num_midi_in_notes; ++pos) {
				if(note > m_midi_in_note[pos]) {
					break;
				}
			}

			// increase count of notes in the buffer if there is space
			if (m_num_midi_in_notes < MAX_MIDI_IN_NOTES) {
				++m_num_midi_in_notes;
			}

			// can the new note be inserted in the buffer? (lower priority notes
			// will be shifted along if there is space, otherwise the lowest
			// note will drop of the buffer)
			if (pos < m_num_midi_in_notes) {

				// shift down along which are after the insertion point
				for(int i = m_num_midi_in_notes - 2; i >= pos; --i) {
					m_midi_in_note[i+1] = m_midi_in_note[i];
				}
				// insert the new note in the buffer
				m_midi_in_note[pos] = note;
				result = (pos==0);
			}
		}
		else { // note off - remove from the buffer

			// search for the note
			for(int i = 0; i < m_num_midi_in_notes; ++i) {
				if(m_midi_in_note[i] == note) {
					result = (i==0);

					// remove the note by shufflng all later notes down
					--m_num_midi_in_notes;
					for(; i<m_num_midi_in_notes; ++i) {
						m_midi_in_note[i] = m_midi_in_note[i+1];
					}
				}
			}
		}
		return result;
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
		case P_SQL_AUTO_GATE_INSERT: return !!m_cfg.m_auto_gate;
		case P_SQL_SHOW_GRID: return !!m_cfg.m_show_grid;
		//case P_EDIT_REC_ARM: return !!m_rec_arm;
		//case P_SQL_MIDI_IN_MODE: return m_cfg.m_midi_in_mode;
		case P_SQL_MIDI_IN_CHAN: return m_cfg.m_midi_in_chan;
		default:
			return g_sequence.get_layer(m_cur_layer).get(param);
		}
	}
	///////////////////////////////////////////////////////////////////////////////
	void set(PARAM_ID param, int value) {
		switch(param) {
		case P_SQL_AUTO_GATE_INSERT: m_cfg.m_auto_gate = !!value; break;
		case P_SQL_SHOW_GRID: m_cfg.m_show_grid = !!value; break;
		case P_SQL_MIDI_IN_CHAN: m_cfg.m_midi_in_chan = (V_SQL_MIDI_IN_CHAN)value; fire_event(EV_MIDI_IN_RESET,0); break;
		default:
			g_sequence.get_layer(m_cur_layer).set(param, value);
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	int is_valid_param(PARAM_ID param) {
		switch(param) {
			case P_SQL_AUTO_GATE_INSERT:
			case P_SQL_SHOW_GRID:
				return 1;
			case P_SQL_OUT_CAL_SCALE:
			case P_SQL_OUT_CAL_OFFSET:
				return ::is_cal_mode();
			default:
				return g_sequence.get_layer(m_cur_layer).is_valid_param(param);
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	void activate() {
		m_action = ACTION_NONE;
		m_action_key = 0;
		m_last_action_key = 0;
		m_key_combo = 0;
		m_key_down_raw = 0;
		m_encoder_moved = 0;
		m_edit_value = 0;
		m_edit_mutes = 0;
		m_command = CMD_NONE;
		m_cmd_prompt = NULL;
		m_cmd_values = NULL;
		m_num_values = 0;
		m_edit_param = P_NONE;

		// calibration is active only inside the menu
		g_sequence.set(P_SEQ_OUT_CAL, V_SEQ_OUT_CAL_NONE);

	}

	/////////////////////////////////////////////////////////////////////////////////////////////////
	void deactivate() {
		if(m_cmd_prompt) {
			m_cmd_prompt = NULL;
			g_popup.hide();
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
				action(layer, ACTION_COMBO_BEGIN);
			}
			break;
		case EV_KEY_DOWN_RAW:
			m_key_down_raw = param;
			action(layer, ACTION_KEY_DOWN_RAW);
			break;
		case EV_KEY_RELEASE:
			if(param & m_key_combo) {
				action(layer, ACTION_COMBO_END);
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
		case EV_CHANGE_LAYER:
			if(param < CSequence::NUM_LAYERS) {
				CSequenceLayer& layer = g_sequence.get_layer(param);
				m_cur_layer = param;
				m_cur_page = layer.is_cue_mode()? 0 : layer.get_play_page();
				show_layer_page();
			}
			break;
		case EV_LOAD_OK:
			// when a patch is loaded, return to layer 1 page A
			m_cur_layer = 0;
			m_cur_page = 0;
			break;
		case EV_MIDI_IN_RESET:
			m_num_midi_in_notes = 0;
			break;
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////////
	void handle_midi_note(byte chan, byte note, byte vel) {

		if(chan == m_cfg.m_midi_in_chan || m_cfg.m_midi_in_chan == V_SQL_MIDI_IN_CHAN_OMNI) {
			if(process_midi_in_note(note,vel)) { // any change to the held note?
				if(m_num_midi_in_notes) {
					g_sequence.midi_note_on(m_cur_layer, note, vel);
				}
				else {
					g_sequence.midi_note_off();
				}
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
		if(!!(m_clone_flags & CLONE_MARKED) &&
				m_clone_source.m_layer == m_cur_layer &&
				m_clone_source.m_page == m_cur_page) {
			mask = g_ui.bit(m_clone_source.m_step);
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
				else if(step.is(CSequenceStep::DATA_POINT)) {
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
			byte trig_or_tie = step.is(CSequenceStep::TRIG_POINT)||step.is(CSequenceStep::TIE_POINT);
			switch(m_gate_view) {
				case GATE_VIEW_GATE_TIE:
					if(step.is(CSequenceStep::TRIG_POINT)) {
						bri = step.is(CSequenceStep::TIE_POINT)? BRIGHT_HIGH : BRIGHT_MED;
					}
					else if(step.is(CSequenceStep::TIE_POINT)){
						bri = BRIGHT_LOW;
					}
					break;
				case GATE_VIEW_PROB:
					if(!!step.get_prob()) {
						bri = trig_or_tie? BRIGHT_HIGH : BRIGHT_MED;
					}
					else if(trig_or_tie){
						bri = BRIGHT_LOW;
					}
					break;
				case GATE_VIEW_RETRIG:
					if(!!step.get_retrig()) {
						bri = trig_or_tie? BRIGHT_HIGH : BRIGHT_MED;
					}
					else if(trig_or_tie){
						bri = BRIGHT_LOW;
					}
					break;
				case GATE_VIEW_HOLD:
					if(!!step.get_hold()) {
						bri = trig_or_tie? BRIGHT_HIGH : BRIGHT_MED;
					}
					else if(trig_or_tie){
						bri = BRIGHT_LOW;
					}
					break;
				case GATE_VIEW_ACCENT:
					if(step.is(CSequenceStep::ACCENT_POINT)) {
						bri = trig_or_tie? BRIGHT_HIGH : BRIGHT_MED;
					}
					else if(trig_or_tie){
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

		if(g_i2c_eeprom.is_busy()) {
			g_ui.raster(0) |= 0b11;
			g_ui.raster(1) |= 0b11;
			g_ui.hilite(0) |= 0b11;
			g_ui.hilite(1) |= 0b11;
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
};
CSequenceEditor g_sequence_editor;


#endif /* SEQUENCE_EDITOR_H_ */
