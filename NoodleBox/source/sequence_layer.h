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
//  SEQUENCER LAYER
//
///////////////////////////////////////////////////////////////////////////////////
#ifndef SEQUENCE_LAYER_H_
#define SEQUENCE_LAYER_H_

///////////////////////////////////////////////////////////////////////////////
// This class holds all the info for a single layer/part
class CSequenceLayer {

public:
	typedef enum {
		VIEW_PITCH,
		VIEW_PITCH_OFFSET,
		VIEW_MODULATION
	} VIEW_TYPE;

	enum {
		OFFSET_ZERO = 64,		// step value for zero transpose offset
		MAX_PAGES = 4					// number of pages
	};

private:

	enum {
		MAX_PLAYING_NOTES = 8,
		DEFAULT_SCROLL_OFS = 24,
		MAX_PAGE_LIST = 16
	};

	// This structure holds the layer information that gets saved with the patch
	typedef struct {
		CSequencePage 		m_page[MAX_PAGES];	// sequencer page
		byte 			m_page_list[MAX_PAGE_LIST];
		byte			m_page_list_count;
		V_SQL_SEQ_MODE 	m_mode;				// the mode for this layer (note, mod etc)
		V_SQL_QUANTIZE 	m_quantize;	// force to scale
		V_SQL_STEP_RATE m_step_rate;		// step rate setting
		char			m_transpose;		// manual transpose amount for the layer
		V_SQL_NOTE_DUR	m_note_dur;
		V_SQL_MIDI_CHAN m_midi_channel;		// MIDI channel
		byte 			m_midi_cc;			// MIDI CC
		V_SQL_CVSCALE	m_cv_scale;
		V_SQL_CVSHIFT	m_cv_shift;
		V_SQL_CVGLIDE	m_cv_glide;
		V_SQL_COMBINE	m_combine_prev;
		byte 			m_midi_vel;
		byte 			m_max_page_no;		// the highest numbered active page (0-3)
		byte 			m_loop_per_page :1;
		int				m_interpolate:1;
		int 			m_enabled:1;
		int 			m_page_advance:1;
	} CONFIG;



	typedef struct {
		VIEW_TYPE m_view;
		byte m_scroll_ofs;					// lowest step value shown on grid
		int m_play_page_no;				// the page number being played
		int m_play_pos;
		int m_page_list_pos;


		CSequenceStep m_step_value;			// the last value output by sequencer
		byte m_stepped;						// stepped flag
		byte m_page_advanced;
		byte m_midi_note; 					// last midi note played on channel
		uint32_t m_next_tick;
		byte m_last_tick_lsb;
		uint32_t m_gate_timeout;
		uint32_t m_step_timeout;
	} STATE;

	const uint32_t INFINITE_GATE = (uint32_t)(-1);

	CONFIG m_cfg;				// instance of config
	STATE m_state;
	byte m_id;

	//
	// PRIVATE METHODS
	//


	///////////////////////////////////////////////////////////////////////////////
	// accessor for a page
	inline CSequencePage& get_page(byte page_no) {
		ASSERT(page_no >= 0 && page_no < MAX_PAGES);
		return m_cfg.m_page[page_no];
	}

