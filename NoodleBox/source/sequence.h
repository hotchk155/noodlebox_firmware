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
			m_cfg.m_layers[i].init(i);
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

		// once per ms housekeeping for layers
		for(int i=0; i<NUM_LAYERS; ++i) {
			m_cfg.m_layers[i].ms_tick(i);
		}

		// ensure the sequencer is running
		if(m_is_running) {

			// get a random dice roll for any random triggers
			// this is a number between 1 and 16
			srand(g_clock.m_ms);
			int dice_roll = 1+rand()%16;

			// tick each layer
			byte any_step = 0;
			for(int i=0; i<NUM_LAYERS; ++i) {
				CSequenceLayer& layer = m_cfg.m_layers[i];
				layer.tick(ticks, parts_tick, dice_roll);
				if(layer.is_stepped() && layer.get_enabled()) {
					any_step = 1;
				}
			}

			// did the tick cause any enabled layer to step?
			if(any_step) {

				// process each layer
				long prev_output = 0;
				for(int i=0; i<NUM_LAYERS; ++i) {
					CSequenceLayer& layer = m_cfg.m_layers[i];
					if(layer.get_enabled()) {
						prev_output = layer.process_cv(i,prev_output);
						if(layer.is_stepped()) {
							layer.process_gate(i);
							switch(layer.get_midi_out_mode()) {
							case V_SQL_MIDI_OUT_NOTE:
								layer.process_midi_note();
								break;
							case V_SQL_MIDI_OUT_CC:
								layer.process_midi_cc();
								break;
							}
						}
					}
					else {
						// ensure the gate for a disabled layer is closed
						layer.stop_note(i);
					}
				}
			}
		}
	}
};

CSequence g_sequence;

#endif /* SEQUENCE_H_ */
