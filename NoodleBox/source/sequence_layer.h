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
// SEQUENCER LAYER
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
#ifndef SEQUENCE_LAYER_H_
#define SEQUENCE_LAYER_H_

///////////////////////////////////////////////////////////////////////////////
// This class holds all the info for a single layer/part
class CSequenceLayer {

public:

	enum {
		NUM_PAGES = 4,					// number of pages
		MAX_PLAYING_NOTES = 8,
		SCROLL_MARGIN = 3,
		MAX_CUE_LIST = 16,
	};

	enum {
		OFFSET_ZERO = 64,
		OFFSET_MIN = 0,
		OFFSET_MAX = 127,
		MOD_AMOUNT_DEFAULT = 50,
		MOD_AMOUNT_MIN = 25,
		MOD_AMOUNT_MAX = 75
	};

	enum {
		INIT_BLANK,
		INIT_FIRST,
		INIT_LAST
	};

	typedef enum:byte {
		REC_GATE_OFF,
		REC_GATE_TRIG,
		REC_GATE_ON,
		REC_GATE_TIE
	} REC_GATE_STATE;

	typedef struct {
		V_SEQ_REC_MODE mode;
		V_SEQ_REC_ARM arm;
		byte note;
	} REC_SESSION;

private:

	enum :byte {
		NO_MIDI_NOTE = 0xff,
		NO_MIDI_CC_VALUE = 0xff
	};

	enum {
		CUE_NONE,
		CUE_AUTO,
		CUE_RANDOM,
		CUE_MANUAL
	};

	// This structure holds the layer information that gets saved with the patch
	typedef struct {
		byte 			m_cue_list[MAX_CUE_LIST]; 	// cued pages list
		byte			m_cue_list_count;			// index to cued pages list
		byte 			m_cue_mode;
		V_SQL_SEQ_MODE 	m_mode;				// the mode for this layer (note, mod etc)
		V_SQL_QUANTIZE 	m_quantize;	// force to scale
		V_SQL_STEP_RATE m_step_rate;		// step rate setting
		V_SQL_OFF_GRID_MODE 	m_off_grid_mode;
		byte			m_off_grid_amount;
		char			m_transpose;		// manual transpose amount for the layer
		V_SQL_TRIG_DUR	m_trig_dur;
		V_SQL_MIDI_OUT  m_midi_out;
		byte 			m_midi_out_chan;		// MIDI channel
		byte 			m_midi_cc;			// MIDI CC
		byte 			m_midi_cc_smooth;			// MIDI CC
		V_SQL_CVSCALE	m_cv_scale;
		V_SQL_CVSHIFT	m_cv_octave;
		int16_t			m_cv_transpose;
		V_SQL_CVGLIDE	m_cv_glide;
		V_SQL_COMBINE	m_combine_prev;
		byte 			m_midi_vel;
		byte 			m_midi_acc_vel;
		byte 			m_max_page_no;		// the highest numbered active page (0-3)
		V_SQL_FILL_MODE	m_fill_mode;
		byte 			m_scroll_ofs;					// lowest step value shown on grid
		int 			m_scaled_view:1;	// whether the pitch view is 7 rows/oct
		int 			m_loop_per_page:1;
		int 			m_muted:1;
		V_SQL_CV_ALIAS 	m_cv_alias;
		V_SQL_GATE_ALIAS m_gate_alias;

	} CONFIG;
	CSequencePage 	m_page[NUM_PAGES];	// sequencer page

	// Current playback state of the layer - stuff that is not saved
	// as part of the patch
	typedef struct {
		int m_play_page_no;				// the page number being played
		int m_play_pos;
		int m_cue_list_next;				// position of the next cued page within cued pages list
		int m_page_advanced:1;
		int m_first_step:1;				// flag says if we have not played any steps since reset
		long m_midi_cc_value;
		long m_midi_cc_target;
		long m_midi_cc_inc;
		byte m_step_midi_note; 					// midi note for the step
		byte m_step_midi_vel;
		byte m_playing_midi_note; 					// midi note currently playing on on channel
		uint32_t m_gate_timeout;		// this is the number of ms remaining of the current gate pulse
		uint32_t m_step_timeout;		// this is the number of ms remaining of the current full step time
		uint32_t m_retrig_ms;			// this is the number of ms between retriggers
		uint32_t m_retrig_timeout;		// this is the time remaining until the next retrigger
		uint32_t m_trig_dur;					// the duration of the current trigger
		clock::TICKS_TYPE m_next_step_time;
	} STATE;

	CONFIG m_cfg;				// instance of config
	STATE m_state;
	byte m_id;

	//
	// PRIVATE METHODS
	//

