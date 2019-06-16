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
	byte m_index_to_note[MAX_INDEX];
	byte m_note_to_index[MAX_NOTE];
	byte m_max_index;
	V_SQL_SCALE_TYPE m_type;
	V_SQL_SCALE_ROOT m_root;
public:
	///////////////////////////////////////////////////////////////////////////////
	// constructor
	CScale()  {
		m_max_index = 0;
	}

	///////////////////////////////////////////////////////////////////////////////
	inline V_SQL_SCALE_TYPE get_type() {
		return m_type;
	}

	///////////////////////////////////////////////////////////////////////////////
	inline V_SQL_SCALE_ROOT get_root() {
		return m_root;
	}

	///////////////////////////////////////////////////////////////////////////////
	inline byte get_notes_per_octave() {
		return 7;
	}

	///////////////////////////////////////////////////////////////////////////////
	// build the note mapping tables for the selected scale type and root
	void build(V_SQL_SCALE_TYPE scale_type, V_SQL_SCALE_ROOT scale_root) {
		byte interval[7] = {
			2, 2, 1, 2, 2, 2, 1
		};
		byte ofs = (int)scale_type; // 0 = ionian
		byte note_number = scale_root;
		byte n2i_index = 0;
		m_type = scale_type;
		m_root = scale_root;
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
};
CScale g_scale;

#endif /* SCALE_H_ */