	///////////////////////////////////////////////////////////////////////////////
	byte get_default_value() {
		switch(m_cfg.m_mode) {
			case V_SQL_SEQ_MODE_OFFSET:
				return OFFSET_ZERO;
			case V_SQL_SEQ_MODE_PITCH:
				return g_scale.default_note();
			case V_SQL_SEQ_MODE_MOD:
			default:
				return 0;
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// Calculate the page and step
	void calc_next_step(int &page_no, int &step_no, byte& page_switch, int &page_list_pos) {

		CSequencePage& page = get_page(page_no);
		page_switch=0;
		if(step_no == page.get_loop_to()) {

			if(m_cfg.m_page_advance) { // automatic advance at end of page?
				if(m_cfg.m_page_list_count) { // has user got a page list?
					if(page_list_pos < m_cfg.m_page_list_count - 1) {
						++page_list_pos;
					}
					else {
						page_list_pos = 0;
					}
					byte next_page_no = m_cfg.m_page_list[page_list_pos];
					if(next_page_no <= m_cfg.m_max_page_no) {
						// still a valid page
						page_no = next_page_no;
						page_switch = 1;
					}
				}
				else {
					if(page_no < m_cfg.m_max_page_no) {
						// automatic page advance to the next page
						++page_no;
						page_switch = 1;
					}
					else {
						// reached end of last page, going back to first
						page_no = 0;
						page_switch = 1;
					}
				}
			}
			// back to first step
			CSequencePage& new_page = get_page(page_no); // could be same page
			step_no = new_page.get_loop_from();
		}
		else {
			if(page.get_loop_to() < page.get_loop_from()) { // run backwards
				if(--step_no < 0) {
					step_no = CSequencePage::MAX_STEPS-1;
				}
				else if(step_no > page.get_loop_from()) {
					step_no = page.get_loop_from();
				}
				else if(step_no < page.get_loop_to()) {
					step_no = page.get_loop_to();
				}
			}
			else {
				if(++step_no > CSequencePage::MAX_STEPS-1) {
					step_no = 0;
				}
				else if(step_no > page.get_loop_to()) {
					step_no = page.get_loop_to();
				}
				else if(step_no < page.get_loop_from()) {
					step_no = page.get_loop_from();
				}
			}
		}
	}


	void recalc_data_points_all_pages() {
		for(int i=0; i<MAX_PAGES; ++i) {
			CSequencePage& page = get_page(i);
			page.recalc(m_cfg.m_interpolate, get_default_value());
		}
	}


public:
	///////////////////////////////////////////////////////////////////////////////
	void clear_data_point(byte page_no, byte index) {
		CSequencePage& page = get_page(page_no);
		CSequenceStep step = page.get_step(index);
		step.clear();
		page.set_step(index, step, m_cfg.m_interpolate, get_default_value());
	}

	///////////////////////////////////////////////////////////////////////////////
	void init(byte id) {
		m_id = id;
		init_config();
		init_state();
	}
	///////////////////////////////////////////////////////////////////////////////
	inline CScale& get_scale() {
		return g_scale;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Assign valid default values to the sequence layer configuration (i.e.
	// the data that forms part of a saved sequence)
	void init_config() {
		m_cfg.m_mode 		= V_SQL_SEQ_MODE_PITCH;
		m_cfg.m_quantize 	= V_SQL_SEQ_QUANTIZE_CHROMATIC;
		m_cfg.m_step_rate	= V_SQL_STEP_RATE_16;
		m_cfg.m_note_dur	= V_SQL_NOTE_DUR_100;
		m_cfg.m_combine_prev= V_SQL_COMBINE_OFF;
		m_cfg.m_transpose	= 0;
		m_cfg.m_midi_channel 	= V_SQL_MIDI_CHAN_NONE;
		m_cfg.m_midi_cc = 1;
		m_cfg.m_enabled = 1;
		m_cfg.m_cv_scale = V_SQL_CVSCALE_1VOCT;
		m_cfg.m_cv_shift = V_SQL_CVSHIFT_NONE;
		m_cfg.m_cv_glide = V_SQL_CVGLIDE_OFF;
		m_cfg.m_midi_vel = 100;
		m_cfg.m_interpolate = 0;
		m_cfg.m_max_page_no = 0;
		m_cfg.m_page_advance = 0;
		m_cfg.m_loop_per_page = 0;
		m_cfg.m_page_list_count = 0;
		set_mode(m_cfg.m_mode);
		clear();
	}

	///////////////////////////////////////////////////////////////////////////////
	// Initialise the state of a configured sequence layer (e.g. when the layer
	// has been loaded from EEPROM)
	void init_state() {
		m_state.m_scroll_ofs = DEFAULT_SCROLL_OFS;
		m_state.m_last_tick_lsb = 0;
		m_state.m_midi_note = 0;
		m_state.m_view = VIEW_PITCH;
		m_state.m_page_list_pos = 0;
		reset();
	}


	///////////////////////////////////////////////////////////////////////////////
	// Reset the playback state of the layer
	void reset() {
		m_state.m_step_value.clear();
		m_state.m_stepped = 0;
		m_state.m_play_pos = 0;
		m_state.m_next_tick = 0;
		m_state.m_gate_timeout = 0;
		m_state.m_step_timeout = 0;
		m_state.m_play_page_no = 0;
		m_state.m_page_advanced = 0;
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
		case P_SQL_NOTE_DUR: m_cfg.m_note_dur = (V_SQL_NOTE_DUR)value; break;
		case P_SQL_MIDI_CHAN: m_cfg.m_midi_channel = (V_SQL_MIDI_CHAN)value; break;
		case P_SQL_MIDI_CC: m_cfg.m_midi_cc = value; break;
		case P_SQL_CVSCALE: m_cfg.m_cv_scale = (V_SQL_CVSCALE)value; break;
		case P_SQL_CVGLIDE: m_cfg.m_cv_glide = (V_SQL_CVGLIDE)value; break;
		case P_SQL_MIDI_VEL: m_cfg.m_midi_vel = value; break;
		case P_SQL_INTERPOLATE: m_cfg.m_interpolate = value; recalc_data_points_all_pages(); break;
		case P_SQL_SCALE_TYPE: g_scale.build((V_SQL_SCALE_TYPE)value, g_scale.get_root()); break;
		case P_SQL_SCALE_ROOT: g_scale.build(g_scale.get_type(), (V_SQL_SCALE_ROOT)value); break;
		case P_SQL_LOOP_PER_PAGE: m_cfg.m_loop_per_page = value; break;
		case P_SQL_AUTO_PAGE_ADVANCE: m_cfg.m_page_advance = value; break;
		case P_SQL_COMBINE: m_cfg.m_combine_prev = (V_SQL_COMBINE)value; break;
		case P_SQL_CVSHIFT: m_cfg.m_cv_shift = (V_SQL_CVSHIFT)value; break;
		default: break;
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	int get(PARAM_ID param) {
		switch(param) {
		case P_SQL_SEQ_MODE: return m_cfg.m_mode;
		case P_SQL_QUANTIZE: return m_cfg.m_quantize;
		case P_SQL_STEP_RATE: return m_cfg.m_step_rate;
		case P_SQL_NOTE_DUR: return m_cfg.m_note_dur;
		case P_SQL_MIDI_CHAN: return m_cfg.m_midi_channel;
		case P_SQL_MIDI_CC: return m_cfg.m_midi_cc;
		case P_SQL_CVSCALE: return m_cfg.m_cv_scale;
		case P_SQL_CVGLIDE: return m_cfg.m_cv_glide;
		case P_SQL_MIDI_VEL: return m_cfg.m_midi_vel;
		case P_SQL_INTERPOLATE: return m_cfg.m_interpolate;
		case P_SQL_SCALE_TYPE: return g_scale.get_type();
		case P_SQL_SCALE_ROOT: return g_scale.get_root();
		case P_SQL_LOOP_PER_PAGE: return m_cfg.m_loop_per_page;
		case P_SQL_AUTO_PAGE_ADVANCE: return m_cfg.m_page_advance;
		case P_SQL_COMBINE: return m_cfg.m_combine_prev;
		case P_SQL_CVSHIFT: return m_cfg.m_cv_shift;
		default:return 0;
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	int is_valid_param(PARAM_ID param) {
		switch(param) {
		//case P_SQL_FORCE_SCALE:
		//case P_SQL_SCALE_TYPE:
		//case P_SQL_SCALE_ROOT:
		case P_SQL_MIDI_VEL:
			return !!(m_cfg.m_mode == V_SQL_SEQ_MODE_PITCH||m_cfg.m_mode == V_SQL_SEQ_MODE_OFFSET);
		case P_SQL_MIDI_CC:	return !!(m_cfg.m_mode == V_SQL_SEQ_MODE_MOD);
		case P_SQL_COMBINE: return (m_id!=0);
		}
		return 1;
	}

	//
	// EDIT FUNCTIONS
	//



	///////////////////////////////////////////////////////////////////////////////
	// change the mode
	void set_mode(V_SQL_SEQ_MODE value) {
		switch (value) {
		case V_SQL_SEQ_MODE_PITCH:
			m_state.m_view = VIEW_TYPE::VIEW_PITCH;
			m_cfg.m_interpolate = 0;
			m_cfg.m_cv_scale = V_SQL_CVSCALE_1VOCT;
			break;
		case V_SQL_SEQ_MODE_OFFSET:
			m_state.m_view = VIEW_TYPE::VIEW_PITCH_OFFSET;
			m_cfg.m_cv_scale = V_SQL_CVSCALE_1VOCT;
			m_cfg.m_interpolate = 0;
			break;
		case V_SQL_SEQ_MODE_MOD:
			m_state.m_view = VIEW_TYPE::VIEW_MODULATION;
			m_cfg.m_cv_scale = V_SQL_CVSCALE_5V;
			m_cfg.m_interpolate = 1;
			break;
		}
		m_cfg.m_mode = value;
	}

	///////////////////////////////////////////////////////////////////////////////
	CSequenceStep get_step(byte page_no, byte index) {
		CSequencePage& page = get_page(page_no);
		return page.get_step(index);
	}

	///////////////////////////////////////////////////////////////////////////////
	void set_step(byte page_no, byte index, CSequenceStep& step) {
		CSequencePage& page = get_page(page_no);
		page.set_step(index, step, m_cfg.m_interpolate, get_default_value());
	}

	///////////////////////////////////////////////////////////////////////////////
	void clear_step(byte page_no, byte index) {
		CSequencePage& page = get_page(page_no);
		page.clear_step(index, m_cfg.m_interpolate, get_default_value());
	}

	///////////////////////////////////////////////////////////////////////////////
	void clear_page(byte page_no) {
		CSequencePage& page = get_page(page_no);
		page.clear(get_default_value());
	}

	///////////////////////////////////////////////////////////////////////////////
	void clear() {
		for(int i=0; i<MAX_PAGES; ++i) {
			CSequencePage& page = get_page(i);
			page.clear(get_default_value());
		}
		m_cfg.m_max_page_no = 0;
		m_cfg.m_page_list_count = 0;
	}

	///////////////////////////////////////////////////////////////////////////////
	void shift_horizontal(byte page_no, int dir) {
		CSequencePage& page = get_page(page_no);
		page.shift_horizontal(dir);
	}

	///////////////////////////////////////////////////////////////////////////////
	byte shift_vertical(byte page_no, int dir, int scaled) {
		CSequencePage& page = get_page(page_no);
		return page.shift_vertical(dir, scaled? &g_scale : NULL, m_cfg.m_interpolate, get_default_value());
	}


	///////////////////////////////////////////////////////////////////////////////
//	inline int get_page_advance() {
	//	return m_cfg.m_page_advance;
//	}

	///////////////////////////////////////////////////////////////////////////////
	//void set_page_advance(int value) {
		//m_cfg.m_page_advance = value;
//	}

	///////////////////////////////////////////////////////////////////////////////
	inline int get_max_page_no() {
		return m_cfg.m_max_page_no;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Make sure that a page is available before trying to access it. When new
	// pages are addeded, they are initialised from the last existing page
	void prepare_page(int page) {
		while(m_cfg.m_max_page_no < page) {
			get_page(m_cfg.m_max_page_no+1) = get_page(m_cfg.m_max_page_no);
			++m_cfg.m_max_page_no;
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// Change the number of existing pages
	void set_max_page_no(int page) {
		prepare_page(page);	// in case the number of pages has increased
		m_cfg.m_max_page_no = page; // in case it has got less
	}

	///////////////////////////////////////////////////////////////////////////////
	inline V_SQL_SEQ_MODE get_mode() {
		return m_cfg.m_mode;
	}

	///////////////////////////////////////////////////////////////////////////////
	inline VIEW_TYPE get_view() {
		return m_state.m_view;
	}
	///////////////////////////////////////////////////////////////////////////////
	byte get_enabled() {
		return m_cfg.m_enabled;
	}

	///////////////////////////////////////////////////////////////////////////////
	void set_enabled(byte e) {
		m_cfg.m_enabled = e;
	}

	///////////////////////////////////////////////////////////////////////////////
	int get_scroll_ofs() {
		return m_state.m_scroll_ofs;
	}

	///////////////////////////////////////////////////////////////////////////////
	void set_scroll_ofs(int scroll_ofs) {
		m_state.m_scroll_ofs = scroll_ofs;
	}
	///////////////////////////////////////////////////////////////////////////////
	void set_loop_from(byte page_no, int from) {
		if(m_cfg.m_loop_per_page) {
			get_page(page_no).set_loop_from(from);
		}
		else {
			for(int i=0; i<MAX_PAGES; ++i) {
				get_page(i).set_loop_from(from);
			}
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	void set_loop_to(byte page_no, int to) {
		if(m_cfg.m_loop_per_page) {
			get_page(page_no).set_loop_to(to);
		}
		else {
			for(int i=0; i<MAX_PAGES; ++i) {
				get_page(i).set_loop_to(to);
			}
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	int get_loop_from(byte page_no) {
		if(m_cfg.m_loop_per_page) {
			return get_page(page_no).get_loop_from();
		}
		else {
			return get_page(0).get_loop_from();
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	int get_loop_to(byte page_no) {
		if(m_cfg.m_loop_per_page) {
			return get_page(page_no).get_loop_to();
		}
		else {
			return get_page(0).get_loop_to();
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	void clear_page_list() {
		m_cfg.m_page_list_count = 0;
		m_state.m_page_list_pos = 0;
	}
	///////////////////////////////////////////////////////////////////////////////
	int get_page_list_count() {
		return m_cfg.m_page_list_count;
	}

	///////////////////////////////////////////////////////////////////////////////
	byte add_to_page_list(byte page_no) {
		ASSERT(page_no >= 0 && page_no < MAX_PAGES);
		if(page_no > m_cfg.m_max_page_no) {
			return 0;
		}
		if(m_cfg.m_page_list_count < MAX_PAGE_LIST) {
			m_cfg.m_page_list[m_cfg.m_page_list_count++] = page_no;
			return 1;
		}
		return 0;
	}

	///////////////////////////////////////////////////////////////////////////////
	byte is_stepped() {
		return m_state.m_stepped;
	}
	///////////////////////////////////////////////////////////////////////////////
	byte is_page_advanced() {
		return m_state.m_page_advanced;
	}
	///////////////////////////////////////////////////////////////////////////////
	CSequenceStep& get_current_step() {
		return m_state.m_step_value;
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
	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////
	//
	// SEQUENCER FUNCTIONS
	//
	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////


	///////////////////////////////////////////////////////////////////////////////
	void send_midi_note(byte note, byte velocity) {
		if(m_cfg.m_midi_channel > V_SQL_MIDI_CHAN_NONE) {
			g_midi.send_note(m_cfg.m_midi_channel-V_SQL_MIDI_CHAN_1, note, velocity);
		}
	}


	///////////////////////////////////////////////////////////////////////////////
	void start(uint32_t ticks, byte parts_tick) {
		m_state.m_next_tick = ticks;

	}

	///////////////////////////////////////////////////////////////////////////////
	void stop_all_notes() {
/*		for(int i=0; i<MAX_PLAYING_NOTES;++i) {
			if(m_state.m_playing[i].count) {
				send_midi_note(m_state.m_playing[i].note, 0);
				m_state.m_playing[i].note = 0;
				m_state.m_playing[i].count = 0;
			}
		}*/
	}

	///////////////////////////////////////////////////////////////////////////////
	void tick(uint32_t ticks, byte parts_tick) {
		if(ticks >= m_state.m_next_tick) {
			m_state.m_next_tick += g_clock.ticks_per_measure(m_cfg.m_step_rate);
			m_state.m_step_timeout = g_clock.get_ms_per_measure(m_cfg.m_step_rate);
			calc_next_step(m_state.m_play_page_no, m_state.m_play_pos, m_state.m_page_advanced, m_state.m_page_list_pos);
			m_state.m_step_value = get_step(m_state.m_play_page_no, m_state.m_play_pos);
			m_state.m_stepped = 1;
		}
		else {
			m_state.m_stepped = 0;
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// called once per ms
	void ms_tick(int which) {
		if(m_state.m_gate_timeout) {
			if(!--m_state.m_gate_timeout) {
				g_outs.gate(which, COuts::GATE_CLOSED);
				send_midi_note(m_state.m_midi_note, 0);
				m_state.m_midi_note = 0;
			}
		}
		if(m_state.m_step_timeout) {
			--m_state.m_step_timeout;
		}
	}


	///////////////////////////////////////////////////////////////////////////////
	// the long value is MIDI notes * 65536
	long process_cv(byte which, long this_input) {

		long this_output;
		if((m_cfg.m_combine_prev == V_SQL_COMBINE_MASK ||
			m_cfg.m_combine_prev == V_SQL_COMBINE_ADD_MASK) &&
			!m_state.m_step_value.is_data_point()) {
			this_output = this_input;
		}
		else {
			long step_output;

			// get the scaled data point
			if(m_cfg.m_mode == V_SQL_SEQ_MODE_OFFSET) {
				step_output = COuts::SCALING*(m_state.m_step_value.get_value() - OFFSET_ZERO);
			}
			else {
				step_output = COuts::SCALING*m_state.m_step_value.get_value();
			}

			// check if we have an absolute volts range (1V - 8V). If so scale the output
			// accordingly (each volt will be 12 scale points)
			if(m_cfg.m_cv_scale < V_SQL_CVSCALE_1VOCT) {
				step_output = (step_output * (1 + m_cfg.m_cv_scale - V_SQL_CVSCALE_1V) * 12)/127;
			}

			// perform any addition of previous layer output
			if(m_cfg.m_combine_prev == V_SQL_COMBINE_ADD ||
				m_cfg.m_combine_prev == V_SQL_COMBINE_ADD_MASK) {
				this_output = this_input + step_output;
			}
			else {
				this_output = step_output;
			}
		}

		// apply octave shift
		if(m_cfg.m_cv_shift != V_SQL_CVSHIFT_NONE) {
			this_output = this_output + 12 * COuts::SCALING * (m_cfg.m_cv_shift - V_SQL_CVSHIFT_NONE);
		}

		// quantize the output to scale if needed
		switch(m_cfg.m_quantize) {
		case V_SQL_SEQ_QUANTIZE_CHROMATIC:
			if(this_output < 0) {
				this_output = 0;
			}
			this_output = COuts::SCALING * (this_output/COuts::SCALING);
			break;
		case V_SQL_SEQ_QUANTIZE_SCALE:
			if(this_output < 0) {
				this_output = 0;
			}
			this_output = COuts::SCALING * g_scale.force_to_scale(this_output/COuts::SCALING);
			break;
		}

		int glide_time;
		switch(m_cfg.m_cv_glide) {
		case V_SQL_CVGLIDE_ON:
			glide_time = m_state.m_step_timeout;
			break;
		case V_SQL_CVGLIDE_TIE:
			glide_time = (m_state.m_step_value.is_tied())? m_state.m_step_timeout : 0;
			break;
		case V_SQL_CVGLIDE_OFF:
		default:
			glide_time = 0;
		}

		// finally update the CV output
		g_outs.cv(which, this_output, m_cfg.m_cv_scale, glide_time);
		return this_output;
	}


	///////////////////////////////////////////////////////////////////////////////
	// Play the gate for a step
	void process_gate(byte which) {
		if(m_state.m_step_value.is_gate()) {
			// set the appropriate note duration
			switch(m_cfg.m_note_dur) {
			case V_SQL_NOTE_DUR_OPEN:
			case V_SQL_NOTE_DUR_LEGA:
				m_state.m_gate_timeout = INFINITE_GATE;	// stay open until the next gate
				break;
			case V_SQL_NOTE_DUR_TRIG:
				m_state.m_gate_timeout = COuts::TRIG_DURATION; // just a short trigger pulse
				break;
			case V_SQL_NOTE_DUR_100:
				m_state.m_gate_timeout = 0; // until the next step
				break;
			default: // other enumerations have integer values 0-10
				m_state.m_gate_timeout = (g_clock.get_ms_per_measure(m_cfg.m_step_rate) * m_cfg.m_note_dur ) / 10;
				break;
			}
			g_outs.gate(which, COuts::GATE_RETRIG);
		}
		else {
			if(!m_state.m_gate_timeout) {
				g_outs.gate(which, COuts::GATE_CLOSED);
			}
		}
	}

/*		if(step.is_gate()) {
			g_cv_gate.gate(which, CCVGate::GATE_RETRIG);
		}
		else if(step.is_tied()) {
			g_cv_gate.gate(which, CCVGate::GATE_OPEN);
		}
		else {
			g_cv_gate.gate(which, CCVGate::GATE_CLOSED);
		}
	}*/



};

//TODO
//const byte CSequenceLayer::c_tick_rates[V_SQL_STEP_RATE_MAX] = {96,72,48,36,32,24,18,16,12,9,8,6,4,3};
//const byte CSequenceLayer::c_step_duration[] = {3,4,6,8,9,12,16,18,24,32,36,48,72,96};

#endif /* SEQUENCE_LAYER_H_ */
