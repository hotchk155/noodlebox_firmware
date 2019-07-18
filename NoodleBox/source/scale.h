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
//  SCALE HANDLING
//
///////////////////////////////////////////////////////////////////////////////////

#ifndef SCALE_H_
#define SCALE_H_


/////////////////////////////////////////////////////////////////
// Scale class represents a musical scale that covers all the
// MIDI note range 0-127
class CScale {
	enum {
		DEFAULT_NOTE = 36,
		MAX_NOTE = 128,
		MAX_INDEX = ((7*MAX_NOTE)/12)
	};
	typedef struct {
		V_SQL_SCALE_TYPE m_type;
		V_SQL_SCALE_ROOT m_root;
	} CONFIG;

	CONFIG m_cfg;
	byte m_index_to_note[MAX_INDEX];
	byte m_note_to_index[MAX_NOTE];
	byte m_max_index;

	static CScale *m_instance;

	///////////////////////////////////////////////////////////////////////////////
	// build the note mapping tables for the selected scale type and root
	void build() {
		byte interval[7] = {
			2, 2, 1, 2, 2, 2, 1
		};
		byte ofs = (int)m_cfg.m_type; // 0 = ionian
		byte note_number = m_cfg.m_root;
		byte n2i_index = 0;
		m_max_index = 0;
		while(note_number < MAX_NOTE && m_max_index < MAX_INDEX) {
			m_index_to_note[m_max_index] = note_number;
			while(n2i_index <= note_number) {
				m_note_to_index[n2i_index++] = m_max_index; // sharpen out of key notes to next note
			}
			note_number += interval[ofs];
			if(++ofs >= 7) {
				ofs = 0;
			}
			++m_max_index;
		}
	}

public:
	///////////////////////////////////////////////////////////////////////////////
	// constructor
	CScale()  {
		m_instance = this;
		m_cfg.m_type = V_SQL_SCALE_TYPE_IONIAN;
		m_cfg.m_root = V_SQL_SCALE_ROOT_C;
		build();
	}

	///////////////////////////////////////////////////////////////////////////////
	static inline CScale& instance() {
		return *m_instance;
	}

	///////////////////////////////////////////////////////////////////////////////
	inline V_SQL_SCALE_TYPE get_type() {
		return m_cfg.m_type;
	}

	///////////////////////////////////////////////////////////////////////////////
	inline V_SQL_SCALE_ROOT get_root() {
		return m_cfg.m_root;
	}

	///////////////////////////////////////////////////////////////////////////////
	inline byte get_notes_per_octave() {
		return 7;
	}

	///////////////////////////////////////////////////////////////////////////////
	void set(V_SQL_SCALE_TYPE scale_type, V_SQL_SCALE_ROOT scale_root) {
		m_cfg.m_type = scale_type;
		m_cfg.m_root = scale_root;
		build();
	}

	/////////////////////////////////////////////////////////////////
	// return highest index in scale
	inline byte max_index() {
		return m_max_index;
	}

	/////////////////////////////////////////////////////////////////
	// return default note value forced to scale
	byte default_note() {
		return force_to_scale(DEFAULT_NOTE);
	}

	/*
	/////////////////////////////////////////////////////////////////
	// return default note value forced to scale and converted
	// to index within the current scale
	byte default_note_scaled() {
		return note_to_index(default_note_chromatic());
	}
*/
	/////////////////////////////////////////////////////////////////
	// convert scale index to MIDI note
	inline byte index_to_note(byte index) {
		//TODO assert() ??
		return m_index_to_note[index];
	}

	/////////////////////////////////////////////////////////////////
	// convert MIDI note to scale index
	inline byte note_to_index(byte note) {
		//TODO assert() ??
		return m_note_to_index[note];
	}

	/////////////////////////////////////////////////////////////////
	// force a MIDI note to valid MIDI note in scale, sharpening if needed
	inline byte force_to_scale(byte note) {
		byte index = m_note_to_index[note];
		return m_index_to_note[index];
	}

	/////////////////////////////////////////////////////////////////
	inline byte inc_note_in_scale(int& note, int dir) {
		int index = (int)m_note_to_index[note] + dir;
		if(index<0 || index>m_max_index) {
			return 0;
		}
		note = m_index_to_note[index];
		return 1;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////
	static int get_cfg_size() {
		return sizeof(CONFIG);
	}

	/////////////////////////////////////////////////////////////////
	void get_cfg(byte **dest) {
		memcpy((*dest), &m_cfg, sizeof m_cfg);
		(*dest) += sizeof m_cfg;
	}

	/////////////////////////////////////////////////////////////////
	void set_cfg(byte **src) {
		memcpy(&m_cfg, (*src), sizeof m_cfg);
		(*src) += sizeof m_cfg;
	}
};
CScale *CScale::m_instance = NULL;

#endif /* SCALE_H_ */
