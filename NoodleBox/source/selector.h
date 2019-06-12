/*
 * selector.h
 *
 *  Created on: 2 Apr 2018
 *      Author: jason
 */

#ifndef SELECTOR_H_
#define SELECTOR_H_



/////////////////////////////////////////////////////////////////////////////////////////////////
class CSelector {
public:
	byte m_value;
	byte m_repaint;
	const char *m_prompt;
	PARAM_ID m_param;
	PARAM_TYPE m_type;
	const char *m_values_text;

	/////////////////////////////////////////////////////////////////////////////////////////////////
	CSelector() {
		m_value = 0;
		m_repaint = 0;
		m_param = P_NONE;
		m_type = PT_NONE;
		m_values_text = NULL;
		m_prompt = NULL;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////
	void activate(const char *prompt, PARAM_ID param, PARAM_TYPE type, const char *values_text) {
		m_prompt = prompt;
		m_param = param;
		m_type = type;
		m_values_text = values_text;
		m_value = CParams::get(param);
		m_repaint = 1;
	}

	int m_evt;
	uint32_t m_evparam;
	/////////////////////////////////////////////////////////////////////////////////////////////////
	void event(int evt, uint32_t param) {
		m_evt = evt;
		m_evparam = param;

/*		int i;
		switch(evt) {
		case EV_ENCODER:
			i = m_value + (int)param;
			if(i >= param_min_value(m_type) && i <= param_max_value(m_type, m_values_text)) {
				m_value = i;
				m_repaint = 1;
			break;
			}
		}*/
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////
	void force_repaint() {
		m_repaint = 1;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////
	void repaint() {
		g_ui.clear();
		switch(m_evt) {
/*		case EV_ENCODER:
			i = m_value + (int)param;
			if(i >= param_min_value(m_type) && i <= param_max_value(m_type, m_values_text)) {
				m_value = i;
				m_repaint = 1;
			break;
			}*/


		case EV_KEY_PRESS: g_ui.print_text("PR", 0, 3, CUiDriver::RASTER); break;
		case EV_KEY_RELEASE: g_ui.print_text("RE", 0, 3, CUiDriver::RASTER); break;
		case EV_KEY_CLICK: g_ui.print_text("CK", 0, 3, CUiDriver::RASTER); break;
		case EV_KEY_HOLD: g_ui.print_text("HL", 0, 3, CUiDriver::RASTER); break;

		}

		g_ui.raster(15) = m_evparam;

		/*
		if(m_repaint) {
			g_ui.clear();
			g_ui.print_text(m_prompt, 0, 3, CUI::RASTER);
			g_ui.print_text(param_value_string(m_type, m_value, m_values_text), 0, 9, CUI::RASTER|CUI::HILITE);
			m_repaint = 0;
		}*/
	}
};

CSelector g_selector;


#endif /* SELECTOR_H_ */
