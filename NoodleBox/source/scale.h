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

	byte m_index_to_note[8];
	byte m_note_to_index[12];
	byte m_max_index;
	enum {
		DEFAULT_NOTE = 36
	};
public:
	///////////////////////////////////////////////////////////////////////////////
	// constructor
	CScale() : m_max_index(0) {
	}

	///////////////////////////////////////////////////////////////////////////////
	// build the note mapping tables for the selected scale type and root
	void build(V_SQL_SCALE_TYPE scale_type, V_SQL_SCALE_ROOT scale_root) {
		byte interval[7] = {
			1, 1, 0, 1, 1, 1, 0
		};
		byte ofs = (int)scale_type; // 0 = ionian
		byte i2n_value = scale_root;
		byte n2i_index = 0;
		for(int i=0; i<8; ++i) {
			m_index_to_note[i] = i2n_value++;
			m_note_to_index[n2i_index++] = i;
			if(interval[ofs]) {
				i2n_value++;
				m_note_to_index[n2i_index++] = i;
			}
			if(++ofs >= 7) {
				ofs = 0;
			}
		}

		m_max_index = note_to_index(127);
	}

	/////////////////////////////////////////////////////////////////
	// return highest index in scale
	inline byte max_index() {
		return m_max_index;
	}

	/////////////////////////////////////////////////////////////////
	// return default note value forced to scale
	byte default_note_chromatic() {
		return force_to_scale(DEFAULT_NOTE);
	}

	/////////////////////////////////////////////////////////////////
	// return default note value forced to scale and converted
	// to index within the current scale
	byte default_note_scaled() {
		return note_to_index(default_note_chromatic());
	}

	/////////////////////////////////////////////////////////////////
	// convert scale index to MIDI note
	byte index_to_note(int index) {
		int octave = index/7;
		int note_in_scale = m_index_to_note[index%7];
		int note = 12*octave + note_in_scale;
		if (note < 0 || note > 127) {
			return 0;
		}
		return note;
	}

	/////////////////////////////////////////////////////////////////
	// convert MIDI note to scale index
	byte note_to_index(int note) {
		int octave = note/12;
		int note_in_scale = m_note_to_index[note%12];
		int index = 7*octave + note_in_scale;
		if (index < 0 || index > 127) {
			return 0;
		}
		return index;
	}

	/////////////////////////////////////////////////////////////////
	// force a MIDI note to valid MIDI note in scale, sharpening if needed
	byte force_to_scale(byte note) {
		int octave = note/12;
		int note_in_scale = m_note_to_index[note%12];
		if(note_in_scale > 11) {
			note_in_scale -= 12;
			++octave;
		}
		int result = 12 * octave + m_index_to_note[note_in_scale];
		if (result < 0 || result > 127) {
			return 0;
		}
		return result;
	}
};

#endif /* SCALE_H_ */