	///////////////////////////////////////////////////////////////////////////////
	inline byte clamp7bit(int in) {
		if(in>127) {
			return 127;
		}
		else if(in<0) {
			return 0;
		}
		else {
			return (byte)in;
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// accessor for a page
	inline CSequencePage& get_page(byte page_no) {
		ASSERT(page_no >= 0 && page_no < NUM_PAGES);
		return m_page[page_no];
	}

	///////////////////////////////////////////////////////////////////////////////
	// Calculate the page and step
	byte calc_next_step(int &page_no, int &step_no) {

		byte loop_from = get_loop_from(page_no);
		byte loop_to = get_loop_to(page_no);
		byte page_advance=0;

		// have we reached end of the current page
		if(step_no == loop_to) {
			// do we have a cue list?
			if(m_cfg.m_cue_list_count) {
				byte cue_list_pos = m_state.m_cue_list_next;
				if(++cue_list_pos >= m_cfg.m_cue_list_count - 1) {
					cue_list_pos = 0;
				}
				page_no = m_cfg.m_cue_list[cue_list_pos];
			}
			// back to first step (poss page change)
			step_no = get_loop_from(page_no);
			page_advance = 1;
		}
		else {

			// have not reached end of page yet...
			if(loop_to < loop_from) { // running backwards
				if(--step_no < 0) {
					step_no = CSequencePage::MAX_STEPS-1;
				}
				else if(step_no > loop_from) {
					step_no = loop_from;
				}
				else if(step_no < loop_to) {
					step_no = loop_to;
				}
			}
			else {
				if(++step_no > CSequencePage::MAX_STEPS-1) {
					step_no = 0;
				}
				else if(step_no > loop_to) {
					step_no = loop_to;
				}
				else if(step_no < loop_from) {
					step_no = loop_from;
				}
			}
		}
		return page_advance;
	}

	///////////////////////////////////////////////////////////////////////////////
	void cue_update() {
		switch(m_cfg.m_cue_mode) {
			case CUE_AUTO:
				if(++m_cfg.m_cue_list[0] > m_cfg.m_max_page_no) {
					m_cfg.m_cue_list[0] = 0;
				}
				break;
			case CUE_RANDOM:
				m_cfg.m_cue_list[0] = random()%(m_cfg.m_max_page_no+1);
				break;
			case CUE_MANUAL:
				if(++m_state.m_cue_list_next >= m_cfg.m_cue_list_count) {
					m_state.m_cue_list_next = 0;
				}
				break;
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	void recalc_data_points_all_pages() {
		for(int i=0; i<NUM_PAGES; ++i) {
			CSequencePage& page = get_page(i);
			page.recalc(m_cfg.m_fill_mode, get_zero_value());
		}
	}

public:

	///////////////////////////////////////////////////////////////////////////////
	void init() {
		init_config();
		init_state();
	}

	///////////////////////////////////////////////////////////////////////////////
	void set_id(byte id) {
		m_id = id;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Assign valid default values to the sequence layer configuration (i.e.
	// the data that forms part of a saved sequence)
	void init_config() {
		m_cfg.m_mode 		= V_SQL_SEQ_MODE_PITCH;
		m_cfg.m_quantize 	= V_SQL_SEQ_QUANTIZE_CHROMATIC;
		m_cfg.m_step_rate	= V_SQL_STEP_RATE_16;
		m_cfg.m_off_grid_mode 	= V_SQL_OFF_GRID_MODE_NONE;
		m_cfg.m_off_grid_amount = MOD_AMOUNT_DEFAULT;
		m_cfg.m_trig_dur	= V_SQL_NOTE_DUR_8;
		m_cfg.m_combine_prev= V_SQL_COMBINE_OFF;
		m_cfg.m_transpose	= 0;
		m_cfg.m_midi_out_chan 	= m_id;	// default to midi chans 1-4
		m_cfg.m_midi_cc = 1;
		m_cfg.m_midi_cc_smooth = 0;
		m_cfg.m_muted = 0;
		m_cfg.m_cv_scale = V_SQL_CVSCALE_1VOCT;
		m_cfg.m_cv_octave = V_SQL_CVSHIFT_NONE;
		m_cfg.m_cv_transpose = 0;
		m_cfg.m_cv_glide = V_SQL_CVGLIDE_OFF;
		m_cfg.m_midi_vel = 100;
		m_cfg.m_midi_acc_vel = 127;
		m_cfg.m_fill_mode = V_SQL_FILL_MODE_PAD;
		m_cfg.m_loop_per_page = 0;
		m_cfg.m_midi_out = V_SQL_MIDI_OUT_NONE;
		m_cfg.m_scaled_view = 1;
		m_cfg.m_cv_alias = V_SQL_CV_ALIAS_NONE;
		m_cfg.m_gate_alias = V_SQL_GATE_ALIAS_NONE;
		set_mode(m_cfg.m_mode);
		clear();
	}

	///////////////////////////////////////////////////////////////////////////////
	void clear() {
		for(int i=0; i<NUM_PAGES; ++i) {
			CSequencePage& page = get_page(i);
			page.clear(get_zero_value(), CSequencePage::DEFAULT_LOOP_FROM, CSequencePage::DEFAULT_LOOP_TO);
		}
		m_cfg.m_max_page_no = 0;
		m_cfg.m_cue_list_count = 0;
		m_cfg.m_cue_mode = CUE_NONE;
		set_scroll_for(get_default_value(),1);
	}

	///////////////////////////////////////////////////////////////////////////////
	// Initialise the state of a configured sequence layer (e.g. when the layer
	// has been loaded from EEPROM)
	void init_state() {
		m_state.m_playing_midi_note = NO_MIDI_NOTE;
		m_state.m_step_midi_note = NO_MIDI_NOTE;
		m_state.m_midi_cc_value = NO_MIDI_CC_VALUE;
		m_state.m_play_page_no = 0;

		for(int i=0; i<NUM_PAGES; ++i) {
			m_page[i].init_state();
		}
		reset();
	}


	///////////////////////////////////////////////////////////////////////////////
	// Reset the playback state of the layer
	void reset() {
		m_state.m_next_step_time = clock::TICKS_INFINITY;
		m_state.m_gate_timeout = 0;
		m_state.m_step_timeout = 0;
		m_state.m_page_advanced = 0;
		m_state.m_retrig_ms = 0;
		m_state.m_retrig_timeout = 0;
		m_state.m_trig_dur = 0;
		m_state.m_first_step = 1;

		silence();	// kill outputs
		cue_reset(); // go to first page in the cued sequence
		m_state.m_play_pos = get_loop_from(m_state.m_play_page_no); // position at start of loop window
	}

	///////////////////////////////////////////////////////////////////////////////
	void event(int event, uint32_t param) {
		switch(event) {
		case EV_SEQ_RESTART:
		case EV_CLOCK_RESET:
			reset();
			break;
		case EV_SEQ_STOP:
			silence();
			break;
		case EV_SEQ_CONTINUE:
			break;
		case EV_LOAD_OK:
			init_state();
			break;
		}
	}

	//
	// CONFIG ACCESSORS
	//

	///////////////////////////////////////////////////////////////////////////////
	void set(PARAM_ID param, int value) {
		switch(param) {
		case P_SQL_SEQ_MODE: set_mode((V_SQL_SEQ_MODE)value); break;
		case P_SQL_QUANTIZE: m_cfg.m_quantize = (V_SQL_QUANTIZE)value; break;
		case P_SQL_STEP_RATE: m_cfg.m_step_rate = (V_SQL_STEP_RATE)value; break;
		case P_SQL_OFF_GRID_MODE: m_cfg.m_off_grid_mode = (V_SQL_OFF_GRID_MODE)value; break;
		case P_SQL_OFF_GRID_AMOUNT: m_cfg.m_off_grid_amount = value; break;
		case P_SQL_TRIG_DUR: m_cfg.m_trig_dur = (V_SQL_TRIG_DUR)value; break;
		case P_SQL_MIDI_OUT: stop_midi_note(); m_cfg.m_midi_out = (V_SQL_MIDI_OUT)value; break;
		case P_SQL_MIDI_OUT_CHAN: stop_midi_note(); m_cfg.m_midi_out_chan = value; break;
		case P_SQL_MIDI_VEL: m_cfg.m_midi_vel = value; break;
		case P_SQL_MIDI_ACC_VEL: m_cfg.m_midi_acc_vel = value; break;
		case P_SQL_MIDI_CC: m_cfg.m_midi_cc = value; break;
		case P_SQL_MIDI_CC_SMOOTH: m_cfg.m_midi_cc_smooth = value; break;
		case P_SQL_CVSCALE: m_cfg.m_cv_scale = (V_SQL_CVSCALE)value; break;
		case P_SQL_CVGLIDE: m_cfg.m_cv_glide = (V_SQL_CVGLIDE)value; break;
		case P_SQL_FILL_MODE: m_cfg.m_fill_mode = (V_SQL_FILL_MODE)value; recalc_data_points_all_pages(); break;
		case P_SQL_LOOP_PER_PAGE: set_loop_per_page(value); break;
		case P_SQL_MIX: m_cfg.m_combine_prev = (V_SQL_COMBINE)value; break;
		case P_SQL_CV_OCTAVE: m_cfg.m_cv_octave = (V_SQL_CVSHIFT)value; break;
		case P_SQL_CV_TRANSPOSE: m_cfg.m_cv_transpose = value; break;
		case P_SQL_SCALED_VIEW: m_cfg.m_scaled_view = !!value; break;
		case P_SQL_CV_ALIAS: m_cfg.m_cv_alias = (V_SQL_CV_ALIAS)value; break;
		case P_SQL_GATE_ALIAS: m_cfg.m_gate_alias = (V_SQL_GATE_ALIAS)value; break;
		case P_SQL_OUT_CAL_SCALE: g_outs.set_cal_scale(m_id, value); fire_event(EV_REAPPLY_CAL_VOLTS,0); break;
		case P_SQL_OUT_CAL_OFFSET:g_outs.set_cal_ofs(m_id, value); fire_event(EV_REAPPLY_CAL_VOLTS,0); break;
		default: break;
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	int get(PARAM_ID param) {
		switch(param) {
		case P_SQL_SEQ_MODE: return m_cfg.m_mode;
		case P_SQL_QUANTIZE: return m_cfg.m_quantize;
		case P_SQL_STEP_RATE: return m_cfg.m_step_rate;
		case P_SQL_OFF_GRID_MODE: return m_cfg.m_off_grid_mode;
		case P_SQL_OFF_GRID_AMOUNT: return m_cfg.m_off_grid_amount;
		case P_SQL_TRIG_DUR: return m_cfg.m_trig_dur;
		case P_SQL_MIDI_OUT_CHAN: return m_cfg.m_midi_out_chan;
		case P_SQL_MIDI_CC: return m_cfg.m_midi_cc;
		case P_SQL_MIDI_CC_SMOOTH: return m_cfg.m_midi_cc_smooth;
		case P_SQL_CVSCALE: return m_cfg.m_cv_scale;
		case P_SQL_CVGLIDE: return m_cfg.m_cv_glide;
		case P_SQL_MIDI_VEL: return m_cfg.m_midi_vel;
		case P_SQL_MIDI_ACC_VEL: return m_cfg.m_midi_acc_vel;
		case P_SQL_FILL_MODE: return m_cfg.m_fill_mode;
		case P_SQL_LOOP_PER_PAGE: return !!m_cfg.m_loop_per_page;
		case P_SQL_MIX: return m_cfg.m_combine_prev;
		case P_SQL_CV_OCTAVE: return m_cfg.m_cv_octave;
		case P_SQL_CV_TRANSPOSE: return m_cfg.m_cv_transpose;
		case P_SQL_MIDI_OUT: return m_cfg.m_midi_out;
		case P_SQL_SCALED_VIEW: return !!m_cfg.m_scaled_view;
		case P_SQL_CV_ALIAS: return m_cfg.m_cv_alias;
		case P_SQL_GATE_ALIAS: return m_cfg.m_gate_alias;
		case P_SQL_OUT_CAL_SCALE: return g_outs.get_cal_scale(m_id);
		case P_SQL_OUT_CAL_OFFSET: return g_outs.get_cal_ofs(m_id);
		default:return 0;
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	int is_valid_param(PARAM_ID param) {
		switch(param) {
		case P_SQL_MIDI_OUT_CHAN:
			return (m_cfg.m_midi_out != V_SQL_MIDI_OUT_NONE);
		case P_SQL_MIDI_VEL:
		case P_SQL_MIDI_ACC_VEL:
			return (m_cfg.m_midi_out == V_SQL_MIDI_OUT_NOTE);
		case P_SQL_MIDI_CC:
		case P_SQL_MIDI_CC_SMOOTH:
			return (m_cfg.m_midi_out == V_SQL_MIDI_OUT_CC);
		case P_SQL_MIX: return (m_id!=0);
		case P_SQL_OUT_CAL_SCALE:
		case P_SQL_OUT_CAL_OFFSET:
			return ::is_cal_mode();
		}
		return 1;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Return a value that can be used to set a default view offset for a layer
	// in the absence of any data points
	byte get_default_value() {
		switch(m_cfg.m_mode) {
			case V_SQL_SEQ_MODE_OFFSET:
				return OFFSET_ZERO;
			case V_SQL_SEQ_MODE_PITCH:
				return CScale::instance().default_note();
			case V_SQL_SEQ_MODE_MOD:
			default:
				return 0;
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// Return a value that all fill points in a layer reverts to in the absence of
	// any data points being defined
	byte get_zero_value() {
		switch(m_cfg.m_mode) {
			case V_SQL_SEQ_MODE_OFFSET:
				return OFFSET_ZERO;
			default:
				return 0;
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	byte any_data_points(byte page_no) {
		return get_page(page_no).any_of(CSequenceStep::DATA_POINT);
	}

	///////////////////////////////////////////////////////////////////////////////
	int get_gate_source_layer() {
		if(V_SQL_GATE_ALIAS_NONE == m_cfg.m_gate_alias) {
			return m_id;
		}
		else {
			return (m_cfg.m_gate_alias - V_SQL_GATE_ALIAS_FROM_L1);
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	int get_cv_source_layer() {
		if(V_SQL_CV_ALIAS_NONE == m_cfg.m_cv_alias) {
			return m_id;
		}
		else {
			return (m_cfg.m_cv_alias - V_SQL_CV_ALIAS_FROM_L1);
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////
	//
	// EDIT FUNCTIONS
	//
	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////
	// change the mode
	void set_mode(V_SQL_SEQ_MODE value) {
		switch (value) {
		case V_SQL_SEQ_MODE_PITCH:
			m_cfg.m_fill_mode = V_SQL_FILL_MODE_PAD;
			m_cfg.m_cv_scale = V_SQL_CVSCALE_1VOCT;
			break;
		case V_SQL_SEQ_MODE_OFFSET:
			m_cfg.m_fill_mode = V_SQL_FILL_MODE_PAD;
			m_cfg.m_cv_scale = V_SQL_CVSCALE_1VOCT;
			break;
		case V_SQL_SEQ_MODE_MOD:
			m_cfg.m_fill_mode = V_SQL_FILL_MODE_INTERPOLATE;
			m_cfg.m_cv_scale = V_SQL_CVSCALE_5V;
			break;
		}
		m_cfg.m_mode = value;
		recalc_data_points_all_pages();
		set_scroll_for_page(0);
	}

	///////////////////////////////////////////////////////////////////////////////
	inline V_SQL_MIDI_OUT get_midi_out_mode() {
		return m_cfg.m_midi_out;
	}

	///////////////////////////////////////////////////////////////////////////////
	CSequenceStep get_step(byte page_no, byte index) {
		CSequencePage& page = get_page(page_no);
		return page.get_step(index);
	}

	///////////////////////////////////////////////////////////////////////////////
	void set_step(byte page_no, byte index, CSequenceStep& step, CSequenceStep::DATA what = CSequenceStep::ALL_DATA, byte auto_data_point = 0) {
		CSequencePage& page = get_page(page_no);
		page.set_step(index, step, m_cfg.m_fill_mode, get_zero_value(), what, auto_data_point);
	}

	///////////////////////////////////////////////////////////////////////////////
	void clear_step(byte page_no, byte index, CSequenceStep::DATA what = CSequenceStep::ALL_DATA) {
		CSequencePage& page = get_page(page_no);
		page.clear_step(index, m_cfg.m_fill_mode, get_zero_value(), what);
	}

	///////////////////////////////////////////////////////////////////////////////
	void clear_page(byte page_no) {
		CSequencePage& page = get_page(page_no);
		page.clear(get_zero_value(), CSequencePage::DEFAULT_LOOP_FROM, CSequencePage::DEFAULT_LOOP_TO);
	}

	///////////////////////////////////////////////////////////////////////////////
	void add_noise_to_page(byte page_no, int seed, int level) {
		CSequencePage& page = get_page(page_no);
		page.add_noise(seed, level, get_default_value(), m_cfg.m_fill_mode, get_zero_value());
	}

	///////////////////////////////////////////////////////////////////////////////
	void randomise_page(byte page_no, int seed) {
		CSequencePage& page = get_page(page_no);
		page.randomise(seed, get_default_value(), m_cfg.m_fill_mode, get_zero_value());
	}

	///////////////////////////////////////////////////////////////////////////////
	void replace_gates(byte page_no, int onsets, int positions, int start_column) {
		CSequencePage& page = get_page(page_no);
		page.replace_gates(onsets, positions, start_column);
	}

	///////////////////////////////////////////////////////////////////////////////
	int count_of(byte page_no, CSequenceStep::POINT_TYPE type, int from=0, int to=CSequencePage::MAX_STEPS-1) {
		CSequencePage& page = get_page(page_no);
		return page.count_of(type, from, to);
	}

	///////////////////////////////////////////////////////////////////////////////
	int any_of(byte page_no, CSequenceStep::POINT_TYPE type, int from=0, int to=CSequencePage::MAX_STEPS-1) {
		CSequencePage& page = get_page(page_no);
		return page.any_of(type, from, to);
	}

	///////////////////////////////////////////////////////////////////////////////
	void get_page_content(byte page_no, CSequencePage& page) {
		ASSERT(page_no >= 0 && page_no < NUM_PAGES);
		page = m_page[page_no];
	}

	///////////////////////////////////////////////////////////////////////////////
	void set_page_content(byte page_no, CSequencePage& page) {
		ASSERT(page_no >= 0 && page_no < NUM_PAGES);
		prepare_page(page_no, INIT_BLANK);
		get_page(page_no) = page;
	}

	///////////////////////////////////////////////////////////////////////////////
	void set_scroll_for(int value, byte centre = 0, int margin = SCROLL_MARGIN) {

		if(m_cfg.m_mode == V_SQL_SEQ_MODE_PITCH && m_cfg.m_scaled_view) {
			value = CScale::instance().note_to_index(value);
		}
		int ofs = m_cfg.m_scroll_ofs;
		if(centre) {
			ofs = value-8;
		}
		else if((value-margin)<ofs) {
			ofs = value-margin;
		}
		else if((value+margin)>ofs+12) {
			ofs = value-12+margin;
		}
		if(ofs < 0) {
			ofs = 0;
		}
		m_cfg.m_scroll_ofs = ofs;
	}

	///////////////////////////////////////////////////////////////////////////////
	void set_scroll_for_page(byte page_no) {
		set_scroll_for(get_page(page_no).get_mean_data_point(get_default_value()),1);
	}


	///////////////////////////////////////////////////////////////////////////////
	void shift_horizontal(byte page_no, int dir) {
		CSequencePage& page = get_page(page_no);
		page.shift_horizontal(dir);
	}

	///////////////////////////////////////////////////////////////////////////////
	byte shift_vertical(byte page_no, int dir) {
		CSequencePage& page = get_page(page_no);
		return page.shift_vertical(
			dir,
			(m_cfg.m_mode == V_SQL_SEQ_MODE_PITCH && m_cfg.m_scaled_view)? &CScale::instance() : NULL,
			m_cfg.m_fill_mode,
			get_zero_value(),
			(m_cfg.m_mode == V_SQL_SEQ_MODE_MOD)
		);
	}

	///////////////////////////////////////////////////////////////////////////////
	inline int get_max_page_no() {
		return m_cfg.m_max_page_no;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Make sure that a page is available before trying to access it. When new
	// pages are addeded, they are initialised from the last existing page
	void prepare_page(int page, byte init_mode) {
		while(m_cfg.m_max_page_no < page) {
			CSequencePage& page = get_page(m_cfg.m_max_page_no+1);
			switch(init_mode) {
			case INIT_FIRST:
				page = get_page(0);
				break;
			case INIT_LAST:
				page = get_page(m_cfg.m_max_page_no);
				break;
			case INIT_BLANK:
			default:
				page.clear(get_zero_value(), get_page(0).loop_from(), get_page(0).loop_to());
				break;
			}
			++m_cfg.m_max_page_no;
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// Change the number of existing pages
	void set_max_page_no(int page) {
		prepare_page(page, INIT_LAST);	// in case the number of pages has increased
		m_cfg.m_max_page_no = page; // in case it has got less
	}

	///////////////////////////////////////////////////////////////////////////////
	inline V_SQL_SEQ_MODE get_mode() {
		return m_cfg.m_mode;
	}

	///////////////////////////////////////////////////////////////////////////////
	inline byte is_scaled_view() {
		return m_cfg.m_scaled_view;
	}

	///////////////////////////////////////////////////////////////////////////////
	int get_scroll_ofs() {
		return m_cfg.m_scroll_ofs;
	}

	///////////////////////////////////////////////////////////////////////////////
	void set_scroll_ofs(int scroll_ofs) {
		m_cfg.m_scroll_ofs = scroll_ofs;
	}

	///////////////////////////////////////////////////////////////////////////////
	void set_loop_per_page(int value) {
		if(value) {
			// when going to loop per page mode we copy the value from page
			// 0 into each page, so that there is no immediate change to the
			// loop points on B/C/D
			for(int i=1; i<NUM_PAGES; ++i) {
				get_page(i).loop_from() = get_page(0).loop_from();
				get_page(i).loop_to() = get_page(0).loop_to();
			}
		}
		m_cfg.m_loop_per_page = value;
	}

	///////////////////////////////////////////////////////////////////////////////
	void set_loop_from(byte page_no, int from) {
		get_page(m_cfg.m_loop_per_page? page_no:0).loop_from() = from;
	}

	///////////////////////////////////////////////////////////////////////////////
	void set_loop_to(byte page_no, int to) {
		get_page(m_cfg.m_loop_per_page? page_no:0).loop_to() = to;
	}

	///////////////////////////////////////////////////////////////////////////////
	int get_loop_from(byte page_no) {
		return get_page(m_cfg.m_loop_per_page? page_no:0).loop_from();
	}

	///////////////////////////////////////////////////////////////////////////////
	int get_loop_to(byte page_no) {
		return get_page(m_cfg.m_loop_per_page? page_no:0).loop_to();
	}

	///////////////////////////////////////////////////////////////////////////////
	int get_loop_span(byte page_no, int*min = nullptr, int*max = nullptr) {
		int from = get_loop_from(page_no);
		int to = get_loop_to(page_no);
		if(from > to) {
			int t=to;
			to = from;
			from = t;
		}
		if(min) *min = from;
		if(max) *max = to;
		return 1 + to - from;
	}

	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////
	//
	// PAGE ARRANGER
	//
	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////
	void cue_all() {
		m_cfg.m_cue_mode = CUE_AUTO;
		m_cfg.m_cue_list[0] = 0;
		m_cfg.m_cue_list_count = 1;
		m_state.m_cue_list_next = 0;
	}

	///////////////////////////////////////////////////////////////////////////////
	void cue_random() {
		m_cfg.m_cue_mode = CUE_RANDOM;
		m_cfg.m_cue_list_count = 1;
		cue_update();
		m_state.m_cue_list_next = 0;
	}

	///////////////////////////////////////////////////////////////////////////////
	byte cue_first(byte page_no) {
		m_cfg.m_cue_mode = CUE_MANUAL;
		m_cfg.m_cue_list_count = 0;
		m_state.m_cue_list_next = 0;
		return cue_next(page_no);
	}

	///////////////////////////////////////////////////////////////////////////////
	byte cue_next(byte page_no) {
		ASSERT(page_no >= 0 && page_no < NUM_PAGES);
		if(page_no <= m_cfg.m_max_page_no) {
			if(m_cfg.m_cue_list_count<MAX_CUE_LIST) {
				m_cfg.m_cue_list[m_cfg.m_cue_list_count++] = page_no;
				return 1;
			}
		}
		return 0;
	}

	///////////////////////////////////////////////////////////////////////////////
	void cue_cancel() {
		m_cfg.m_cue_mode = CUE_NONE;
		m_cfg.m_cue_list[0] = 0;
		m_cfg.m_cue_list_count = 0;
		m_state.m_cue_list_next = 0;
	}

	///////////////////////////////////////////////////////////////////////////////
	// update the cue point when there is a sequencer reset
	void cue_reset() {
		if(CUE_NONE != m_cfg.m_cue_mode) {
			if(CUE_AUTO == m_cfg.m_cue_mode) {
				m_cfg.m_cue_list[0] = 0; // back to page A
			}
			m_state.m_play_page_no = m_cfg.m_cue_list[0];
			m_state.m_cue_list_next = 0;
			m_state.m_page_advanced = 1;
			cue_update();
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	int get_cue_list_count() {
		return m_cfg.m_cue_list_count;
	}

	///////////////////////////////////////////////////////////////////////////////
	byte is_cue_mode() {
		return (m_cfg.m_cue_mode != CUE_NONE);
	}

	///////////////////////////////////////////////////////////////////////////////
	byte is_page_advanced() {
		return m_state.m_page_advanced;
	}

	///////////////////////////////////////////////////////////////////////////////
	void set_play_page(int page_no) {
		m_state.m_play_page_no = page_no;
	}
	///////////////////////////////////////////////////////////////////////////////
	int get_play_page() {
		return m_state.m_play_page_no;
	}
	///////////////////////////////////////////////////////////////////////////////
	void set_pos(int pos) {
		m_state.m_play_pos = pos;
	}
	///////////////////////////////////////////////////////////////////////////////
	int get_pos() {
		return m_state.m_play_pos;
	}

	///////////////////////////////////////////////////////////////////////////////
	void set_content(CSequenceLayer& other) {
		m_cfg = other.m_cfg;
		m_state = other.m_state;
	}

	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////
	//
	// SEQUENCER FUNCTIONS
	//
	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////
	void stop_midi_note() {
		if(m_state.m_playing_midi_note != NO_MIDI_NOTE) {
			g_midi.stop_note(m_cfg.m_midi_out_chan, m_state.m_playing_midi_note);
			m_state.m_playing_midi_note = NO_MIDI_NOTE;
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	void start_midi_note(byte tie) {
		if(tie) {
			if(m_state.m_step_midi_note != m_state.m_playing_midi_note) {
				g_midi.start_note(m_cfg.m_midi_out_chan, m_state.m_step_midi_note, m_state.m_step_midi_vel);
				stop_midi_note();
			}
		}
		else {
			stop_midi_note();
			g_midi.start_note(m_cfg.m_midi_out_chan, m_state.m_step_midi_note, m_state.m_step_midi_vel);
		}
		m_state.m_playing_midi_note = m_state.m_step_midi_note;
	}

	///////////////////////////////////////////////////////////////////////////////
	void silence() {
		g_outs.gate(m_id, COuts::GATE_CLOSED);
		m_state.m_retrig_ms = 0;
		m_state.m_retrig_timeout = 0;
		stop_midi_note();
	}

	///////////////////////////////////////////////////////////////////////////////
	void mute() {
		m_cfg.m_muted = 1;
		silence();
	}

	///////////////////////////////////////////////////////////////////////////////
	void unmute() {
		m_cfg.m_muted = 0;
	}

	///////////////////////////////////////////////////////////////////////////////
	void toggle_mute() {
		if(m_cfg.m_muted) {
			unmute();
		}
		else {
			mute();
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	int is_muted() {
		return m_cfg.m_muted;
	}

	///////////////////////////////////////////////////////////////////////////////
	void set_cal_volts(int volts) {
		g_outs.test_dac(m_id, volts);
	}

	///////////////////////////////////////////////////////////////////////////////
	// Get a number of ticks, x, where
	// -ticks_per_step < x < ticks_per_step
	// Which is a time offset for swing, slide etc at position step_no steps
	// from the start of the loop
	int get_ticks_offset(int step_no, int max_offset) {

		// Calculate any required offset from "grid" time. The units of scheduling
		// slide is the 'off gridness' measured in 1/256s of a 24PPQN clock so the
		// offset can have a range +/- (rate_pp24 * 256)
		int offset = 0;

		// mod amount has  a range 25 thru 75.. map this to between -1 and +1 with 50=0
		float amp = (m_cfg.m_off_grid_amount-50.0)/26.0; // -1.0 >> 1.0
		switch(m_cfg.m_off_grid_mode) {
			case V_SQL_OFF_GRID_MODE_SWING: {
				// work out the 'equivalent step' (i.e. step number withing
				// the selected loop points)
				int equiv_step = (int)step_no - get_loop_from(m_state.m_play_page_no);
				while(equiv_step<0) {
					equiv_step += 4;
				}
				if(equiv_step&1) {
					offset = max_offset*amp;
				}
				return offset;
			}
			case V_SQL_OFF_GRID_MODE_SLIDE:
				return max_offset*amp;
			case V_SQL_OFF_GRID_MODE_RANDOM:
				offset = -amp*(random()%max_offset);
				if(random()&1) {
					return -offset;
				}
				else {
					return offset;
				}
			default:
				return 0;
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// This method handles the scheduling of grid steps on the current layer
	//
	// All layer step rates are multiples of 24PP and when there is a new
	// 24PP tick we use the master pp24 counter to decide whether it is time
	// for this layer to "step".
	//
	// On all steps except for the first one following a reset, we are scheduling
	// when to move to and play the NEXT step
	//
	// The actual stepping is scheduled in TICKS_TYPE (256 ticks per 24PP) so that
	// micro-offsetting from true grid time is possible to support swing etc
	// The maximum offset from grid is +/- half of a grid step, so it is never
	// possible for steps to be scheduled out of order
	//
	byte play(clock::TICKS_TYPE ticks, int dice_roll, REC_SESSION *rec, CSequenceStep& step_value) {

		auto do_advance = 0; 	// flag says if the play position moved at this call
		auto do_play = 0; 		// flag says if we started playing a step at this call
		if(m_state.m_first_step) {
			// the very first step.. we'll play it now and schedule the next
			do_play = 1;
			m_state.m_first_step = 0;
		}
		else if(m_state.m_next_step_time <= ticks) {
			do_advance = 1;
			do_play = 1;
		}

		// move to the next step, unless this is the very first step following
		// a restart, in which case we are already pointing at step zero
		if(do_advance) {
			m_state.m_page_advanced = 0;
			if(calc_next_step(m_state.m_play_page_no, m_state.m_play_pos)) {
				if(m_cfg.m_cue_mode != CUE_NONE) {
					m_state.m_play_page_no = m_cfg.m_cue_list[m_state.m_cue_list_next];
				}
				cue_update();
				m_state.m_page_advanced = 1;
			}
		}

		if(do_play) {
			step_value = get_step(m_state.m_play_page_no, m_state.m_play_pos);
			if(rec && rec->mode == V_SEQ_REC_MODE_CV) {
				step_value.set_value(rec->note);
				if(rec->arm == V_SEQ_REC_ARM_ON) {
					set_step(m_state.m_play_page_no, m_state.m_play_pos, step_value, CSequenceStep::ALL_DATA, 1);
				}
			}

			m_state.m_step_timeout = g_clock.get_ms_per_measure(m_cfg.m_step_rate);

			//m_state.m_suppress_step = 0;
			if(step_value.get_prob()) { // nonzero probability?
				if(dice_roll>step_value.get_prob()) {
					// dice roll is between 1 and 16, if this number is greater
					// than the step probability (1-15) then the step will
					// be suppressed
					step_value.clear(CSequenceStep::ALL_DATA);
					step_value.set(CSequenceStep::IGNORE_POINT,1);
				}
			}

			// after we play a step, we need to schedule the next one...

			// get the step rate for the layer in PP24 units and convert to ticks
			int rate_pp24 = clock::pp24_per_measure(m_cfg.m_step_rate);
			ASSERT(rate_pp24);
			clock::TICKS_TYPE ticks_per_step = clock::pp24_to_ticks(rate_pp24);

			// work out the next "grid" step position
			clock::TICKS_TYPE next_step_grid_time = ticks_per_step * (int)(1.5+(double)ticks/ticks_per_step);

			// add in the hold duration
			next_step_grid_time += ticks_per_step * (clock::TICKS_TYPE)(step_value.get_hold());
			m_state.m_step_timeout = m_state.m_step_timeout * (step_value.get_step_count());

			// apply timing adjustments for swing etc
			clock::TICKS_TYPE next_step_time = next_step_grid_time + get_ticks_offset(1+m_state.m_play_pos, ticks_per_step/2);
			if(next_step_time < 0) {
				m_state.m_next_step_time = 0;
			}
			else {
				m_state.m_next_step_time = next_step_time;
			}
		}
		return do_play;
	}


	///////////////////////////////////////////////////////////////////////////////
	// called once per ms
	void run() {

		if(!m_cfg.m_muted) {
			if(m_state.m_gate_timeout) {
				if(!--m_state.m_gate_timeout) {
					g_outs.gate(m_id, COuts::GATE_CLOSED);
					stop_midi_note();
				}
			}

			if(m_state.m_midi_cc_inc) {
				int prev_value = m_state.m_midi_cc_value>>16;
				m_state.m_midi_cc_value += m_state.m_midi_cc_inc;
				if(m_state.m_midi_cc_inc>0) {
					if(m_state.m_midi_cc_value > m_state.m_midi_cc_target) {
						m_state.m_midi_cc_value = m_state.m_midi_cc_target;
						m_state.m_midi_cc_inc = 0;
					}
				}
				else {
					if(m_state.m_midi_cc_value < m_state.m_midi_cc_target) {
						m_state.m_midi_cc_value = m_state.m_midi_cc_target;
						m_state.m_midi_cc_inc = 0;
					}
				}
				int next_value = (m_state.m_midi_cc_value>>16);
				if(next_value != prev_value) {
					g_midi.send_cc(m_cfg.m_midi_out_chan, m_cfg.m_midi_cc, next_value);
				}
			}


			// is retrigger active on this step?
			if(m_state.m_retrig_ms) {
				if(m_state.m_retrig_timeout) {
					// sill waiting for the next retrig to be due
					--m_state.m_retrig_timeout;
				}
				else {
					// retrigger the gate
					g_outs.gate(m_id, COuts::GATE_TRIG);
					m_state.m_gate_timeout = m_state.m_trig_dur;

					// retrigger the MIDI note
					if( V_SQL_MIDI_OUT_NOTE == m_cfg.m_midi_out &&
						m_state.m_step_midi_note != NO_MIDI_NOTE) {
						start_midi_note(0);
					}

					// schedule the next retrigger
					m_state.m_retrig_timeout = m_state.m_retrig_ms;
				}
			}

			// one less ms of this step
			if(m_state.m_step_timeout) {
				--m_state.m_step_timeout;
			}
			else {
				// ensure retrig stops at end of stpe
				m_state.m_retrig_ms = 0;
			}
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// the long value is MIDI notes * 65536
	CV_TYPE get_step_output(CV_TYPE this_input, CSequenceStep& step_value) {

		CV_TYPE this_output;
		if((m_cfg.m_combine_prev == V_SQL_COMBINE_MASK ||
			m_cfg.m_combine_prev == V_SQL_COMBINE_ADD_MASK) &&
			!step_value.is(CSequenceStep::DATA_POINT)) {
			// in mask/add and mask modes we simply duplicate the previous layer output
			this_output = this_input;
		}
		else
		{

			// get the scaled data point
			int value = step_value.get_value();
			if(m_cfg.m_mode == V_SQL_SEQ_MODE_OFFSET) {
				this_output = COuts::SCALING*(value - OFFSET_ZERO);
			}
			else {
				this_output = COuts::SCALING*value;
			}

			// check if we have an absolute volts range (1V - 8V). If so scale the output
			// accordingly (each volt will be 12 scale points)
			if(m_cfg.m_cv_scale < V_SQL_CVSCALE_1VOCT) {
				this_output = (this_output * (1 + m_cfg.m_cv_scale - V_SQL_CVSCALE_1V) * 12)/127;
			}

			// perform any addition of previous layer output
			if(m_cfg.m_combine_prev == V_SQL_COMBINE_ADD ||
				m_cfg.m_combine_prev == V_SQL_COMBINE_ADD_MASK) {
				this_output += this_input;
			}
		}

		return this_output;
	}

	///////////////////////////////////////////////////////////////////////////////
	CV_TYPE get_layer_output(CV_TYPE step_output, CSequenceStep& step_value) {

		// apply transposition
		CV_TYPE output = step_output + COuts::SCALING * (int)m_cfg.m_cv_transpose;

		// quantize the output to scale if needed
		switch(m_cfg.m_quantize) {
		case V_SQL_SEQ_QUANTIZE_CHROMATIC:
			if(output < 0) {
				output = 0;
			}
			output = COuts::SCALING * (output/COuts::SCALING);
			break;
		case V_SQL_SEQ_QUANTIZE_SCALE:
			if(output < 0) {
				output = 0;
			}
			output = COuts::SCALING * CScale::instance().force_to_scale(output/COuts::SCALING);
			break;
		}

		// apply octave shift
		if(m_cfg.m_cv_octave != V_SQL_CVSHIFT_NONE) {
			output += 12 * COuts::SCALING * (m_cfg.m_cv_octave - V_SQL_CVSHIFT_NONE);
		}

		return output;
	}

	///////////////////////////////////////////////////////////////////////////////
	void process_cv(CV_TYPE output, CSequenceStep& step_value) {

		// calculate glide time
		int glide_time;
		switch(m_cfg.m_cv_glide) {
		case V_SQL_CVGLIDE_ON:
			glide_time = m_state.m_step_timeout;
			break;
		case V_SQL_CVGLIDE_TIE:
			glide_time = (step_value.is(CSequenceStep::TIE_POINT))? m_state.m_step_timeout : 0;
			break;
		case V_SQL_CVGLIDE_OFF:
		default:
			glide_time = 0;
		}
		g_outs.cv(m_id, output, m_cfg.m_cv_scale, glide_time);
	}

	///////////////////////////////////////////////////////////////////////////////
	// Play the gate for a step. This is usually done after CV so that we have
	// already set the appropriate pitch before trigging a VCA etc
	void process_gate(CSequenceStep& step_value) {
		m_state.m_retrig_ms = 0;
		if(step_value.is(CSequenceStep::IGNORE_POINT)) {
			if(!m_state.m_gate_timeout) {
				g_outs.gate(m_id, COuts::GATE_CLOSED);
			}
		}
		else if(step_value.is(CSequenceStep::TRIG_POINT) || step_value.get_retrig()>0) {
			if(step_value.is(CSequenceStep::TIE_POINT)) {
				m_state.m_trig_dur = 0; // until next step
			}
			else {
				// set the appropriate note duration
				switch(m_cfg.m_trig_dur) {
				case V_SQL_NOTE_DUR_16:
					m_state.m_trig_dur = 0; // until next step
					break;
				case V_SQL_NOTE_DUR_TRIG:
					m_state.m_trig_dur = COuts::TRIG_DURATION; // just a trigger
					break;
				default: // other enumerations have integer values 0-15
					m_state.m_trig_dur = (g_clock.get_ms_per_measure(m_cfg.m_step_rate) * (1 + m_cfg.m_trig_dur - V_SQL_NOTE_DUR_1)) / 16;
					break;
				}
			}
			m_state.m_gate_timeout = m_state.m_trig_dur;

			if(step_value.get_retrig()) {
				int step_ms = g_clock.get_ms_per_measure(m_cfg.m_step_rate);
				int n_steps = step_value.get_step_count();
				m_state.m_retrig_ms = ((16-(int)step_value.get_retrig()) * step_ms * n_steps) / 16;
			}
			else {
				m_state.m_retrig_ms = 0;
			}
			m_state.m_retrig_timeout = m_state.m_retrig_ms;
			g_outs.gate(m_id, COuts::GATE_TRIG);
		}
		else if(step_value.is(CSequenceStep::TIE_POINT)) {
			m_state.m_gate_timeout = 0;
			g_outs.gate(m_id, COuts::GATE_OPEN);
		}
		else {
			if(!m_state.m_gate_timeout) {
				g_outs.gate(m_id, COuts::GATE_CLOSED);
			}
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// This function does the extra work following process_gate() call in order
	// to send out any required MIDI notes
	void process_midi_note(CV_TYPE output, CSequenceStep& step_value) {

		// is there a trig or tie to action at at this step?
		if(step_value.is(CSequenceStep::TRIG_POINT) || step_value.is(CSequenceStep::TIE_POINT)) {
			// round the output pitch to the closest MIDI note
			int note = (output<0)?
					((output-COuts::SCALING/2)/COuts::SCALING) :
					((output+COuts::SCALING/2)/COuts::SCALING);
			while(note <= 0) 	{
				note += 12;
			}
			while(note > 127) {
				note -= 12;
			}
			m_state.m_step_midi_note = note;
			m_state.m_step_midi_vel = step_value.is(CSequenceStep::ACCENT_POINT) ? m_cfg.m_midi_acc_vel : m_cfg.m_midi_vel;
			start_midi_note(step_value.is(CSequenceStep::TIE_POINT) && !step_value.is(CSequenceStep::TRIG_POINT));
		}
		else if(!m_state.m_gate_timeout) {
			// stop a note that was left playing until the next step
			stop_midi_note();
		}

	}

	///////////////////////////////////////////////////////////////////////////////
	void process_midi_cc(CV_TYPE output) {
		byte value = clamp7bit(((output+COuts::SCALING/2)/COuts::SCALING));

		m_state.m_midi_cc_target = value * COuts::SCALING;
		if(m_cfg.m_midi_cc_smooth) {
			m_state.m_midi_cc_target = value * COuts::SCALING;
			m_state.m_midi_cc_inc = (m_state.m_midi_cc_target - m_state.m_midi_cc_value) / (int)m_state.m_step_timeout;
		}
		else {
			if(m_state.m_midi_cc_value != m_state.m_midi_cc_target) {
				m_state.m_midi_cc_value = m_state.m_midi_cc_target;
				g_midi.send_cc(m_cfg.m_midi_out_chan, m_cfg.m_midi_cc, value);
			}
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////////
	static int get_cfg_size() {
		return sizeof(CONFIG) + NUM_PAGES * CSequencePage::get_cfg_size();
	}

	/////////////////////////////////////////////////////////////////////////////////////////////
	void get_cfg(byte **dest) {
		memcpy(*dest, &m_cfg, sizeof m_cfg);
		(*dest) += sizeof m_cfg;
		for(int i=0; i<NUM_PAGES; ++i) {
			m_page[i].get_cfg(dest);
		}
	}
	/////////////////////////////////////////////////////////////////////////////////////////////
	void set_cfg(byte **src) {
		memcpy(&m_cfg, *src, sizeof m_cfg);
		(*src) += sizeof m_cfg;
		for(int i=0; i<NUM_PAGES; ++i) {
			m_page[i].set_cfg(src);
		}
	}
};

#endif /* SEQUENCE_LAYER_H_ */

