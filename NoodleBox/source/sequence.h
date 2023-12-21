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
// SEQUENCER TOP LEVEL
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
#ifndef SEQUENCE_H_
#define SEQUENCE_H_

///////////////////////////////////////////////////////////////////////////////
// SEQUENCER CLASS
class CSequence {

	enum {
		NRPNH_GLOBAL = 1,
		NRPNH_LAYER1 = 21,
		NRPNH_LAYER2 = 22,
		NRPNH_LAYER3 = 23,
		NRPNH_LAYER4 = 24,
		NRPNL_VOLTS = 15,
		NRPNL_CAL_SCALE = 98,
		NRPNL_CAL_OFFSET = 99,
		NRPNL_SAVE_CONFIG = 100
	};

public:
	enum {
		NUM_LAYERS = 4,	// number of layers in the sequence
		MIDI_TRANSPOSE_RANGE = 24,
		MIDI_TRANSPOSE_MIN = (MIDI_TRANSPOSE_ZERO - MIDI_TRANSPOSE_RANGE),
		MIDI_TRANSPOSE_MAX = (MIDI_TRANSPOSE_ZERO + MIDI_TRANSPOSE_RANGE)
	};

private:
	CScale m_scale;
	CSequenceLayer m_layer_content[NUM_LAYERS];
	CSequenceLayer *m_layers[NUM_LAYERS];

	// this array will receive the current step value from each of the
	// four layers, regardless of whether the layer has advanced to a
	// new step or whether the layer is muted
	CSequenceStep m_step_value[NUM_LAYERS];
	CV_TYPE m_step_output[NUM_LAYERS];

	// whether the sequencer is running
	byte m_is_running;

	// MIDI recording info
	int m_rec_layer;
	CSequenceLayer::REC_SESSION m_rec;

	// calibration voltage
	V_SEQ_OUT_CAL m_cal_mode;

public:

	///////////////////////////////////////////////////////////////////////////////
	CSequence() {
	}

	///////////////////////////////////////////////////////////////////////////////
	void init() {
		for(int i=0; i<NUM_LAYERS; ++i) {
			m_layers[i] = &m_layer_content[i];
			m_layers[i]->init();
			m_layers[i]->set_id(i);
		}
		m_is_running = 0;
		m_cal_mode = V_SEQ_OUT_CAL_NONE;
	}

