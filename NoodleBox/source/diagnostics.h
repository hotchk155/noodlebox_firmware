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
//  CHARACTER SET DEFINITIONS                                               //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
#ifndef DIAGNOSTICS_H_
#define DIAGNOSTICS_H_

class CDiagnostics {
	void show_text(uint32_t *raster, const char *text, unsigned int size) {
		memset(raster,0,5*sizeof(uint32_t));
		int col = 0;
		for(uint32_t i=0; i<size; ++i) {
			g_ui.print_char(text[i], col, 0, raster, NULL, 5);
			col += 4;
		}
	}

	int memory_test() {
		g_i2c_bus.wait_for_idle();
		for(int slot=0; slot<NUM_SLOTS; ++slot) {
			int seed = (byte)(slot*3);
			for(int i=0; i<g_i2c_eeprom.buf_size(); ++i) {
				g_i2c_eeprom.buf()[i] = (byte)(seed+i);
			}
			g_i2c_eeprom.write(slot, g_i2c_eeprom.buf_size());
			g_i2c_bus.wait_for_idle();
		}

		for(int slot=0; slot<NUM_SLOTS; ++slot) {
			int seed = (byte)(slot*3);
			g_i2c_eeprom.read(slot, g_i2c_eeprom.buf_size());
			g_i2c_bus.wait_for_idle();
			for(int i=0; i<g_i2c_eeprom.buf_size(); ++i) {
				if(g_i2c_eeprom.buf()[i] != (byte)(seed+i)) {
					return 0;
				}
			}
		}

		return 1;
	}

public:

