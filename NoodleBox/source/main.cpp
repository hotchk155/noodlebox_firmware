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
///////////////////////////////////////////////////////////////////////////////////

//
// API INCLUDES
//
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MKE04Z1284.h"
#include "fsl_clock.h"
#include "fsl_spi.h"
#include "fsl_pit.h"
#include "fsl_common.h"
#include "fsl_kbi.h"
#include "fsl_gpio.h"
#include "fsl_i2c.h"
#include "fsl_uart.h"


//
// APPLICATION INCLUDES
//
#include "defs.h"
#include "digital_out.h"
#include "chars.h"
#include "ui_driver.h"
#include "leds.h"
#include "midi.h"
#include "clock.h"
#include "i2c_bus.h"
#include "popup.h"
#include "scale.h"
#include "outs.h"
#include "sequence_step.h"
#include "sequence_page.h"
#include "sequence_layer.h"
#include "sequence.h"
#include "sequence_editor.h"
#include "params.h"
#include "menu.h"


const uint32_t title_screen[] = {
		(uint32_t)0xFFFFFFFF,
		(uint32_t)0xFFFFFFFF,
		(uint32_t)0xFFFFFFFF,
		(uint32_t)0xFFFFFFFF,
		(uint32_t)0xFFFFFFFF,
		(uint32_t)0xFFFFFFFF,
		(uint32_t)0xFFFFFFFF,
		(uint32_t)0xFFFFFFFF,
		(uint32_t)0xFFFFFFFF,
		(uint32_t)0xFFFFFFFF,
		(uint32_t)0xFFFFFFFF,
		(uint32_t)0xFFFFFFFF,
		(uint32_t)0xFFFFFFFF,
		(uint32_t)0xFFFFFFFF,
		(uint32_t)0xFFFFFFFF,
		(uint32_t)0xFFFFFFFF
};


#if NB_PROTOTYPE
 CDigitalOut<kGPIO_PORTE, 2> PowerControl;
 CDigitalIn<kGPIO_PORTE, 1> OffSwitch;
#else
 CDigitalOut<kGPIO_PORTE, 2> PowerControl;
 CDigitalIn<kGPIO_PORTE, 7> OffSwitch;
#endif


typedef enum:byte {
	 VIEW_SEQUENCER,
	 VIEW_MENU_A,
	 VIEW_MENU_B
 } VIEW_TYPE;

VIEW_TYPE g_view = VIEW_SEQUENCER;

/////////////////////////////////////////////////////////////////////////////////////////////
void midi::handle_note(byte chan, byte note, byte vel) {
	g_sequence_editor.handle_midi_note(chan, note, vel);
}

/////////////////////////////////////////////////////////////////////////////////////////////
void midi::handle_realtime(byte ch) {
	clock::g_midi_clock_in.on_midi_realtime(ch, g_clock.get_ms());
}


