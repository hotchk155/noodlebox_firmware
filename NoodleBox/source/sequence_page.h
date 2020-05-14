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
// SEQUENCER PAGE
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
#ifndef SEQUENCE_PAGE_H_

// Holds a single page of the sequence
// Has methods which act on entire page
class CSequencePage {
	enum {
		DEFAULT_LOOP_FROM = 0,
		DEFAULT_LOOP_TO = 15
	};
public:
	enum {
		MAX_STEPS = 32,					// number of steps in page
	};
private:
	typedef struct {
		CSequenceStep 	m_step[MAX_STEPS];	// data value and gate for each step
		byte 			m_loop_from;		// loop start point
		byte 			m_loop_to;			// loop end point
	} CONFIG;
	CONFIG m_cfg;

	///////////////////////////////////////////////////////////////////////////////
	// Create interpolated points between two waypoints
	void interpolate_section(int pos, int end)
	{
		// calculate the number of new points that we will need to
		// crate during the interpolation
		int num_points = end - pos;
		if(num_points < 0) {
			num_points += MAX_STEPS;
		}
		if(num_points > 0) {

			// starting point and gradient
			double value =  m_cfg.m_step[pos].get_value();
			double gradient = (m_cfg.m_step[end].get_value() - value)/num_points;
			while(--num_points > 0) {
				// wrap around the column
				if(++pos >= MAX_STEPS) {
					pos = 0;
				}
				value += gradient;
				m_cfg.m_step[pos].set_value((byte)(value+0.5));
			}
		}
	}


