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

	byte m_gate_open:1;
	byte m_retrig:1;
	byte m_glide:1;
	byte m_prob:2;			// probabitity 0=Always, 1,2,3 = high medium low
	byte m_is_data_point:1; // is the CV value user defined rather than auto filled

	byte m_value;	// CV value
public:
	enum {
		PROB_OFF,
		PROB_HIGH,
		PROB_MED,
		PROB_LOW
	};
	inline byte get_value() {
		return m_value;
	}
	inline void set_value(byte value) {
		m_value = value;
	}

	inline byte is_data_point() {
		return m_is_data_point;
	}
	inline void set_data_point(byte value) {
		m_is_data_point = !!value;
	}

	inline int is_gate_open() {
		return m_gate_open;
	}
	inline byte is_trigger() {
		return m_retrig;
	}

	inline byte is_glide() {
		return m_glide;
	}
	inline byte get_prob() {
		return m_prob;
	}

	///////////////////////////////////////////////////////////////////////////////////
	void copy_data_point(CSequenceStep &other) {
		m_value = other.m_value;
		m_is_data_point = other.m_is_data_point;
	}

	///////////////////////////////////////////////////////////////////////////////////
	void copy_gate(CSequenceStep &other) {
		m_gate_open = other.m_gate_open;
		m_retrig = other.m_retrig;
		m_glide = other.m_glide;
		m_prob = other.m_prob;
	}

	///////////////////////////////////////////////////////////////////////////////////
	// clear data point and gates
	void reset_all(byte value = 0) {
		m_gate_open = 0;
		m_retrig = 0;
		m_glide = 0;
		m_prob = 0;
		m_is_data_point = 0;
		m_value = value;
	}

	///////////////////////////////////////////////////////////////////////////////////
	// clear data point but preserve gate
	void reset_data_point(byte value = 0) {
		m_is_data_point = 0;
		m_value = value;
	}

	///////////////////////////////////////////////////////////////////////////////////
	// order here is OFF->TRIG->OPEN->OFF
	void inc_gate() {
		if(m_retrig) {
			// TRIG -> OPEN
			m_gate_open = 1;
			m_retrig = 0;
		}
		else if(m_gate_open) {
			// OPEN -> OFF
			m_gate_open = 0;
		}
		else {
			// OFF->TRIG
			m_gate_open = 1;
			m_retrig = 1;
		}
	}

	///////////////////////////////////////////////////////////////////////////////////
	// glide is simply a toggle
	void inc_glide() {
		m_glide = !m_glide;
	}

	///////////////////////////////////////////////////////////////////////////////////
	// Prob order is OFF->HIGH->MED->LOW->OFF
	void inc_prob() {
		if(!m_prob) {
			m_prob = PROB_HIGH;
		}
		else {
			--m_prob;
		}
	}
};

#endif /* SEQUENCE_STEP_H_ */
