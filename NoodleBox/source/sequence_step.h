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
//  SEQUENCER STEP
//
///////////////////////////////////////////////////////////////////////////////////

#ifndef SEQUENCE_STEP_H_
#define SEQUENCE_STEP_H_

// this class represents a single step in the sequence, both "data points" (as entered
// by the user) and any automatically generated points. It also stores the associated
// gate information
//
// the user enters "data points" and other values are extrapolated depending on the
// layer type
//
class CSequenceStep {
public:
	enum {
		GATE_NONE = 0,
		GATE_OPEN = 0x01,
		GATE_RETRIG = 0x03,
		GATE_ACCENT = 0x07
	};

	byte m_is_data_point:1; // is this a "user" data point rather than an automatic one?
	byte m_gate_type:3;
	byte m_value;

	void set_gate(int gate) {
		m_gate_type = gate;
	}
	int get_gate() {
		return m_gate_type;
	}
	inline int is_gate_open() {
		return (m_gate_type & 0x01);
	}
	inline byte is_trigger() {
		return (m_gate_type & 0x02);
	}
	inline byte is_accent() {
		return (m_gate_type & 0x04);
	}

	///////////////////////////////////////////////////////////////////////////////////
	void copy_data_point(CSequenceStep &other) {
		m_value = other.m_value;
		m_is_data_point = other.m_is_data_point;
	}

	///////////////////////////////////////////////////////////////////////////////////
	// clear data point and gates
	void reset_all(byte value = 0) {
		m_is_data_point = 0;
		m_gate_type = GATE_NONE;
		m_value = value;
	}

	///////////////////////////////////////////////////////////////////////////////////
	// clear data point but preserve gate
	void reset_data_point(byte value = 0) {
		m_is_data_point = 0;
		m_value = value;
	}

	///////////////////////////////////////////////////////////////////////////////////
	void clear_accent() {
		m_gate_type &= ~0x04;
	}

	///////////////////////////////////////////////////////////////////////////////
	void inc_gate() {
		switch(m_gate_type) {
		case GATE_NONE: m_gate_type = GATE_OPEN; break;
		case GATE_OPEN: m_gate_type = GATE_RETRIG; break;
		case GATE_RETRIG: m_gate_type = GATE_ACCENT; break;
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	void dec_gate() {
		switch(m_gate_type) {
		case GATE_RETRIG: m_gate_type = GATE_OPEN; break;
		case GATE_ACCENT: m_gate_type = GATE_RETRIG; break;
		case GATE_OPEN: m_gate_type = GATE_NONE; break;
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	void toggle_gate() {
		switch(m_gate_type) {
		case GATE_NONE: m_gate_type = GATE_RETRIG; break;
		default:m_gate_type = GATE_NONE; break;
		}
	}

};

#endif /* SEQUENCE_STEP_H_ */
