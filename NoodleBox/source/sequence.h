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

public:
	enum {
		NUM_LAYERS = 4,	// number of layers in the sequence
	};

private:
	CScale m_scale;
	CSequenceLayer m_layer_content[NUM_LAYERS];
	CSequenceLayer *m_layers[NUM_LAYERS];

	byte m_is_running;
public:

	///////////////////////////////////////////////////////////////////////////////
	CSequence() {
		for(int i=0; i<NUM_LAYERS; ++i) {
			m_layers[i] = &m_layer_content[i];
		}
	}

	void init() {
		for(int i=0; i<NUM_LAYERS; ++i) {
			m_layers[i]->init();
			m_layers[i]->set_id(i);
		}
		m_is_running = 0;
	}

	///////////////////////////////////////////////////////////////////////////////
	void init_state() {
		for(int i=0; i<NUM_LAYERS; ++i) {
			m_layers[i]->init_state();
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	void set(PARAM_ID param, int value) {
		switch(param) {
		case P_SQL_SCALE_TYPE: m_scale.set((V_SQL_SCALE_TYPE)value, m_scale.get_root()); break;
		case P_SQL_SCALE_ROOT: m_scale.set(m_scale.get_type(), (V_SQL_SCALE_ROOT)value); break;
		default: break;
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	int get(PARAM_ID param) {
		switch(param) {
		case P_SQL_SCALE_TYPE: return m_scale.get_type();
		case P_SQL_SCALE_ROOT: return m_scale.get_root();
		default:return 0;
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	int is_valid_param(PARAM_ID param) {
		return 1;
	}

	///////////////////////////////////////////////////////////////////////////////
	CSequenceLayer& get_layer(int index) {
		return *m_layers[index];
	}

	///////////////////////////////////////////////////////////////////////////////
	void move_layer(byte from, byte to) {
		int i;
		CSequenceLayer *layer = m_layers[from];
		for(i=from; i<NUM_LAYERS-1; ++i) {
			m_layers[i] = m_layers[i+1];
		}
		for(i=NUM_LAYERS-1; i>to; --i) {
			m_layers[i] = m_layers[i-1];
		}
		m_layers[to] = layer;
		for(i=0; i<NUM_LAYERS; ++i) {
			m_layers[i]->set_id(i);
		}
	}


	///////////////////////////////////////////////////////////////////////////////
	void event(int event, uint32_t param) {
		switch(event) {
		case EV_SEQ_RESTART:
		case EV_SEQ_CONTINUE:
			m_is_running = 1;
			break;
		case EV_SEQ_STOP:
			m_is_running = 0;
			break;
		}
		for(int i=0; i<NUM_LAYERS; ++i) {
			m_layers[i]->event(event,param);
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	byte is_running() {
		return m_is_running;
	}


	/////////////////////////////////////////////////////////////////////////////////////////////
	// called once per ms
	void run() {

		// ensure the sequencer is running
		if(m_is_running) {

			// get a random dice roll for any random triggers
			// this is a number between 1 and 16
			srand(g_clock.get_ms());
			int dice_roll = 1+rand()%16;

			// get the current clock tick count
			clock::TICKS_TYPE ticks = g_clock.get_ticks();

			// call the play method for each layer to run
			// the scheduling of that layer's playback
			byte played_step = 0;
			for(int i=0; i<NUM_LAYERS; ++i) {
				CSequenceLayer *layer = m_layers[i];
				if(layer->play(ticks, dice_roll)) {
					played_step = 1;
				}
			}

			// did any new step start playing?
			if(played_step) {

				// update each layer
				CV_TYPE prev_output = 0;
				for(int i=0; i<NUM_LAYERS; ++i) {
					CSequenceLayer *layer = m_layers[i];

					// update the voltage output of layer
					prev_output = layer->process_cv(prev_output);

					// if the layer has stepped, may need to trigger the gate
					if(layer->is_played_step()) {
						layer->process_gate();
						switch(layer->get_midi_out_mode()) {
						case V_SQL_MIDI_OUT_NOTE:
							layer->process_midi_note();
							break;
						case V_SQL_MIDI_OUT_CC:
							layer->process_midi_cc();
							break;
						}
					}
					else {
						layer->silence();
					}
				}
			}
		}

		// finally do once per ms housekeeping for layers
		for(int i=0; i<NUM_LAYERS; ++i) {
			m_layers[i]->run();
		}

	}

	/////////////////////////////////////////////////////////////////////////////////////////////
	static int get_cfg_size() {
		return CScale::get_cfg_size() + NUM_LAYERS * CSequenceLayer::get_cfg_size();
	}

	/////////////////////////////////////////////////////////////////////////////////////////////
	void get_cfg(byte **dest) {
		m_scale.get_cfg(dest);
		for(int i=0; i<NUM_LAYERS; ++i) {
			m_layers[i]->get_cfg(dest);
		}
	}


	/////////////////////////////////////////////////////////////////////////////////////////////
	void set_cfg(byte **src) {
		m_scale.set_cfg(src);
		for(int i=0; i<NUM_LAYERS; ++i) {
			m_layers[i]->set_cfg(src);
		}
	}

	void save_patch(int which) {
	}
	/*
		int address = PATCH_SLOT_SIZE * which;
		int cfg_size = get_cfg_size() + 2;
		byte buf[cfg_size] = {0};
		buf[cfg_size-2] = PATCH_DATA_COOKIE1;
		buf[cfg_size-1] = PATCH_DATA_COOKIE2;
		byte *ptr = buf;
		get_cfg(&ptr);
		if(!g_storage.write(address, buf, cfg_size)) {
			g_popup.text("ERROR");
		}
		else {
			g_popup.text("DONE");
		}
	}*/
	void load_patch(int which) {
	}
	/*
		int address = PATCH_SLOT_SIZE * which;
		int cfg_size = get_cfg_size() + 2;
		byte buf[cfg_size] = {0};
		if(!g_storage.read(address, buf, cfg_size)) {
			g_popup.text("ERROR");
		}
		else {
			if(buf[cfg_size-2] == PATCH_DATA_COOKIE1 && buf[cfg_size-1] == PATCH_DATA_COOKIE2) {
				byte *ptr = buf;
				set_cfg(&ptr);
				init_state();
				if(which) {
					g_popup.text("DONE");
				}
			}
			else {
				if(which) {
					g_popup.text("EMPTY");
				}
			}
	 	}
	}*/

};

CSequence g_sequence;

#endif /* SEQUENCE_H_ */
