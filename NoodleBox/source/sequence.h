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

#ifndef SEQUENCE_H_
#define SEQUENCE_H_

///////////////////////////////////////////////////////////////////////////////
// SEQUENCER CLASS
class CSequence {

	enum {
		NUM_LAYERS = 4,	// number of layers in the sequence
	};


	// Config info that forms part of the patch
	typedef struct {
		CScale& m_scale;
		CSequenceLayer m_layers[NUM_LAYERS];
	} CONFIG;
	CONFIG m_cfg;

	byte m_is_running;
public:

	CSequence() : m_cfg {g_scale} {}

	void init() {
		init_config();
		m_is_running = 0;
	}

	///////////////////////////////////////////////////////////////////////////////
	// initialise the saved configuration
	void init_config() {
		m_cfg.m_scale.build(V_SQL_SCALE_TYPE_IONIAN, V_SQL_SCALE_ROOT_C);
		for(int i=0; i<NUM_LAYERS; ++i) {
			m_cfg.m_layers[i].init();
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	CSequenceLayer& get_layer(int index) {
		return m_cfg.m_layers[index];
	}
	///////////////////////////////////////////////////////////////////////////////
	void start() {
		m_is_running = 1;
		for(int i=0; i<NUM_LAYERS; ++i) {
			m_cfg.m_layers[i].start(g_clock.get_ticks(), g_clock.get_part_ticks());
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	void stop() {
		m_is_running = 0;
		for(int i=0; i<NUM_LAYERS; ++i) {
			m_cfg.m_layers[i].stop_all_notes();
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	void reset() {
		for(int i=0; i<NUM_LAYERS; ++i) {
			m_cfg.m_layers[i].reset();
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	byte is_running() {
		return m_is_running;
	}


	/////////////////////////////////////////////////////////////////////////////////////////////
	void run(uint32_t ticks, byte parts_tick) {

		for(int i=0; i<NUM_LAYERS; ++i) {
			m_cfg.m_layers[i].ms_tick(i);
		}
		// ensure the sequencer is running
		if(m_is_running) {

			// tick each layer
			for(int i=0; i<NUM_LAYERS; ++i) {
				m_cfg.m_layers[i].tick(ticks, parts_tick);
			}


			// process each layer
			for(int i=0; i<NUM_LAYERS; ++i) {
				CSequenceLayer& layer = m_cfg.m_layers[i];
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
						case V_SQL_SEQ_MODE_PITCH:
							layer.action_step_note(
									i,
									CSequenceStep(),
									//m_cfg.m_midi_vel_accent,
									//m_cfg.m_midi_vel,
									1
							);

							// note layers pass note information to subsequent transpose/velocity layers
							for(int j=i+1; j<NUM_LAYERS; ++j) {
								CSequenceLayer& other_layer = m_cfg.m_layers[j];
								if(other_layer.get_mode() == V_SQL_SEQ_MODE_PITCH) {
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

CSequence g_sequence;

#endif /* SEQUENCE_H_ */