	///////////////////////////////////////////////////////////////////////////////
	void clear() {
		for(int i=0; i<NUM_LAYERS; ++i) {
			m_layers[i]->clear();
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	void init_state() {
		for(int i=0; i<NUM_LAYERS; ++i) {
			m_layers[i] = &m_layer_content[i];
			m_layers[i]->set_id(i);
			m_layers[i]->init_state();
		}
		m_rec_layer = -1;

		m_rec.arm = V_SEQ_REC_ARM_OFF;
		m_rec.mode = V_SEQ_REC_MODE_NONE;
		m_rec.note = 0;
	}

	///////////////////////////////////////////////////////////////////////////////
	void set(PARAM_ID param, int value) {
		switch(param) {
		case P_SEQ_REC_ARM: m_rec.arm = (V_SEQ_REC_ARM)value; break;
		case P_SEQ_REC_MODE: m_rec.mode = (V_SEQ_REC_MODE)value; break;
		case P_SEQ_SCALE_TYPE: m_scale.set((V_SQL_SCALE_TYPE)value, m_scale.get_root()); break;
		case P_SEQ_SCALE_ROOT: m_scale.set(m_scale.get_type(), (V_SQL_SCALE_ROOT)value); break;
		case P_SEQ_OUT_CAL: m_cal_mode = (V_SEQ_OUT_CAL)value; set_cal_volts(); break;
		default: break;
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	int get(PARAM_ID param) {
		switch(param) {
		case P_SEQ_REC_ARM: return m_rec.arm;
		case P_SEQ_REC_MODE: return m_rec.mode;
		case P_SEQ_SCALE_TYPE: return m_scale.get_type();
		case P_SEQ_SCALE_ROOT: return m_scale.get_root();
		case P_SEQ_OUT_CAL: return m_cal_mode;
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
		case EV_REAPPLY_CAL_VOLTS:
			set_cal_volts();
			return;
		case EV_SEQ_RESTART:
		case EV_SEQ_CONTINUE:
			m_is_running = 1;
			break;
		case EV_SEQ_STOP:
			m_is_running = 0;
			break;
		case EV_SAVE_OK:
			if(param>SLOT_CONFIG) {
				save_patch_complete(param);
			}
			break;
		case EV_LOAD_OK:
			if(param>SLOT_CONFIG) {
				load_patch_complete(param);
			}
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

	///////////////////////////////////////////////////////////////////////////////
	int is_cal_mode() {
		return (m_cal_mode != V_SEQ_OUT_CAL_NONE);
	}

	///////////////////////////////////////////////////////////////////////////////
	void set_cal_volts() {
		if(m_cal_mode != V_SEQ_OUT_CAL_NONE) {
			int volts = m_cal_mode - V_SEQ_OUT_CAL_NONE;
			for(int i=0; i<NUM_LAYERS; ++i) {
				m_layers[i]->set_cal_volts(volts);
			}
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	void silence() {
		for(int i=0; i<NUM_LAYERS; ++i) {
			m_layers[i]->silence();
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	void midi_note_on(int layer, byte note, byte vel) {
		m_rec_layer = layer;
		m_rec.note = note;

		if(m_rec_layer >= 0 && m_rec.mode == V_SEQ_REC_MODE_TRANSPOSE) {
			if(note>=MIDI_TRANSPOSE_MIN && note<=MIDI_TRANSPOSE_MAX) {
				m_layers[layer]->set(P_SQL_CV_TRANSPOSE, (int)note-MIDI_TRANSPOSE_ZERO);
				fire_event(EV_REPAINT_MENU,0);
			}
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	void midi_note_off() {
		if(m_rec_layer >= 0 && m_rec.mode == V_SEQ_REC_MODE_TRANSPOSE && m_rec.arm != V_SEQ_REC_ARM_ON) {
			m_layers[m_rec_layer]->set(P_SQL_CV_TRANSPOSE, 0);
			fire_event(EV_REPAINT_MENU,0);
		}

		m_rec_layer = -1;
		m_rec.note = 0;
	}

	///////////////////////////////////////////////////////////////////////////////
	void handle_nrpn(int nrpn_hi, int nrpn_lo, int value_hi, int value_lo) {

		// currently only used in calibration mode
		if(m_cal_mode != V_SEQ_OUT_CAL_NONE) {
			int layer;
			int value = value_hi? -value_lo : value_lo;
			switch(nrpn_hi) {

			// 1/100/x/x - SAVE CONFIG
			case NRPNH_GLOBAL:
				if(nrpn_lo == NRPNL_SAVE_CONFIG) {
					save_config();
				}
				break;

			case NRPNH_LAYER1: // 21/x/x/x - addressing layer 1
			case NRPNH_LAYER2: // 22/x/x/x - addressing layer 2
			case NRPNH_LAYER3: // 23/x/x/x - addressing layer 3
			case NRPNH_LAYER4: // 24/x/x/x - addressing layer 4
				layer = nrpn_hi - NRPNH_LAYER1;
				switch(nrpn_lo) {
				case NRPNL_VOLTS: // x/15/0/1 .. x/15/0/8 - set reference voltage
					if(value > 0 && value < V_SEQ_OUT_CAL_MAX) {
						set(P_SEQ_OUT_CAL, value);
					}
					break;
				case NRPNL_CAL_SCALE: // x/98/sign/value - set the scale for the layer
					if(value >= CAL_SETTING_MIN && value <= CAL_SETTING_MAX) {
						m_layers[layer]->set(P_SQL_OUT_CAL_SCALE, value);
					}
					break;
				case NRPNL_CAL_OFFSET: // x/99/sign/value - set the offset for the layer
					if(value >= CAL_SETTING_MIN && value <= CAL_SETTING_MAX) {
						m_layers[layer]->set(P_SQL_OUT_CAL_OFFSET, value);
					}
					break;
				}

				// ensure menu is repainted to update new values
				// should calibration happen to be visible..
				fire_event(EV_REPAINT_MENU,0);
			}
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////////
	// Called once each millisecond, this is the entry point of the sequencing engine
	void run() {

		// ensure the sequencer is running
		if(m_is_running) {

			///////////////////////////////////////////////////////////////////////////////
			// 1 - CHECK THE SCHEDULING OF EACH LAYER TO SEE IF ANY LAYER HAS
			// MOVED TO A NEW STEP
			///////////////////////////////////////////////////////////////////////////////

			byte layer_update[NUM_LAYERS] = {0}; 			// whether individual layer has updated
			int any_layer_updated = 0;						// whether any layer has updated
			int dice_roll = rand()%16;		 			// random value for gate probability
			clock::TICKS_TYPE ticks = g_clock.get_ticks(); 	// get the current clock tick count
			for(int i=0; i<NUM_LAYERS; ++i) {
				int update;
				CSequenceStep step_value;
				CSequenceLayer& layer = *m_layers[i];
				if(m_rec_layer == i) {
					update = layer.play(ticks, dice_roll, &m_rec, step_value);
				}
				else {
					update = layer.play(ticks, dice_roll, NULL, step_value);
				}
				// If this is an ignore point (due to probability) then we
				// keep the same step value as before
				if(update && !step_value.is(CSequenceStep::IGNORE_POINT)) {
					m_step_value[i] = step_value;
					layer_update[i] = 1;
					any_layer_updated = 1;
				}
			}

			// is there anything to update?
			if(any_layer_updated) {

				///////////////////////////////////////////////////////////////////////////////
				// 2 - RECALCULATE THE CV OUTPUT VALUES FOR EACH LAYER EACH TIME ANY
				// LAYER IS UPDATED
				///////////////////////////////////////////////////////////////////////////////

				int any_accented_step = 0;	// we will track if any accent gate is active
				CV_TYPE output_value = 0; 	// used to pass output from one layer as input to the next
				for(int i=0; i<NUM_LAYERS; ++i) {
					CSequenceLayer& layer = *m_layers[i];

					// update output values based on current step vaues
					m_step_output[i] = layer.get_step_output(output_value, m_step_value[i]);
					output_value = m_step_output[i];

					// check if any mapped layer has an accent point (NB: need to establish this before any layer gates are
					// triggered, since accent triggers first)
					if(!layer.is_muted() && m_step_value[layer.get_gate_source_layer()].is(CSequenceStep::ACCENT_POINT)) {
						any_accented_step = 1;
					}
				}

				// set accent gate if required
				g_clock.set_accent(any_accented_step);

				///////////////////////////////////////////////////////////////////////////////
				// 3 - UPDATE THE OUTPUTS ANALOG OUTPUTS AND MIDI OUTPUTS FOR EACH LAYER.
				// THESE MIGHT ACTUALLY BE BE TAKING INPUT FROM OTHER LAYERS
				///////////////////////////////////////////////////////////////////////////////
				for(int i=0; i<NUM_LAYERS; ++i) {

					// we only need to do this for layers that are not muted
					CSequenceLayer& layer = *m_layers[i];
					if(!layer.is_muted()) {

						// Lookup the CV and gate value that should be output from this layer
						CSequenceStep& step_value = m_step_value[layer.get_gate_source_layer()];
						CV_TYPE step_output = m_step_output[layer.get_cv_source_layer()];

						// transpose and quantize to get the layer output from step output
						CV_TYPE layer_output = layer.get_layer_output(step_output, step_value);

						if(m_cal_mode == V_SEQ_OUT_CAL_NONE) {
							// Update the analog CV output
							layer.process_cv(layer_output, step_value);

						}

						// Update the MIDI CC output if needed
						if(V_SQL_MIDI_OUT_CC == layer.get_midi_out_mode()) {
							layer.process_midi_cc(layer_output);
						}

						// Check if there is a change to the output on the gate source layer
						if(layer_update[layer.get_gate_source_layer()]) {

							// Update the analog gate output
							layer.process_gate(step_value);

							// Update MIDI note if appropriate
							if(V_SQL_MIDI_OUT_NOTE == layer.get_midi_out_mode()) {
								layer.process_midi_note(layer_output, step_value);
							}
						}
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

	/////////////////////////////////////////////////////////////////////////////////////////////
	byte save_patch(int slot) {
		byte *ptr = g_i2c_eeprom.buf();
		int len = get_cfg_size();
		*ptr++ = PATCH_DATA_COOKIE1;
		*ptr++ = PATCH_DATA_COOKIE2;
		get_cfg(&ptr);
		*ptr++ = g_i2c_eeprom.buf_checksum(len + 2);
		return g_i2c_eeprom.write(slot, len + 3);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////
	void save_patch_complete(int slot) {
		g_popup.text("OK");
	}
	/////////////////////////////////////////////////////////////////////////////////////////////
	byte load_patch(int slot) {
		return g_i2c_eeprom.read(slot, get_cfg_size() + 3);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////
	void load_patch_complete(int slot) {
		byte *buf = g_i2c_eeprom.buf();
		int len = get_cfg_size();
		byte checksum = g_i2c_eeprom.buf_checksum(len + 2);
		if(buf[0] == PATCH_DATA_COOKIE1 && buf[1] == PATCH_DATA_COOKIE2 && buf[len+2] == checksum) {
			buf+=2;
			set_cfg(&buf);
			init_state();
			switch(slot) {
			case SLOT_CONFIG:
				break;
			case SLOT_TEMPLATE:
				clear();
				g_popup.text("INIT");
				break;
			default:
				int i;
				for(i=0; i<NUM_LAYERS; ++i) {
					if(m_layers[i]->is_muted())
						break;
				}
				g_popup.text(i<NUM_LAYERS? "OK MUTES": "OK");
				break;
			}

			// build the scale mappings
			m_scale.build();
		}
		else {
			switch(slot) {
			case SLOT_CONFIG:
				break;
			case SLOT_TEMPLATE:
				init();
				g_popup.text("INIT");
				break;
			default:
				g_popup.text("EMPTY");
				break;
			}
		}
		fire_event(EV_CLOCK_RESET,0);
	}

};

CSequence g_sequence;

#endif /* SEQUENCE_H_ */
