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
	byte m_gate:1;
	byte m_tie:1;
	byte m_prob:4;			// probabitity 0=Always, 1,2,3 = high medium low
	byte m_retrig:4;			// Retrig 0=Never, 1,2,3 = high medium low
	byte m_is_data_point:1; // is the CV value user defined rather than auto filled
	byte m_value;	// CV value
public:
	enum {
		PROB_MAX = 15,
		RETRIG_MAX = 15
	};

	typedef enum: byte {
		CV_DATA = 1,
		GATE_DATA = 2,
		ALL_DATA = CV_DATA|GATE_DATA
	} DATA;

	///////////////////////////////////////////////////////////////////////////////////
	inline byte get_value() {
		return m_value;
	}

	///////////////////////////////////////////////////////////////////////////////////
	inline void set_value(byte value) {
		m_value = value;
	}

	///////////////////////////////////////////////////////////////////////////////////
	inline byte is_data_point() {
		return m_is_data_point;
	}

	///////////////////////////////////////////////////////////////////////////////////
	inline void set_data_point(byte value) {
		m_is_data_point = !!value;
	}

	///////////////////////////////////////////////////////////////////////////////////
	inline void set_gate(byte gate) {
		m_gate = !!gate;
	}

	///////////////////////////////////////////////////////////////////////////////////
	inline int get_gate() {
		return m_gate;
	}

	///////////////////////////////////////////////////////////////////////////////////
	inline void set_tie(byte tie) {
		m_tie = !!tie;
	}

	///////////////////////////////////////////////////////////////////////////////////
	inline byte get_tie() {
		return m_tie;
	}

	///////////////////////////////////////////////////////////////////////////////////
	inline byte get_prob() {
		return m_prob;
	}

	///////////////////////////////////////////////////////////////////////////////////
	void set_prob(byte prob) {
		m_prob = prob;
	}

	///////////////////////////////////////////////////////////////////////////////////
	inline byte get_retrig() {
		return m_retrig;
	}

	///////////////////////////////////////////////////////////////////////////////////
	void set_retrig(byte retrig) {
		m_retrig = retrig;
	}

	///////////////////////////////////////////////////////////////////////////////////
	// clear data point and gates
	void clear(DATA what) {
		if(what&CV_DATA) {
			m_is_data_point = 0;
			m_value = 0;
		}
		if(what&GATE_DATA) {
			m_gate = 0;
			m_tie = 0;
			m_prob = 0;
			m_retrig = 0;
		}
	}

	///////////////////////////////////////////////////////////////////////////////////
	// clear data point and gates
	void copy(CSequenceStep &other, DATA what) {
		if(what&CV_DATA) {
			m_value = other.m_value;
			m_is_data_point = other.m_is_data_point;
		}
		if(what&GATE_DATA) {
			m_gate = other.m_gate;
			m_tie = other.m_tie;
			m_prob = other.m_prob;
			m_retrig = other.m_retrig;
		}
	}

};

#endif /* SEQUENCE_STEP_H_ */