	///////////////////////////////////////////////////////////////////////////////
	// create interpolated points between all user data points in pattern
	void interpolate(byte value)
	{
		int i;
		int first_waypoint = -1;
		int prev_waypoint = -1;
		for(i=0; i<MAX_STEPS; ++i) {
			if(m_cfg.m_step[i].is(CSequenceStep::DATA_POINT)) {
				if(prev_waypoint < 0) {
					first_waypoint = i;
				}
				else {
					interpolate_section(prev_waypoint, i);
				}
				prev_waypoint = i;
			}
		}

		if(first_waypoint < 0) {
			// no waypoints defined
			for(i=0; i<MAX_STEPS; ++i) {
				m_cfg.m_step[i].set_value(value);
			}
		}
		else if(prev_waypoint == first_waypoint) {
			// only one waypoint defined
			for(i=0; i<MAX_STEPS; ++i) {
				if(i!=prev_waypoint) {
					m_cfg.m_step[i].set_value(m_cfg.m_step[first_waypoint].get_value());
				}
			}
		}
		else {
			// multiple waypoints defined
			interpolate_section(prev_waypoint, first_waypoint);
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	void pad(byte value)
	{
		int i;
		int first_data_point = -1;
		for(i=0; i<MAX_STEPS; ++i) {
			if(m_cfg.m_step[i].is(CSequenceStep::DATA_POINT)) {
				if(first_data_point < 0) {
					first_data_point = i;
				}
				value = m_cfg.m_step[i].get_value();
			}
			else {
				m_cfg.m_step[i].set_value(value);
			}
		}
		if(first_data_point >= 0) {
			for(i=0; i<first_data_point; ++i) {
				m_cfg.m_step[i].set_value(value);
			}
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	void zero_fill(byte zero_value)
	{
		for(int i=0; i<MAX_STEPS; ++i) {
			if(!m_cfg.m_step[i].is(CSequenceStep::DATA_POINT)) {
				m_cfg.m_step[i].set_value(zero_value);
			}
		}
	}

public:
	CSequencePage() {
		m_cfg.m_loop_from = DEFAULT_LOOP_FROM;
		m_cfg.m_loop_to = DEFAULT_LOOP_TO;
	}

	///////////////////////////////////////////////////////////////////////////////
	void recalc(V_SQL_FILL_MODE fill_mode, byte zero_value) {
		switch(fill_mode) {
		case V_SQL_FILL_MODE_PAD:
			pad(zero_value);
			break;
		case V_SQL_FILL_MODE_INTERPOLATE:
			interpolate(zero_value);
			break;
		case V_SQL_FILL_MODE_OFF:
			zero_fill(zero_value);
			break;
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	inline CSequenceStep get_step(int index) {
		ASSERT(index>=0 && index < MAX_STEPS);
		return m_cfg.m_step[index];
	}


	///////////////////////////////////////////////////////////////////////////////
	void set_step(byte index, CSequenceStep& source, V_SQL_FILL_MODE fill_mode, byte zero_value, CSequenceStep::DATA what, byte auto_data_point) {
		ASSERT(index>=0 && index < MAX_STEPS);
		CSequenceStep& dest = m_cfg.m_step[index];

		// see if we might need to promote a fill point to a data point so that it can retain its value
		if(auto_data_point && (what&CSequenceStep::CV_DATA) && (source.get_value() != dest.get_value())) {
			// create a new data point
			dest.copy(source, what);
			dest.set(CSequenceStep::DATA_POINT, 1);
			recalc(fill_mode, zero_value);
		}
		else {
			// paste the new step value and recalc fill points
			dest.copy(source, what);
			recalc(fill_mode, zero_value);
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	void clear_step(byte index, V_SQL_FILL_MODE fill_mode, byte zero_value, CSequenceStep::DATA what) {
		ASSERT(index>=0 && index < MAX_STEPS);
		m_cfg.m_step[index].clear(what);
		recalc(fill_mode, zero_value);
	}

	///////////////////////////////////////////////////////////////////////////////
	inline void set_loop_from(byte index) {
		ASSERT(index>=0 && index < MAX_STEPS);
		m_cfg.m_loop_from = index;
	}

	///////////////////////////////////////////////////////////////////////////////
	inline byte get_loop_from() {
		return m_cfg.m_loop_from;
	}

	///////////////////////////////////////////////////////////////////////////////
	inline void set_loop_to(byte index) {
		ASSERT(index>=0 && index < MAX_STEPS);
		m_cfg.m_loop_to = index;
	}

	///////////////////////////////////////////////////////////////////////////////
	inline byte get_loop_to() {
		return m_cfg.m_loop_to;
	}
	///////////////////////////////////////////////////////////////////////////////
	void clear(byte zero_value) {
		for(int i=0; i<MAX_STEPS; ++i) {
			m_cfg.m_step[i].clear(CSequenceStep::ALL_DATA);
		}
		recalc(V_SQL_FILL_MODE_OFF, zero_value);
		m_cfg.m_loop_from = DEFAULT_LOOP_FROM;
		m_cfg.m_loop_to = DEFAULT_LOOP_TO;
	}

	///////////////////////////////////////////////////////////////////////////////
	// shift pattern vertically up or down by one space
	byte shift_vertical(int dir, CScale *scale, V_SQL_FILL_MODE fill_mode, byte zero_value, byte allow_clip) {
		int value[MAX_STEPS];
		byte changed = 0;
		for(int i=0; i<MAX_STEPS; ++i) {
			if(m_cfg.m_step[i].is(CSequenceStep::DATA_POINT)) {
				value[i] = m_cfg.m_step[i].get_value();
				if(scale) {
					if(scale->inc_note_in_scale(value[i],dir)) {
						changed = 1;
					}
					else if(!allow_clip) {
						return 0;
					}
				}
				else {
					int new_value = value[i] + dir;
					if(new_value >= 0 && new_value <= 127) {
						value[i] = new_value;
						changed = 1;
					}
					else if(!allow_clip) {
						return 0;
					}
				}
			}
			else {
				value[i] = 0;
			}
		}

		// any point changed?
		if(!changed) {
			return 0;
		}

		// perform the actual shift of all the data points
		for(int i = 0; i<MAX_STEPS; ++i) {
			m_cfg.m_step[i].set_value(value[i]);
		}

		recalc(fill_mode, zero_value);
		return 1;
	}


	///////////////////////////////////////////////////////////////////////////////
	// shift pattern horizontally by one step
	void shift_horizontal(int dir) {
		CSequenceStep step;
		if(dir<0) {
			step = m_cfg.m_step[0];
			for(int i = 0; i<MAX_STEPS-1; ++i) {
				m_cfg.m_step[i] = m_cfg.m_step[i+1];
			}
			m_cfg.m_step[MAX_STEPS-1] = step;
		}
		else {
			step = m_cfg.m_step[MAX_STEPS-1];
			for(int i = MAX_STEPS-1; i>0; --i) {
				m_cfg.m_step[i] = m_cfg.m_step[i-1];
			}
			m_cfg.m_step[0] = step;
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////////
	int get_mean_data_point(int default_value) {
		int sum = 0;
		int count = 0;
		for(int i = 0; i<MAX_STEPS-1; ++i) {
			if(m_cfg.m_step[i].is(CSequenceStep::DATA_POINT)) {
				sum += m_cfg.m_step[i].get_value();
				++count;
			}
		}
		return count? (int)(0.5+sum/count) : default_value;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////
	byte any_of(CSequenceStep::POINT_TYPE type, int from=0, int to=MAX_STEPS-1) {
		while(from<to && from < MAX_STEPS) {
			if(m_cfg.m_step[from].is(type)) {
				return 1;
			}
			++from;
		}
		return 0;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////
	int count_of(CSequenceStep::POINT_TYPE type, int from=0, int to=MAX_STEPS-1) {
		int count = 0;
		while(from<to && from < MAX_STEPS) {
			if(m_cfg.m_step[from].is(type)) {
				++count;
			}
			++from;
		}
		return count;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////
	void add_noise(int seed, int level, byte default_value, V_SQL_FILL_MODE fill_mode, byte zero_value) {
		srand(seed);
		for(int i=0; i<MAX_STEPS; ++i) {

			CSequenceStep& step = m_cfg.m_step[i];
			int value = step.get_value();
			value += (level * ((rand()&255)-(rand()&255)))/(255);
			if(value<0) {
				value = 0;
			}
			if(value>127) {
				value = 127;
			}
			if(step.get_value() != value) {
				step.set(CSequenceStep::DATA_POINT, 1);
				step.set_value(value);
			}
		}
		recalc(fill_mode, zero_value);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////
	void randomise(int seed, byte default_value, V_SQL_FILL_MODE fill_mode, byte zero_value) {

		srand(seed);
		for(int i=0; i<MAX_STEPS; ++i) {
			CSequenceStep& step = m_cfg.m_step[i];
			step.clear(CSequenceStep::ALL_DATA);
			if((rand()%10)<5) {
				step.set_value(default_value+(rand()%12)-(rand()%12));
				step.set(CSequenceStep::DATA_POINT, 1);
			}
			if((rand()%10)<2) {
				step.set(CSequenceStep::TRIG_POINT, 1);
			}
			if((rand()%10)<2) {
				step.set(CSequenceStep::TIE_POINT, 1);
			}
		}
		recalc(fill_mode, zero_value);
	}

	void replace_gates(int onsets, int positions) {
		for(int i=0; i<MAX_STEPS; ++i) {
			CSequenceStep& step = m_cfg.m_step[i];
			step.clear(CSequenceStep::GATE_DATA);
		}
		if(positions>0 && onsets>0) {
			double rate = (double)positions/onsets;
			double pos = 0.0;
			while((int)pos < MAX_STEPS-1) {
				CSequenceStep& step = m_cfg.m_step[(int)pos];
				step.set(CSequenceStep::TRIG_POINT, 1);
				pos += rate;
			}
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////////
	void init_state() {

	}

	/////////////////////////////////////////////////////////////////////////////////////////////
	static int get_cfg_size() {
		return sizeof(CONFIG);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////
	void get_cfg(byte **dest) {
		*((CONFIG*)*dest) = m_cfg;
		(*dest) += sizeof m_cfg;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////
	void set_cfg(byte **src) {
		m_cfg = *((CONFIG*)*src);
		(*src) += sizeof m_cfg;
	}

};

#define SEQUENCE_PAGE_H_

#endif /* SEQUENCE_PAGE_H_ */
