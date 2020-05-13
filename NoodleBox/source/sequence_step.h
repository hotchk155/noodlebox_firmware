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

	typedef struct {
		byte m_trig:1;
		byte m_tie:1;
		byte m_accent:1;
		byte m_prob:4;
		byte m_retrig:4;
	} GATE_TYPE;

	typedef struct {
		byte m_value:7;	// CV value
		byte m_is_data_point:1; // is the CV value user defined rather than auto filled
	} CV_TYPE;

	GATE_TYPE 	m_gate;
	CV_TYPE 	m_cv;
public:
	enum {
		VALUE_MAX = 127,
		PROB_MAX = 15,
		RETRIG_MAX = 15,
	};

	typedef enum: byte {
		CV_DATA = 1,
		GATE_DATA = 2,
		ALL_DATA = CV_DATA|GATE_DATA
	} DATA;

	typedef enum: byte {
		DATA_POINT,
		TRIG_POINT,
		TIE_POINT,
		ACCENT_POINT
	} POINT_TYPE;

	///////////////////////////////////////////////////////////////////////////////////
	CSequenceStep() {
		clear(ALL_DATA);
	}

	///////////////////////////////////////////////////////////////////////////////////
	inline byte get_value() {
		return m_cv.m_value;
	}

	///////////////////////////////////////////////////////////////////////////////////
	inline void set_value(byte value) {
		ASSERT(value<=VALUE_MAX);
		m_cv.m_value = value;
	}

	///////////////////////////////////////////////////////////////////////////////////
	inline byte is(POINT_TYPE type) {
		switch(type) {
		case DATA_POINT:
			return !!m_cv.m_is_data_point;
		case TRIG_POINT:
			return !!m_gate.m_trig;
		case TIE_POINT:
			return !!m_gate.m_tie;
		case ACCENT_POINT:
			return !!m_gate.m_accent;
		}
		return 0;
	}

	///////////////////////////////////////////////////////////////////////////////////
	inline byte set(POINT_TYPE type, byte value) {
		switch(type) {
		case DATA_POINT:
			m_cv.m_is_data_point = !!value;
			break;
		case TRIG_POINT:
			m_gate.m_trig = !!value;
			break;
		case TIE_POINT:
			return m_gate.m_tie = !!value;
		case ACCENT_POINT:
			return m_gate.m_accent = !!value;
		}
		return 0;
	}

	///////////////////////////////////////////////////////////////////////////////////
	inline byte get_prob() {
		return m_gate.m_prob;
	}

	///////////////////////////////////////////////////////////////////////////////////
	void set_prob(byte prob) {
		ASSERT(prob<=PROB_MAX);
		m_gate.m_prob = prob;
	}

	///////////////////////////////////////////////////////////////////////////////////
	inline byte get_retrig() {
		return m_gate.m_retrig;
	}

	///////////////////////////////////////////////////////////////////////////////////
	void set_retrig(byte retrig) {
		ASSERT(retrig<=RETRIG_MAX);
		m_gate.m_retrig = retrig;
	}

	///////////////////////////////////////////////////////////////////////////////////
	// clear data point and gates
	void clear(DATA what) {
		if(what&CV_DATA) {
			memset(&m_cv, 0, sizeof m_cv);
		}
		if(what&GATE_DATA) {
			memset(&m_gate, 0, sizeof m_gate);
		}
	}

	///////////////////////////////////////////////////////////////////////////////////
	// clear data point and gates
	void copy(CSequenceStep &other, DATA what) {
		if(what&CV_DATA) {
			m_cv = other.m_cv;
		}
		if(what&GATE_DATA) {
			m_gate = other.m_gate;
		}
	}

};

#endif /* SEQUENCE_STEP_H_ */