/////////////////////////////////////////////////////////////////////////////////////////////////
void set(PARAM_ID param, int value) {
	if(param < P_SQL_MAX) {
		g_sequence_editor.set(param,value);
	}
	else if(param < P_SEQ_MAX) {
		g_sequence.set(param,value);
	}
	else if(param < P_CLOCK_MAX) {
		g_clock.set(param,value);
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////
int get(PARAM_ID param) {
	if(param < P_SQL_MAX) {
		return g_sequence_editor.get(param);
	}
	else if(param < P_SEQ_MAX) {
		return g_sequence.get(param);
	}
	else if(param < P_CLOCK_MAX) {
		return g_clock.get(param);
	}
	return 0;
}
/////////////////////////////////////////////////////////////////////////////////////////////////
int is_valid_for_menu(PARAM_ID param) {
	if(param < P_SQL_MAX) {
		return g_sequence_editor.is_valid_param(param);
	}
	else if(param < P_SEQ_MAX) {
		return g_sequence.is_valid_param(param);
	}
	else if(param < P_CLOCK_MAX) {
		return g_clock.is_valid_param(param);
	}
	return 0;
}

void dispatch_event(int event, uint32_t param) {
	switch(g_view) {
	case VIEW_SEQUENCER:
		g_sequence_editor.event(event, param);
		break;
	case VIEW_MENU_A:
	case VIEW_MENU_B:
		g_menu.event(event, param);
		break;
	}
}

void fire_event(int event, uint32_t param) {

	switch(event) {
	///////////////////////////////////
	case EV_KEY_PRESS:
		switch(param) {
			case KEY_RUN:
				fire_event(g_sequence.is_running()? EV_SEQ_STOP : EV_SEQ_CONTINUE, 0);
				break;
			case KEY_CV|KEY_RUN:
				fire_event(EV_SEQ_RESTART, 0);
				break;
			default:
				dispatch_event(event, param);
				break;
		}
		break;
	///////////////////////////////////
	case EV_KEY_CLICK:
		switch(param) {
		case KEY_LAYER:
			switch(g_view) {
			case VIEW_MENU_A:
			case VIEW_MENU_B:
				g_view = VIEW_SEQUENCER;
				g_sequence_editor.activate();
				break;
			case VIEW_SEQUENCER:
			default:
				g_view = VIEW_MENU_A;
				g_menu.activate(CMenu::MENU_A);
				break;
			}
			break;
		case KEY_LAYER|KEY_FUNC:
			switch(g_view) {
			case VIEW_MENU_B:
				g_view = VIEW_MENU_A;
				g_menu.activate(CMenu::MENU_A);
				break;
			case VIEW_MENU_A:
			case VIEW_SEQUENCER:
			default:
				g_view = VIEW_MENU_B;
				g_menu.activate(CMenu::MENU_B);
				break;
			}
			break;
		default:
			dispatch_event(event, param);
			break;
		}
		break;
	///////////////////////////////////
	case EV_REPAINT_MENU:
		g_menu.event(event,param);
		break;
	///////////////////////////////////
	case EV_CHANGE_LAYER:
		g_sequence_editor.event(event, param);
		break;
	///////////////////////////////////
	case EV_SAVE_FAIL:
	case EV_LOAD_FAIL:
		g_popup.text("M.ERR");
		break;
	case EV_SEQ_STOP:
	case EV_SEQ_RESTART:
	case EV_SEQ_CONTINUE:
	case EV_CLOCK_RESET:
	case EV_REAPPLY_CONFIG:
	case EV_SAVE_OK:
	case EV_LOAD_OK:
		g_clock.event(event, param);
		g_outs.event(event, param);
		g_sequence.event(event, param);
		g_sequence_editor.event(event, param);
		break;
	///////////////////////////////////
	default:
		dispatch_event(event, param);
		break;
	}
}


void force_full_repaint() {
	g_popup.force_repaint();
	g_menu.force_repaint();
}

void test() {
	int dac = 0;
	int gate = 0;
	int gate_tmr = 0;
	while(1) {
    	if(g_clock.is_ms_tick()) {
    		g_outs.test_dac(2,  dac);
    		g_outs.test_dac(3,  dac);
    		if(++dac > 4095) {
    			dac = 0;
    		}
    		if(++gate_tmr > 50) {
    			gate_tmr = 0;
    			gate = !gate;
    			g_outs.gate(3, gate? COuts::GATE_OPEN: COuts::GATE_CLOSED);

    		}

    	}

		//g_outs.run_i2c();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
void save_config() {
	byte *ptr = g_i2c_eeprom.buf();
	int len = g_outs.get_cfg_size() + g_clock.get_cfg_size() + g_sequence_editor.get_cfg_size();
	*ptr++ = CONFIG_DATA_COOKIE1;
	*ptr++ = CONFIG_DATA_COOKIE2;
	g_outs.get_cfg(&ptr);
	g_clock.get_cfg(&ptr);
	g_sequence_editor.get_cfg(&ptr);
	byte checksum = g_i2c_eeprom.buf_checksum(len + 2);
	*ptr++ = checksum;
	g_i2c_eeprom.write(SLOT_CONFIG, len + 3);
	g_i2c_bus.wait_for_idle();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void load_config() {
	byte *buf = g_i2c_eeprom.buf();
	int len = g_outs.get_cfg_size() + g_clock.get_cfg_size();
	g_i2c_eeprom.read(SLOT_CONFIG, len + 3);
	g_i2c_bus.wait_for_idle();
	byte checksum = g_i2c_eeprom.buf_checksum(len + 2);
	if(buf[0] == CONFIG_DATA_COOKIE1 && buf[1] == CONFIG_DATA_COOKIE2 && buf[len+2] == checksum) {
		buf+=2;
		g_outs.set_cfg(&buf);
		g_clock.set_cfg(&buf);
		g_sequence_editor.set_cfg(&buf);
		fire_event(EV_REAPPLY_CONFIG, 0);
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////
int main(void) {
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();

    g_clock.init();
    g_ui.init();
    for(int i=15; i>=0; --i) {
        g_ui.lock_for_update();
        for(int j=0; j<16; ++j) {
        	g_ui.raster(j) = title_screen[j];
        	if(j>=i) {
        		g_ui.hilite(j) = title_screen[j];
        	}
        	else {
        		g_ui.hilite(j) = 0;
        	}
        }
        g_ui.unlock_for_update();
        g_clock.wait_ms(50);
    }
    PowerControl.set(1);
    g_i2c_bus.init();
    g_midi.init();
    g_sequence.init();
    load_config();
    if(!g_ui.is_key_down(KEY_CV)) {
        g_sequence.load_patch(SLOT_AUTOSAVE);
    }
    g_i2c_bus.wait_for_idle();

    g_sequence_editor.activate();

    while(1) {

    	if(g_clock.is_ms_tick()) {

        	g_clock.run();
       		g_sequence.run();
        	g_outs.run();
        	g_midi.run();

       		g_sequence_editor.run();
        	g_ui.run();
    		g_popup.run();

			g_ui.lock_for_update();
			switch(g_view) {
			case VIEW_SEQUENCER:
				g_sequence_editor.repaint();
				break;
			case VIEW_MENU_A:
			case VIEW_MENU_B:
				g_menu.repaint();
				break;
			}
			g_popup.repaint();
			g_ui.unlock_for_update();

    		if(!OffSwitch.get()) {
    			break;
    		}

    		g_midi_led.run();
    		g_gate_led.run();
    		g_tempo_led.run();
    	}

    	// run the i2c bus.
    	g_i2c_bus.run();
    }
    save_config();
    g_sequence.save_patch(SLOT_AUTOSAVE);
    g_i2c_bus.wait_for_idle();
	PowerControl.set(0);
	for(;;);
    return 0 ;
}
