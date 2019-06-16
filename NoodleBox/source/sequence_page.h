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
//  hotchk155/2019                                          Sixty-four pixels ltd
//
//  SEQUENCE PAGE
//
///////////////////////////////////////////////////////////////////////////////////
#ifndef SEQUENCE_PAGE_H_

// Holds a single page of the sequence
// Has methods which act on entire page
class CSequencePage {
public:
	enum {
		MAX_STEPS = 32,					// number of steps in page
	};
private:
	CSequenceStep m_step[MAX_STEPS];	// data value and gate for each step

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
			double value =  m_step[pos].get_value();
			double gradient = (m_step[end].get_value() - value)/num_points;
			while(--num_points > 0) {
				// wrap around the column
				if(++pos >= MAX_STEPS) {
					pos = 0;
				}
				value += gradient;
				m_step[pos].set_value((byte)(value+0.5));
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
			if(m_step[i].is_data_point()) {
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
				m_step[i].set_value(value);
			}
		}
		else if(prev_waypoint == first_waypoint) {
			// only one waypoint defined
			for(i=0; i<MAX_STEPS; ++i) {
				if(i!=prev_waypoint) {
					m_step[i].set_value(m_step[first_waypoint].get_value());
				}
			}
		}
		else {
			// multiple waypoints defined
			interpolate_section(prev_waypoint, first_waypoint);
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	void fill(byte value)
	{
		int i;
		int first_data_point = -1;
		for(i=0; i<MAX_STEPS; ++i) {
			if(m_step[i].is_data_point()) {
				if(first_data_point < 0) {
					first_data_point = i;
				}
				value = m_step[i].get_value();
			}
			else {
				m_step[i].set_value(value);
			}
		}
		if(first_data_point >= 0) {
			for(i=0; i<first_data_point; ++i) {
				m_step[i].set_value(value);
			}
		}
	}

public:

	///////////////////////////////////////////////////////////////////////////////
	void recalc(byte interpolation, byte default_value) {
		if(interpolation) {
			interpolate(default_value);
		}
		else {
			fill(default_value);
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	inline CSequenceStep get_step(int index) {
		return m_step[index];
	}


	///////////////////////////////////////////////////////////////////////////////
	void set_step(byte index, CSequenceStep& step, byte interpolate, byte default_value) {
		m_step[index] = step;
		recalc(interpolate, default_value);
	}

	///////////////////////////////////////////////////////////////////////////////
	void clear(byte default_value) {
		for(int i=0; i<MAX_STEPS; ++i) {
			m_step[i].clear();
		}
		recalc(0, default_value);
	}

	///////////////////////////////////////////////////////////////////////////////
	// shift pattern vertically up or down by one space
	byte shift_vertical(int dir) {
		// first make sure that the shift will not exceed the bounds at
		// the top of the bottom of the pattern
		for(int i = 0; i<MAX_STEPS; ++i) {
			int new_value = (int)m_step[i].get_value() + dir;
			if(new_value < 0 || new_value > 127) {
				return 0;
			}
		}

		// perform the actual shift of all the data points
		for(int i = 0; i<MAX_STEPS; ++i) {
			m_step[i].set_value(dir + (int)m_step[i].get_value());
		}

		return 1;
	}


	///////////////////////////////////////////////////////////////////////////////
	// shift pattern horizontally by one step
	void shift_horizontal(int dir) {
		CSequenceStep step;
		if(dir<0) {
			step = m_step[0];
			for(int i = 0; i<MAX_STEPS-1; ++i) {
				m_step[i] = m_step[i+1];
			}
			m_step[MAX_STEPS-1] = step;
		}
		else {
			step = m_step[MAX_STEPS-1];
			for(int i = MAX_STEPS-1; i>0; --i) {
				m_step[i] = m_step[i-1];
			}
			m_step[0] = step;
		}
	}

};

#define SEQUENCE_PAGE_H_

#endif /* SEQUENCE_PAGE_H_ */