	void run() {
		extern CDigitalIn OffSwitch;

		int count = 0;
		int enc_pos = 0;

		uint32_t text_raster[5];
		byte this_keys[8];
		byte last_keys[8] = {0};

		show_text(text_raster,"DIAG",4);
		while(1) {
			if(g_clock.is_ms_tick()) {

				int mem_test = 0;
				memset(this_keys,0,sizeof(this_keys));

				uint32_t mask1 = 0;
				if(g_ui.is_key_down(KEY_B1)) {
					mask1 |= g_ui.make_mask(0, 3);
					this_keys[0] = 1;
					mem_test++;
				}
				if(g_ui.is_key_down(KEY_B2)) {
					mask1 |= g_ui.make_mask(4, 7);
					this_keys[1] = 1;
					mem_test++;
				}
				if(g_ui.is_key_down(KEY_B3)) {
					mask1 |= g_ui.make_mask(8, 11);
					this_keys[2] = 1;
					mem_test++;
				}
				if(g_ui.is_key_down(KEY_B4)) {
					mask1 |= g_ui.make_mask(12, 15);
					this_keys[3] = 1;
					mem_test++;
				}
				if(g_ui.is_key_down(KEY_B5)) {
					mask1 |= g_ui.make_mask(16, 19);
					this_keys[4] = 1;
					mem_test = 0;
				}
				if(g_ui.is_key_down(KEY_B6)) {
					mask1 |= g_ui.make_mask(20, 23);
					this_keys[5] = 1;
					mem_test = 0;
				}
				if(g_ui.is_key_down(KEY_B7)) {
					mask1 |= g_ui.make_mask(24, 27);
					this_keys[6] = 1;
					mem_test = 0;
				}
				if(g_ui.is_key_down(KEY_B8)) {
					mask1 |= g_ui.make_mask(28, 31);
					this_keys[7] = 1;
					mem_test = 0;
				}

				for(int i=0; i<8; ++i) {
					if(this_keys[i] != last_keys[i]) {
						if(this_keys[i]) {
							g_midi.start_note(0, 36+i, 127);
						}
						else {
							g_midi.stop_note(0, 36+i);
						}
						last_keys[i] = this_keys[i];
					}
				}


				byte is_test_pattern = 0;
				uint32_t test_pattern0 = 0;
				uint32_t test_pattern1 = 0;
				uint32_t test_pattern2 = 0;
				if(g_ui.is_key_down(KEY_R1)) {
					test_pattern1 |= 0xAAAAAAAA;
					test_pattern2 |= 0x55555555;
					is_test_pattern = 1;
					mem_test++;
				}
				if(g_ui.is_key_down(KEY_R2)) {
					test_pattern1 |= 0x55555555;
					test_pattern2 |= 0xAAAAAAAA;
					is_test_pattern = 1;
					mem_test = 0;
				}
				if(g_ui.is_key_down(KEY_R3)) {
					test_pattern0 = 0xFFFFFFFF;
					is_test_pattern = 1;
					mem_test = 0;
				}
				if(mem_test<5) {
					mem_test = 0;
				}


				uint32_t mask3 = 0;
				if(!g_clock_in.get()) {
					mask3 |= g_ui.make_mask(20, 23);
				}
#ifndef NB_PROTOTYPE
				if(!g_aux_in.get()) {
					mask3 |= g_ui.make_mask(24, 27);
				}
#endif
				if(!OffSwitch.get()) {
					mask3 |= g_ui.make_mask(28, 31);
				}

				enc_pos += g_ui.get_enc_movement();
				while(enc_pos < 0) {
					enc_pos += 32;
				}
				while(enc_pos > 31) {
					enc_pos -= 32;
				}
				uint32_t mask4 = g_ui.make_mask(enc_pos, enc_pos + 1);;

				if(mem_test) {
					show_text(text_raster,"MEMO",4);
					is_test_pattern = 0;
				}

				g_ui.lock_for_update();
				g_ui.clear();
				if(is_test_pattern) {
					for(int i=0; i<16; i+=2) {
						g_ui.raster(i) = test_pattern0;
						g_ui.raster(i+1) = test_pattern0;
						g_ui.hilite(i) = test_pattern1;
						g_ui.hilite(i+1) = test_pattern2;
					}
				}
				else {
					g_ui.raster(0) = text_raster[0];
					g_ui.raster(1) = text_raster[1];
					g_ui.raster(2) = text_raster[2];
					g_ui.raster(3) = text_raster[3];
					g_ui.raster(4) = text_raster[4];

					g_ui.raster(0) |= mask3;
					g_ui.raster(1) |= mask3;
					g_ui.raster(14) |= mask1;
					g_ui.raster(15) |= mask1;

					for(int i=0; i<16; ++i) {
						g_ui.hilite(i) |= mask4;
					}
				}
				g_ui.unlock_for_update();

				if(mem_test) {
					if(memory_test()) {
						show_text(text_raster,"PASS",4);
					}
					else {
						show_text(text_raster,"FAIL",4);
					}
				}

				//g_clock.run();
				//g_outs.run();
				g_midi.run();
				g_ui.run();

				g_tempo_led.set(!!((count & 0x300) == 0x000));
				g_midi_led.set(!!((count & 0x300) == 0x100));
				g_gate_led.set(!!((count & 0x300) == 0x200));

				g_clock_out.set(!!((count & 0x300) == 0x000));
#ifndef NB_PROTOTYPE
				g_aux_out.set(!!((count & 0x300) == 0x100));
#endif

				g_outs.impl_set_gate(0, !!((count & 0x300) == 0x000));
				g_outs.impl_set_gate(1, !!((count & 0x300) == 0x100));
				g_outs.impl_set_gate(2, !!((count & 0x300) == 0x200));
				g_outs.impl_set_gate(3, !!((count & 0x300) == 0x300));

				g_outs.impl_set_cv(0, ((count<<4) + 0x000) & 0xFFF);
				g_outs.impl_set_cv(1, ((count<<4) + 0x400) & 0xFFF);
				g_outs.impl_set_cv(2, ((count<<4) + 0x800) & 0xFFF);
				g_outs.impl_set_cv(3, ((count<<4) + 0xC00) & 0xFFF);
				++count;

			}
			// run the i2c bus.
			g_i2c_bus.run();
		}
	}
};
CDiagnostics g_diagnostics;


#endif /* DIAGNOSTICS_H_ */
