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
// POPUP WINDOW
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
#ifndef POPUP_H_
#define POPUP_H_

//////////////////////////////////////////////////////////////////////////////
class CPopup {

	/////////////////////////////////////////////////////////////////////////
	void format_number(int value, int msd) {
		if(value<0) {
			m_text[m_len++] = '-';
			value = -value;
		}
		//value%=msd;
		while(msd>0) {
			m_text[m_len++] = '0' + value/msd;
			value%=msd;
			msd/=10;
		}
	}

public:
	enum {
		MAX_TEXT = 8,
		ALIGN_LEFT = 1,
		ALIGN_RIGHT = 2,
		ALIGN_CENTRE = 3,
		DISPLAY_TIMEOUT = 1000
	};
	uint32_t m_mask;
	uint32_t m_raster[5];
	char m_text[MAX_TEXT];
	byte m_len;
	byte m_row;
	byte m_align;
	byte m_render;
	int m_timeout;

	/////////////////////////////////////////////////////////////////////////
	CPopup() {
		m_len = 0;
		m_align = ALIGN_RIGHT;
		m_timeout = 0;
		m_row = 2;
		m_render = 0;
		memset(m_raster,0,sizeof m_raster);
		m_mask = 0;
	}

	/////////////////////////////////////////////////////////////////////////
	// prepare bitmap
	void prepare() {
		for(int i=0; i<5; ++i) {
			m_raster[i] = 0;
		}
		m_mask = 0;
		if(m_len) {
			// calculate starting column
			int col;
			switch(m_align) {
			case ALIGN_CENTRE:
				col = 32-2*m_len;
				break;
			case ALIGN_RIGHT:
				col = 32-4*m_len;
				break;
			case ALIGN_LEFT:
			default:
				col = 0;
			}
			m_mask = g_ui.make_mask(col, col + 4 * m_len - 1);
			for(int i=0; i<m_len; ++i) {
				g_ui.print_char(m_text[i], col, 0, m_raster, NULL, 5);
				col += 4;
			}
		}
	}

	/////////////////////////////////////////////////////////////////////////
	void align(int mode) {
		if(mode != m_align) {
			m_align = mode;
			m_render = 1;
		}
	}

	/////////////////////////////////////////////////////////////////////////
	// automatically set left or right alignment based
	void avoid(int col) {
		byte prev = m_align;
		if(col < 16 && m_align != ALIGN_RIGHT) {
			m_align = ALIGN_RIGHT;
		}
		else if(col >= 16 && m_align != ALIGN_LEFT) {
			m_align = ALIGN_LEFT;
		}

		// re-render if alignment changed
		if (m_len > 0 && prev != m_align) {
			m_render = 1;
		}
	}

	/////////////////////////////////////////////////////////////////////////
	// for popup showing note name
	void note_name(byte note) {
		m_len = 1;
		m_text[1] = '#';
		switch(note%12) {
		//
		case 1:
			m_len = 2;
		case 0:
			m_text[0] = 'C';
			break;
		//
		case 3:
			m_len = 2;
		case 2:
			m_text[0] = 'D';
			break;
		//
		case 4:
			m_text[0] = 'E';
			break;
		//
		case 6:
			m_len = 2;
		case 5:
			m_text[0] = 'F';
			break;
		//
		case 8:
			m_len = 2;
		case 7:
			m_text[0] = 'G';
			break;
		//
		case 10:
			m_len = 2;
		case 9:
			m_text[0] = 'A';
			break;
		//
		case 11:
			m_text[0] = 'B';
			break;
		}

		// octave
		format_number((note/12)+(MIDDLE_C_OCTAVE-5), 1);

		m_render = 1;
		m_align = ALIGN_RIGHT;
		m_timeout = DISPLAY_TIMEOUT;
	}

	/////////////////////////////////////////////////////////////////////////
	void num1digit(int value, byte append=0) {
		if(!append) {
			m_len = 0;
		}
		format_number(value, 1);
		m_render = 1;
		m_align = ALIGN_RIGHT;
		m_timeout = DISPLAY_TIMEOUT;
	}

	/////////////////////////////////////////////////////////////////////////
	void num2digits(int value, byte append=0) {
		if(!append) {
			m_len = 0;
		}
		format_number(value, 10);
		m_render = 1;
		m_align = ALIGN_RIGHT;
		m_timeout = DISPLAY_TIMEOUT;
	}

	/////////////////////////////////////////////////////////////////////////
	void num3digits(int value, byte append=0) {
		if(!append) {
			m_len = 0;
		}
		format_number(value, 100);
		m_render = 1;
		m_align = ALIGN_RIGHT;
		m_timeout = DISPLAY_TIMEOUT;
	}

	/////////////////////////////////////////////////////////////////////////
	void show_offset(int value) {
		m_len = 0;
		if(value >= 0) {
			m_text[m_len++] = '+';
		}
		if(value<-99 || value>99) {
			format_number(value, 100);
		}
		else {
			format_number(value, 10);
		}
		m_render = 1;
		m_align = ALIGN_RIGHT;
		m_timeout = DISPLAY_TIMEOUT;
	}

	/////////////////////////////////////////////////////////////////////////
	void text(const char *sz, byte append = 0) {
		if(!append) {
			m_len = 0;
		}
		while(*sz) {
			m_text[m_len++] = *sz++;
		}
		m_render = 1;
		m_align = ALIGN_RIGHT;
		m_timeout = DISPLAY_TIMEOUT;
	}

	/////////////////////////////////////////////////////////////////////////
	void text_value(const char *values, int which, byte append = 0) {
		const char *ch = values;
		while(*ch) {
			char this_value[10];
			int len = 0;
			while(*ch && *ch != '|' && len<8) {
				this_value[len++] = *ch++;
			}
			this_value[len] = '\0';
			if(!which) {
				text(this_value, append);
				break;
			}
			if(!*ch) {
				break;
			}
			++ch;
			--which;
		}
	}

	/////////////////////////////////////////////////////////////////////////
	void repaint() {
		if(m_render) {
			prepare();
			m_render = 0;
		}

		int row = m_row;
		for(int i=0; i<5; ++i) {
			g_ui.raster(row) &= ~m_mask;
			g_ui.raster(row) |= m_raster[i];
			g_ui.hilite(row) |= m_mask;
			++row;
		}
	}

	/////////////////////////////////////////////////////////////////////////
	void no_hide() {
		m_timeout = 0;
	}

	/////////////////////////////////////////////////////////////////////////
	void hide_after_timeout() {
		m_timeout = DISPLAY_TIMEOUT;
	}

	/////////////////////////////////////////////////////////////////////////
	void hide() {
		m_len = 0;
		force_full_repaint();
	}

	/////////////////////////////////////////////////////////////////////////
	void force_repaint() {
		m_render = 1;
	}

	/////////////////////////////////////////////////////////////////////////
	void run() {
		if(m_timeout) {
			if(!--m_timeout) {
				hide();
			}
		}
	}
};

CPopup g_popup;

#endif /* POPUP_H_ */
