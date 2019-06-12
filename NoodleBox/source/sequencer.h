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
//  SEQUENCER
//
///////////////////////////////////////////////////////////////////////////////////

#ifndef SEQUENCER_H_
#define SEQUENCER_H_

///////////////////////////////////////////////////////////////////////////////
// SEQUENCER CLASS
class CSequencer {

	enum {
		NUM_LAYERS = 4,	// number of layers in the sequence
	};


	// Config info that forms part of the patch
	typedef struct {
		V_SQL_SCALE_TYPE 	m_scale_type;
		V_SQL_SCALE_ROOT 	m_scale_root;
//		byte 				m_midi_vel_accent;
//		byte 				m_midi_vel;
	} CONFIG;
	CONFIG m_cfg;

	byte m_is_running;
	int m_cur_layer;			// this is the current layer being viewed/edited
	CSequenceLayer m_layers[NUM_LAYERS];
	CScale m_scale;
public:

	///////////////////////////////////////////////////////////////////////////////
	// constructor
	CSequencer() : m_layers{m_scale,m_scale,m_scale,m_scale}
	{
		init_config();
		m_is_running = 0;
		m_cur_layer = 0;
	}


	///////////////////////////////////////////////////////////////////////////////
	// initialise the saved configuration
	void init_config() {
		m_cfg.m_scale_type = V_SQL_SCALE_TYPE_IONIAN;
		m_cfg.m_scale_root = V_SQL_SCALE_ROOT_C;
		m_scale.build(m_cfg.m_scale_type, m_cfg.m_scale_root);
	}

	///////////////////////////////////////////////////////////////////////////////
	// get a reference to the current layer object
	inline CSequenceLayer& cur_layer() {
		return m_layers[m_cur_layer];
	}

	///////////////////////////////////////////////////////////////////////////////
	// return current layer index 0-3
	inline int get_cur_layer() {
		return m_cur_layer;
	}

	///////////////////////////////////////////////////////////////////////////////
	// change current layer
	void set_cur_layer(int layer) {
		m_cur_layer = layer;
	}

	///////////////////////////////////////////////////////////////////////////////
	// config setter
	void set(PARAM_ID param, int value) {
		switch(param) {
		case P_SQL_SCALE_TYPE: m_cfg.m_scale_type = (V_SQL_SCALE_TYPE)value; m_scale.build(m_cfg.m_scale_type, m_cfg.m_scale_root); break;
		case P_SQL_SCALE_ROOT: m_cfg.m_scale_root = (V_SQL_SCALE_ROOT)value; m_scale.build(m_cfg.m_scale_type, m_cfg.m_scale_root); break;
		default: m_layers[m_cur_layer].set(param,value);
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// config getter
	int get(PARAM_ID param) {
		switch(param) {
		case P_SQL_SCALE_TYPE: return m_cfg.m_scale_type;
		case P_SQL_SCALE_ROOT: return m_cfg.m_scale_root;
		default: return m_layers[m_cur_layer].get(param);
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// config validator
	int is_valid_param(PARAM_ID param) {
		switch(param) {
		case P_SQL_SCALE_TYPE:
		case P_SQL_SCALE_ROOT:
			switch(cur_layer().get_mode()) {
			case V_SQL_SEQ_MODE_CHROMATIC:
			case V_SQL_SEQ_MODE_TRANSPOSE:
				return cur_layer().get(P_SQL_FORCE_SCALE) == V_SQL_FORCE_SCALE_ON;
			case V_SQL_SEQ_MODE_SCALE:
				return 1;
			case V_SQL_SEQ_MODE_MOD:
				return 0;
			default:
				break;
			}
			break;
		case P_SQL_MIDI_VEL:
		case P_SQL_MIDI_VEL_ACCENT:
			switch(cur_layer().get_mode()) {
			case V_SQL_SEQ_MODE_CHROMATIC:
			case V_SQL_SEQ_MODE_TRANSPOSE:
			case V_SQL_SEQ_MODE_SCALE:
				return 1;
			case V_SQL_SEQ_MODE_MOD:
				return 0;
			default:
				break;
			}
			break;
		default:
			break;
		}
		return cur_layer().is_valid_param(param);
	}

	///////////////////////////////////////////////////////////////////////////////
	void start() {
		m_is_running = 1;
		for(int i=0; i<NUM_LAYERS; ++i) {
			m_layers[i].start(g_clock.get_ticks(), g_clock.get_part_ticks());
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	void stop() {
		m_is_running = 0;
		for(int i=0; i<NUM_LAYERS; ++i) {
			m_layers[i].stop_all_notes();
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	void reset() {
		for(int i=0; i<NUM_LAYERS; ++i) {
			m_layers[i].reset();
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	byte is_running() {
		return m_is_running;
	}


	/////////////////////////////////////////////////////////////////////////////////////////////
	void run(uint32_t ticks, byte parts_tick) {

		for(int i=0; i<NUM_LAYERS; ++i) {
			m_layers[i].ms_tick(i);
		}
		// ensure the sequencer is running
		if(m_is_running) {

			// tick each layer
			for(int i=0; i<NUM_LAYERS; ++i) {
				m_layers[i].tick(ticks, parts_tick);
			}


			// process each layer
			for(int i=0; i<NUM_LAYERS; ++i) {
				CSequenceLayer& layer = m_layers[i];
				if(layer.is_stepped() && layer.get_enabled()) {
					switch(layer.get_mode()) {

						//////////////////////////////////////////////////
						case V_SQL_SEQ_MODE_MOD:
							layer.action_step_mod(i);
							layer.action_step_gate(i);
							break;

						//////////////////////////////////////////////////
						case V_SQL_SEQ_MODE_TRANSPOSE:
							layer.action_step_gate(i);
							break;

						//////////////////////////////////////////////////
						case V_SQL_SEQ_MODE_CHROMATIC:
						case V_SQL_SEQ_MODE_SCALE:
							layer.action_step_note(
									i,
									CSequenceStep(),
									//m_cfg.m_midi_vel_accent,
									//m_cfg.m_midi_vel,
									1
							);

							// note layers pass note information to subsequent transpose/velocity layers
							for(int j=i+1; j<NUM_LAYERS; ++j) {
								CSequenceLayer& other_layer = m_layers[j];
								if(other_layer.get_mode() == V_SQL_SEQ_MODE_SCALE ||
									other_layer.get_mode() == V_SQL_SEQ_MODE_CHROMATIC) {
									// another note layer found - stops current note layer providing any
									// info to further layers
									break;
								}
								else if(other_layer.get_mode() == V_SQL_SEQ_MODE_TRANSPOSE) {
									// transpose layer, action as a note layer passing in the
									// note from the active layer to be transposed
									other_layer.action_step_note(
											j,
											layer.get_current_step(),
											//m_cfg.m_midi_vel_accent,
											//m_cfg.m_midi_vel,
											0
									);
								}
								//else if(other_layer.get_mode() == V_SQL_SEQ_MODE_VELOCITY) {
								//	g_cv_gate.mod_cv(j, layer.get_last_velocity(), other_layer.get(P_SQL_CVRANGE),0,0);
								//}
							}

							break;


						default:
							break;
					}
				}
			}
		}
	}
};

CSequencer g_sequencer;

#endif /* SEQUENCER_H_ */
