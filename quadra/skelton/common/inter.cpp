/* -*- Mode: C++; c-basic-offset: 2; tab-width: 2; indent-tabs-mode: nil -*-
 * Copyright (c) 1998-2000 Ludus Design enr.
 * All Rights Reserved.
 * Tous droits r�serv�s.
 */

#include "video.h"
#include "bitmap.h"
#include "input.h"
#include "main.h"
#include "sound.h"
#include "cursor.h"
#include "inter.h"
#include "res.h"

int Inter::last_mouse_x = -1, Inter::last_mouse_y = -1;
bool Inter::kb_visible = false;

Zone::Zone(Inter* in, int px, int py, int pw, int ph) {
	dirt();
	focusable = 0;
	kb_focusable = stay_on_top = false;
	child = NULL;
	parent = NULL;
	x = px;
	y = py;
	w = pw;
	h = ph;
	enabled = 0;
	inter = in;
	if(inter)
		inter->add(this);
}

Zone::~Zone() {
	if(inter)
		inter->remove(this);
}

void Zone::clicked(int quel) {
	if(child)
		child->clicked(quel);
}
void Zone::double_clicked() {
	if(child)
		child->double_clicked();
}
void Zone::entered() {
	if(child)
		child->entered();
}
void Zone::waiting() {
	if(child)
		child->waiting();
}
void Zone::leaved() {
	if(child)
		child->leaved();
}
void Zone::set_child(Zone* chil) {
	child = chil;
	child->parent = this;
	child->kb_focusable = false; // empeche de focuser un 'child'
}

int Zone::in() const {
	if(!cursor)
		return 0;
	if(cursor->x >= x && cursor->x < (x+w) &&
		 cursor->y >= y && cursor->y < (y+h))
		 return 1;
	else
		return 0;
}

Zone_sprite::Zone_sprite(Inter *in, const char *nam, int px, int py): Zone(in) {
	Res_doze res(nam);
	Raw raw(res);
	Bitmap bitmap(raw);
	sp = new Sprite(bitmap, 0, 0);
	w = sp->width;
	h = sp->height;
	if(px == -1)
		x = (video->width - w) / 2;
	if(py == -1)
		y = (video->height - h) / 2;
	w = h = 0; // special: zone_sprite pas clickable
	stay_on_top = true; // special: les zone_sprite se reblittent constamment.
}

Zone_sprite::~Zone_sprite() {
	delete sp;
}

void Zone_sprite::draw() {
	sp->draw(video->vb, x, y);
}

Zone_bitmap::Zone_bitmap(Inter* in, Bitmap* bit, int px, int py, Bitmap* bit2):
Zone(in, px, py, bit->width, bit->height) {
	bit_ = bit;
	bit2_ = bit2;
	actual = bit;
	del_bit = false;
}

Zone_bitmap::Zone_bitmap(Inter* in, Bitmap* bit, int px, int py, bool del):
Zone(in, px, py, bit->width, bit->height) {
	bit_ = bit;
	bit2_ = NULL;
	actual = bit;
	del_bit = del;
}

Zone_bitmap::~Zone_bitmap() {
	if(del_bit)
		delete bit_;
}

Zone_state::Zone_state(Inter* in, int *pval, int px, int py, int pw, int ph, int pnstate):
Zone_watch_int(in, pval, px, py, pw, ph) {
	nstate = pnstate;
	kb_focusable = true;
}

void Zone_watch_int::set_val(int *pv) {
	val = pv;
	last_val = -9999;
	process();
}

void Zone_watch_int::process() {
	if(val && last_val != *val) {
		dirt();
		last_val = *val;
	}
}

void Zone_state::clicked(int quel) {
	if(quel==0) {
		*val = (*val) + 1;
		if(*val > (nstate-1))
			*val = 0;
	}
	else {
		*val = (*val) - 1;
		if(*val < 0)
			*val = (nstate-1);
	}
	process();
	Zone_watch_int::clicked(quel);
}

Zone_state_bit::Zone_state_bit(Inter* in, const char* b1, int *pval, int px, int py, const char* b2, const char* b3):
	Zone_state(in, pval, px, py) {
	Res_doze res(b1);
	Raw raw(res);
	state[0] = new Bitmap(raw);
	w = state[0]->width;
	h = state[0]->height;
	if(b2) {
		Res_doze res2(b2);
		Raw raw(res2);
		state[nstate++] = new Bitmap(raw);
	}
	if(b3) {
		Res_doze res3(b3);
		Raw raw(res3);
		state[nstate++] = new Bitmap(raw);
	}
}

Zone_state_text::Zone_state_text(Inter* in, int *pval, int px, int py, int pw, int ph):
	Zone_state(in, pval, px, py, pw, ph, 0) {
	pan = new Zone_panel(NULL, x, y, w, h);
}

void Zone_state_text::add_string(const char* s, Font *f) {
	if(!f)
		f = inter->font;
	fonts[nstate] = f;
	state[nstate++] = s;
	int w2 = f->width(s) + 6;
	if(w2 > w) {
		w = w2;
		pan->w = w;
		dirt();
		pan->resize();
	}
}

void Zone_state_text::draw() {
	pan->draw();
	video->vb->vline(x, y, h, 255);
	video->vb->hline(y, x, w, 255);
	fonts[last_val]->draw(state[last_val], pan->pan, CENTER, 0);
}

void Zone_state_text::leaved() {
	Zone_state::leaved();
	pan->high = false;
	pan->dirt();
	dirt();
}

void Zone_state_text::entered() {
	Zone_state::entered();
	pan->high = true;
	pan->dirt();
	dirt();
}

Zone_text::Zone_text(Inter* in, const char* s, int px, int py):
Zone(in, px, py, in->font->width(s), in->font->height()) {
	font = in->font;
	lock_size = false;
	set_text(s);
}

Zone_text::Zone_text(Font* f2, Inter* in, const char* s, int px, int py):
Zone(in, px, py, in->font->width(s)+2, f2->height()) {
	font = f2;
	lock_size = false;
	set_text(s);
}

Zone_text::Zone_text(Font* f2, Inter* in, const char* s, int py):
Zone(in, 0, py, video->width, f2->height()) {
	font = f2;
	lock_size = true;
	set_text(s);
}

Zone_text::Zone_text(Inter* in, const char* s, int px, int py, int pw):
Zone(in, px, py, pw, in->font->height()) {
	font = in->font;
	lock_size = true;
	set_text(s);
}

Zone_text::Zone_text(Inter* in, const char* s, int py):
Zone(in, 0, py, video->width, in->font->height()) {
	font = in->font;
	lock_size = true;
	set_text(s);
}

void Zone_text::set_font(Font* f) {
	font = f;
	dirt();
}

void Zone_text::set_text(const char* s) {
	strncpy(st, s, sizeof(st));
	st[sizeof(st)-1]=0;
	if(!lock_size) {
		w = font->width(s);
		h = font->height();
		text_x = x;
	} else
		text_x = (w-font->width(s))/2 + x;
	dirt();
}

void Zone_text::draw() {
	font->draw(st, video->vb, text_x, y);
}

Zone_text_select::Zone_text_select(Inter* in, Font* f, const char* s, int px, int py):
Zone_text(in, s, px, py) {
	font2 = f;
	actual = in->font;
	kb_focusable = true;
}

Zone_text_select::Zone_text_select(Inter* in, Font* f, const char* s, int px, int py, int pw):
Zone_text(in, s, px, py, pw) {
	font2 = f;
	actual = in->font;
	kb_focusable = true;
}

void Zone_text_select::draw() {
	video->vb->hline(y, x, w, 210);
	video->vb->hline(y+h-1, x, w, 210);
	video->vb->vline(x, y, h, 210);
	video->vb->vline(x+w-1, y, h, 210);
	actual->draw(st, video->vb, text_x, y);
}

void Zone_text_select::leaved() {
	actual = font;
	dirt();
	Zone_text::leaved();
}

void Zone_text_select::entered() {
	actual = font2;
	dirt();
	Zone_text::entered();
}

void Zone_text_select::set_font(Font* f) {
	if(actual == font)
		actual = f;
	Zone_text::set_font(f);
}

Zone_text_button::Zone_text_button(Inter* in, Bitmap *fond, Font* f, const char* s, int py):
Zone_text_select(in, f, s, (video->width - f->width(s))>>1, py-2, f->width(s)) {
	h+=4;
	w+=6;
	set_bit(fond);
	high = false;
	kb_focusable = true;
}

Zone_text_button::Zone_text_button(Inter* in, Bitmap *fond, Font* f, const char* s, int px, int py):
Zone_text_select(in, f, s, px-3, py-2) {
	h+=4;
	w+=6;
	set_bit(fond);
	high = false;
	kb_focusable = true;
}

Zone_text_button::Zone_text_button(Inter* in, Bitmap *fond, Font* f, const char* s, int px, int py, int pw):
Zone_text_select(in, f, s, px-3, py-2, pw+6) {
	h+=4;
	set_bit(fond);
	high = false;
	kb_focusable = true;
}

Zone_text_button::~Zone_text_button() {
	if(bit)
		delete bit;
}

void Zone_text_button::set_text(const char* s) {
	Zone_text_select::set_text(s);
	if(!lock_size) {
		h+=4;
		w+=6;
	}
}

void Zone_text_button::set_bit(Bitmap *fond) {
	if(fond)
		bit = new Bitmap((*fond)[y]+x, w-2, h-2, fond->realwidth);
	else
		bit = NULL;
}

void Zone_text_button::draw() {
	if(!bit)
		video->vb->rect(x+1, y+1, w-2, h-2, 0);
	if(high) {
		if(bit)
			bit->draw(video->vb, x+2, y+2);
		actual->draw(st, video->vb, text_x+4, y+3);
		video->vb->vline(x, y, h, 0);
		video->vb->hline(y, x, w, 0);
		video->vb->vline(x+w-1, y+1, h-1, 0);
		video->vb->hline(y+h-1, x+1, w-1, 0);
		video->vb->vline(x+1, y+1, h-2, 0);
		video->vb->hline(y+1, x+1, w-2, 0);
		video->vb->vline(x+w-2, y+2, h-3, 255);
		video->vb->hline(y+h-2, x+2, w-3, 255);
	} else {
		if(bit)
			bit->draw(video->vb, x, y);
		actual->draw(st, video->vb, text_x+3, y+2);
		video->vb->vline(x, y, h, 0);
		video->vb->hline(y, x, w, 0);
		video->vb->vline(x+w-1, y+1, h-1, 0);
		video->vb->hline(y+h-1, x+1, w-1, 0);
		video->vb->vline(x+1, y+1, h-2, 255);
		video->vb->hline(y+1, x+1, w-2, 255);
		video->vb->vline(x+w-2, y+2, h-3, 0);
		video->vb->hline(y+h-2, x+2, w-3, 0);
	}
}

void Zone_text_button::entered() {
	Zone_text_select::entered();
	high = true;
}

void Zone_text_button::leaved() {
	Zone_text_select::leaved();
	high = false;
}

Zone_panel::Zone_panel(Inter* in, int px, int py, int pw, int ph):
Zone(in, px, py, pw, ph) {
	draw_frame = true;
	pan = NULL;
	high = false;
	resize();
}

void Zone_panel::resize() {
	if(pan)
		delete pan;
	pan = Video_bitmap::New(x+2, y+2, max(w-4, 0), max(h-4, 0));
	dirt();
}

void Zone_panel::draw() {
	if(draw_frame) {
		video->vb->hline(y, x, w, 210);
		video->vb->hline(y+h-1, x, w, 210);
		video->vb->vline(x, y+1, h-2, 210);
		video->vb->vline(x+w-1, y+1, h-2, 210);
		if(high) {
			video->vb->hline(y+1, x+1, w-2, 255);
			video->vb->hline(y+h-2, x+1, w-2, 255);
			video->vb->vline(x+1, y+2, h-4, 255);
			video->vb->vline(x+w-2, y+2, h-4, 255);
			if(h-4 > 0)
				video->vb->rect(x+2, y+2, w-4, h-4, 0);
		} else {
			if(h-2 > 0)
				video->vb->rect(x+1, y+1, w-2, h-2, 0);
		}
	}
	pan->setmem();
}

Zone_text_input::Zone_text_input(Inter* in, const Palette& pal, char* s, int mlen, int px, int py, int pw, int mwidth):
Zone_panel(in, px, py, pw, in->font->height()+2) {
	set_val(s);
	focusable = 1;
	focus = 0;
	maxlen = mlen-1;
	maxwidth = mwidth;
	kb_focusable = true;

	Remap temp(pal);
	temp.findrgb(0, 255, 255, 0);
	curcolor = temp.map[0];

	font_selected = new Font(in->font->fdata_original, pal, 0, 0, 0, 255, 255, 0);
	select_start = -1;
	panx = 0;
}

Zone_text_input::~Zone_text_input() {
	if(focus)
		lost_focus(1); // force une perte de focus si delete zone
	delete font_selected;
}

void Zone_text_input::set_val(char* s) {
	if(s)
		strcpy(st, s);
	else
		st[0] = 0;
	val = s;
	dirt();
}

void Zone_text_input::clicked(int quel) {
	if(focus == 0) {
		focus = 10;
		curpos = strlen(st);
		actual_len = curpos;
		select_start = 0; // select_all par defaut
		panx = 0;
		input->deraw();
		input->clear_key();
		Zone_panel::clicked(quel);
		high = true;
		dirt();
		first_click = false;
	} else {
		first_click = true;
		if(input->shift_key & SHIFT) {
			if(select_start == -1)
				select_start = curpos;
		} else {
			select_start = -1;
		}
		set_mouse_curpos();
		dirt();
	}
}

void Zone_text_input::waiting() {
	if(focus != 0 && first_click) {
		if(input->mouse.button[0] & PRESSED) {
			if(select_start == -1)
				select_start = curpos;
			set_mouse_curpos();
		}
	}
}

void Zone_text_input::set_mouse_curpos() {
	int i;
	int px = cursor->x - x + panx;
	int last_width = 0;
	for(i=1; i<=actual_len; i++) {
		int sx = inter->font->width(st, i);
		if(px < (last_width + (sx-last_width)/2)) {
			curpos = i-1;
			break;
		}
		last_width = sx;
	}
	if(i > actual_len)
		curpos = actual_len;
}

void Zone_text_input::draw() {
	int sx;
	Zone_panel::draw();
	if(focus) {
		int total_x = inter->font->width(st);
		sx = inter->font->width(st, curpos);
		if(sx - panx > w-8) {
			panx = min(panx + (w>>2), total_x - (w-8));
		}
		if(sx - panx < 0) {
			panx = max(0, panx - (w>>2));
		}
		inter->font->draw(st, pan, -panx, 0);

		if(select_start != -1 && select_start != curpos) {
			int x1, x2, x3, x4;
			if(select_start > curpos) {
				x1 = curpos;
				x2 = select_start;
			} else {
				x2 = curpos;
				x1 = select_start;
			}
			x3 = inter->font->width(st, x1)-2; // -2: enleve le 'shrink' ajouter par width()
			x4 = inter->font->width(st, x2)-2; // -2: enleve le 'shrink' ajouter par width()
			pan->rect(x3-panx, 0, x4-x3+2, inter->font->height(), curcolor);

			char tube_char = st[x2];
			st[x2] = 0;
			font_selected->draw(&st[x1], pan, x3-panx, 0);
			st[x2] = tube_char;
		}
		if(focus > 10) {
			pan->vline(sx-panx-2, 0, h, curcolor);
			pan->vline(sx-panx-1, 0, h, curcolor);
		}
	} else {
		inter->font->draw(st, pan, 0, 0);
	}
}

void Zone_text_input::lost_focus(int cancel) {
	focus = 0;
	dirt();
	if(cancel)
		strcpy(st, val);
	else
		strcpy(val, st);
	input->reraw();
}

void Zone_text_input::process() {
	Byte c;
	if(focus) {
		// Support du Clipboard en Doze
		check_clipboard();
		for(int i=0; i<input->key_pending; i++) {
			c = input->key_buf[i].c;
			if((c == 8) || (c == 127)) {  // backspace
				if(!cut_selection()) { // si rien de selecter n'a ete couper,
					if(curpos > 0) { // procede a un backspace normal
						curpos--;
						memmove(&st[curpos], &st[curpos+1], actual_len - curpos);
						actual_len--;
					}
				}
				continue;
			}
			if(input->key_buf[i].special) { // touches de deplacement et autres
				if(c == 46) {// delete
					if(!cut_selection()) { // si rien de selecter n'a ete couper,
						if(curpos != actual_len) { // procede a un delete normal
							memmove(&st[curpos], &st[curpos+1], actual_len - curpos);
							actual_len--;
						}
					}
				}
				if(c==37 || c==39 || c==36 || c==35) {
					if(input->shift_key & SHIFT) {
							if(select_start == -1)
								select_start = curpos;
					} else {
						select_start = -1;
					}
				}
				if(c == 37 && curpos > 0) { // fleche gauche
					curpos--;
				}
				if(c == 39 && curpos != actual_len) { // fleche droite
					curpos++;
				}
				if(c == 36) { // home
					curpos = 0;
				}
				if(c == 35) { // end
					curpos = actual_len;
				}
				continue;
			}
			if(c == 1) { // CTRL-A (select all)
				curpos = actual_len;
				select_start = 0;
			}
			input_char(c);
		}
		input->key_pending = 0;
		focus++;
		if(focus > 20)
			focus=1;
		dirt();
	}
}

void Zone_text_input::input_char(const Byte c) {
	if(c > 31 && c != 183 && c != 127) {
		cut_selection();
		memmove(&st[curpos+1], &st[curpos], actual_len-curpos + 1);
		st[maxlen] = 0;
		if(curpos < maxlen) {
			st[curpos] = c;
			curpos++;
		}
		if(actual_len < maxlen) {
			actual_len++;
		}
		if(maxwidth != -1) {
			if(inter->font->width(st) > maxwidth) {
				actual_len--;
				if(curpos > actual_len)
					curpos = actual_len;
				st[actual_len] = 0;
			}
		}
	}
}

bool Zone_text_input::cut_selection() {
	if(select_start == -1 || select_start == curpos) {
		select_start = -1;
		return false;
	}
	int x1, x2;
	if(select_start > curpos) {
		x1 = curpos;
		x2 = select_start;
	} else {
		x2 = curpos;
		x1 = select_start;
	}
	memmove(&st[x1], &st[x2], actual_len - x2+1);
	actual_len = actual_len - (x2-x1);
	select_start = -1;
	curpos = x1;
	return true;
}

void Zone_text_input::check_clipboard() {
#ifdef UGS_DIRECTX
	if(input->shift_key != CONTROL)
		return;
	if(!input->keys[DIK_V] & PRESSED && !input->keys[DIK_C] & PRESSED && !input->keys[DIK_X] & PRESSED)
		return;
	if(!OpenClipboard(NULL)) {
		skelton_msgbox("  Error opening clipboard.\n");
		return;
	}
	if(input->keys[DIK_V] & PRESSED) {
		input->keys[DIK_V] = 0;
		HANDLE h = GetClipboardData(CF_TEXT);
		if(h == NULL) {
			skelton_msgbox("  Error getting clipboard data.\n");
		} else {
			char *clip = (char *) h;
			cut_selection();
			int ma = strlen(clip);
			for(int i=0; i<ma; i++)
				input_char(clip[i]);
		}
	}
	bool cut=false,copy=false;
	if(input->keys[DIK_X] & PRESSED) {
		input->keys[DIK_X] = 0;
		cut = true;
	}
	if(input->keys[DIK_C] & PRESSED) {
		input->keys[DIK_C] = 0;
		copy = true;
	}
	if(copy || cut) {
		int x1, x2;
		if(select_start > curpos) {
			x1 = curpos;
			x2 = select_start;
		} else {
			x2 = curpos;
			x1 = select_start;
		}
		int ma = x2-x1+1;
		if(ma > 1) {
			HANDLE mem = GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE, ma);
			LPVOID buf = GlobalLock(mem);
			memcpy((char *) buf, &st[x1], ma-1);
			((char *)buf)[ma-1] = 0;
			GlobalUnlock(mem);
			HANDLE h = SetClipboardData(CF_TEXT, mem);
			if(h == NULL) {
				skelton_msgbox("  Error setting clipboard data.\n");
			} else {
				if(cut) {
					cut_selection();
				}
			}
		} else {
			skelton_msgbox("  nothing selected. aborting.\n",ma);
		}
	}
	CloseClipboard();
#endif
}

void Zone_text_input::leaved() {
	Zone_panel::leaved();
	high = false;
	dirt();
}

void Zone_text_input::entered() {
	Zone_panel::entered();
	high = true;
	dirt();
}

Zone_input_numeric::Zone_input_numeric(Inter* in, int *pvar, int ncar, int pmin, int pmax, const Palette &pal, int px, int py, int pw):
	Zone_text_input(in, pal, (sprintf(temp_st, "%i",*pvar), temp_st), ncar, px, py, pw) {
	/* P.S.: ya-tu une autre facon d'ecrire la ligne si-dessus, ou ben
	   le C++ c'est vraiment de la marde? */
	var_min = pmin;
	var_max = pmax;
	num_var = pvar;
}

void Zone_input_numeric::lost_focus(int cancel) {
	if(!cancel) {
		int num;
		if(sscanf(Zone_text_input::st, "%i", &num) != 1)
			num = 0;
		if(num >= var_min && num <= var_max) {
			*num_var = num;
		} else {  // si invalide, cancel l'entree
			cancel = 1;
		}
	}
	Zone_text_input::lost_focus(cancel);
}

Zone_text_field::Zone_text_field(Inter* in, int* s, int px, int py, int pw, Font *f2, bool frame):
Zone_panel(in, px, py, pw, in->font->height()+2) {
	if(f2 == NULL)
		font = in->font;
	else
		font = f2;
	set_val(s);
	draw_frame = frame;
}
Zone_text_field::Zone_text_field(Inter* in, const char* s, int px, int py, int pw, Font *f2, bool frame):
Zone_panel(in, px, py, pw, in->font->height()+2) {
	if(f2 == NULL)
		font = in->font;
	else
		font = f2;
	set_val(s);
	draw_frame = frame;
}

void Zone_text_field::process() {
	if(var && val != *var)
		set_val(var);
}

void Zone_text_field::set_val(int* s) {
	val = *s;
	var = s;
	sprintf(st, "%i", val);
	dirt();
}

void Zone_text_field::set_val(const char* s) {
	var = NULL; // c'est une string
	if(s) {
		strncpy(st, s, sizeof(st)-1);
		st[sizeof(st)-1] = 0;
	} else {
		st[0] = 0;
	}
	dirt();
}

void Zone_text_field::draw() {
  Zone_panel::draw();
	if(!draw_frame) {
		video->vb->hline(y, x, w, 255);
		video->vb->hline(y+h-1, x, w, 0);
		video->vb->vline(x, y+1, h-2, 255);
		video->vb->vline(x+w-1, y+1, h-2, 0);
		video->vb->rect(x+1, y+1, w-2, h-2, 210);
	}
	if(var) {
		font->draw(st, pan, w - font->width(st) - 3, 0);  // alignement des numerics a droite
	} else {
		font->draw(st, pan, 3, 0);  // alignement du texte a gauche
	}
}

Zone_clear::Zone_clear(Inter* in, int px, int py, int pw, int ph, int c):
Zone(in, px, py, pw, ph) {
	color=c;
}

void Zone_clear::draw() {
	video->vb->rect(x, y, w, h, color);
}

Inter::Inter() {
	first_zone = 0;
	font = NULL;
	kb_x = kb_y = 0;
	kb_active = true;
	flush();
}

Inter::Inter(Inter *in) {
	for(int i=0; i<in->nzone(); i++)
		add(in->zone[i]);
	first_zone = nzone();
	set_font(in->font, false);
	kb_active = true;
	flush();
}

Inter::~Inter() {
	flush();
	if(font && del_font)
		delete font;
}

void Inter::set_font(Font* f1, bool del) {
	font = f1;
	del_font = del;
}

void Inter::draw_zone() {
	int i;
#ifdef UGS_DIRECTX
	if(!alt_tab)
#endif
	{
		if(video->need_paint) {
			dirt_all();
			video->need_paint--;
		}
		for(i=0; i<nzone(); i++) {
			if(zone[i]->dirty && zone[i]->enabled >=0 && !zone[i]->stay_on_top) {
				zone[i]->dirty--;
				zone[i]->draw();
			}
		}
		kb_draw_focus();
		for(i=0; i<nzone(); i++) {
			if(zone[i]->enabled >=0 && zone[i]->stay_on_top) {
				zone[i]->draw();
			}
		}
	}
}

void Inter::dirt_all() {
	for(int i=0; i<nzone(); i++)
		zone[i]->dirt();
}

Zone* Inter::do_frame() {
	process();
	return clicked;
}

void Inter::remove(Zone *z) {
	for(int i=0; i<nzone(); i++)
		if(zone[i] == z) {
			remove(i);
			break;
		}
}

void Inter::remove(int i) {
	if(in == zone[i])
		in = NULL;
	if(focus == zone[i])
		focus = NULL;
	if(kb_visible && kb_focus == zone[i])
		kb_focus = NULL;
	if(double_clicked_first == zone[i]) {
		double_clicked_first = NULL;
		double_click_delay = 0;
	}
	zone.remove(i);
}

void Inter::flush() {
	while(first_zone != nzone())
		delete zone[first_zone];
	in = NULL;
	focus = NULL;
	clicked = double_clicked = double_clicked_first = NULL;
	double_click_delay = 0;
	kb_focus = NULL;
	kb_x = kb_y = 0;
	kb_anim = 0;
	input->quel_key = -1;
}

void Inter::process() {
	int i;
	clicked = double_clicked = NULL;
	for(i = nzone()-1; i >= first_zone; i--)
		if(zone[i]->enabled >=0)
			zone[i]->process();

	if(kb_visible && !kb_focus) {
		kb_focus = kb_find_closest();
		if(kb_focus)
			tag(kb_focus);
	}

	if(focus) {
		int lost=-1;
		if(input->quel_key == KEY_ESCAPE)
			lost=1;
		if(input->quel_key == KEY_ENTER || input->quel_key == KEY_PADENTER)
			lost=0;
		if(lost != -1) {
			focus->lost_focus(lost);
			if(!kb_visible && in != focus)
				focus->leaved();
			focus = NULL;
			input->quel_key = -1;
		}
	} else {
		// stuff pour keyboard control
		if(!kb_visible) {
			if(kb_focus != NULL) { // si une zone etait focuser et que le kb_visible est devenu faux
				de_tag(kb_focus); // il faut la detagger.
				kb_focus = NULL;
			}
			if(kb_active) {
				if(kb_check_key(KEY_DOWNARROW) || kb_check_key(KEY_UPARROW) ||
					kb_check_key(KEY_LEFTARROW) || kb_check_key(KEY_RIGHTARROW) ||
					kb_check_key(KEY_TAB)) {
					kb_focus=NULL;
					if(in) {
						if(in->kb_focusable)
							kb_focus = in;
						else if(in->parent && in->parent->kb_focusable) {
							kb_focus = in->parent;
						}
					}
					if(!kb_focus)
						kb_focus = kb_find_upmost();
					if(kb_focus) { // si une zone 'focusable' dans toute l'interface
						last_mouse_x = cursor->x;
						last_mouse_y = cursor->y;
						kb_anim = 0;
						in = NULL;
						tag(kb_focus);
						kb_visible = true;
						if(cursor)
							cursor->visible = false;
					}
					input->quel_key = -1;
				}
			}
		} else {
			if(last_mouse_x != cursor->x || last_mouse_y != cursor->y || alt_tab) {
				// la souris a bouge: enleve le kb_focus
				kb_visible = false;
				if(kb_focus) {
					de_tag(kb_focus);
					kb_focus = NULL;
				}
			} else {
				if(!kb_focus) {
					kb_focus = kb_find_closest();
				}
				if(kb_focus && (kb_focus->enabled < 0 || !kb_focus->kb_focusable)) { // si la zone keyboard a ete disabler ou n'est plus kb_focusable
					de_tag(kb_focus);
					kb_focus = kb_find_closest();
					if(kb_focus)
						tag(kb_focus);
				}
			}
			if(kb_visible) {
				bool bouge = false;
				Zone *temp = NULL;
				if(kb_check_key(KEY_DOWNARROW)) {
					temp = kb_find_down();
					bouge = true;
				}
				if(kb_check_key(KEY_TAB)) {
					temp = kb_find_next();
					bouge = true;
				}
				if(kb_check_key(KEY_TAB) && input->shift_key & SHIFT) {
					temp = kb_find_prev();
					bouge = true;
				}
				if(kb_check_key(KEY_UPARROW)) {
					temp = kb_find_up();
					bouge = true;
				}
				if(kb_check_key(KEY_RIGHTARROW)) {
					temp = kb_find_right();
					if(temp)
						bouge = true;
				}
				if(kb_check_key(KEY_LEFTARROW)) {
					temp = kb_find_left();
					if(temp)
						bouge = true;
				}
				if(bouge) {
					if(temp && temp != kb_focus) {
						de_tag(kb_focus);
						tag(temp);
						kb_focus = temp;
					}
					input->quel_key = -1;
				}

				if(input->quel_key == KEY_ENTER || input->quel_key == KEY_PADENTER || kb_check_key(KEY_SPACE)) {
					if(kb_focus) {
						select_zone(kb_focus, 0);
						input->quel_key = -1;
					}
				}
			}
		}
	}
	if(cursor && !cursor->visible) {
		if(last_mouse_x != cursor->x || last_mouse_y != cursor->y) {
			// la souris a bouge: refait apparaitre le curseur
			cursor->visible = true;
		}
	}

	if(!kb_visible) {
		for(i=nzone()-1; i >= first_zone; i--) {
			if(zone[i]->in() && zone[i]->enabled >=0) {
				if(focus) {
					if(input->mouse.quel != -1) {
						if(focus != zone[i]) {
							focus->lost_focus(0);
							if(in != focus)
								focus->leaved();
							focus = NULL;
						}
					}
				}
				if(!focus || !in) {
					if(in != zone[i]) {
						if(in)
							de_tag(in);
						in = zone[i];
						tag(in);
					}
				}
				if(in->parent)
					in->parent->waiting();
				else
					in->waiting();
				if(input->mouse.quel != -1) {
					select_zone(zone[i], input->mouse.quel);
				}
				break;
			}
		}
	}
	if(double_click_delay)
		double_click_delay--;
}

void Inter::de_tag(Zone *z) {
	if(z->parent)
		z->parent->leaved();
	else
		z->leaved();
}

void Inter::tag(Zone *z) {
	if(z->parent)
		z->parent->entered();
	else
		z->entered();
	kb_x = z->x;
	kb_y = z->y;
}

void Inter::kb_deactivate() {
	kb_active = false;
	kb_visible = false;
}

void Inter::kb_reactivate() {
	kb_active = true;
}

void Inter::kb_alloc_key(const int i) {
	kb_keys.add(i);
}

void Inter::kb_free_key(const int i) {
	kb_keys.remove_item(i);
}

bool Inter::kb_check_key(const int i) const {
	if(input->quel_key == i) {
		for(int j=0; j<kb_keys.size(); j++)
			if(i == kb_keys[j])
				return false;
		return true;
	}
	return false;
}

Zone *Inter::kb_find_upmost() {
	int best_y = 999;
	Zone *best = NULL;
	for(int i = nzone()-1; i >= first_zone; i--)
		if(zone[i]->enabled >=0 && zone[i]->kb_focusable)
			if(zone[i]->y < best_y) {
				best = zone[i];
				best_y = zone[i]->y;
			}
	return best;
}

Zone *Inter::kb_find_downmost() {
	int best_y = -1;
	Zone *best = NULL;
	for(int i = nzone()-1; i >= first_zone; i--)
		if(zone[i]->enabled >=0 && zone[i]->kb_focusable)
			if(zone[i]->y > best_y) {
				best = zone[i];
				best_y = zone[i]->y;
			}
	return best;
}

Zone *Inter::kb_find_down() {
	int best_y = 9999999, dist;
	Zone *best = NULL;
	for(int i = nzone()-1; i >= first_zone; i--) {
		Zone *z = zone[i];
		if(z->enabled >=0 && z->kb_focusable)
			if(z->y > kb_y && (abs(z->x - kb_x)>>1) < z->y - kb_y) {
				dist = (z->x - kb_x)*(z->x - kb_x) + (z->y - kb_y)*(z->y - kb_y);
				if(dist < best_y) {
					best = z;
					best_y = dist;
				}
			}
	}
	return best;
}

Zone *Inter::kb_find_up() {
	int best_y = 9999999, dist;
	Zone *best = NULL;
	for(int i = nzone()-1; i >= first_zone; i--) {
		Zone *z = zone[i];
		if(z->enabled >=0 && z->kb_focusable)
			if(z->y < kb_y && (abs(z->x - kb_x)>>1) < kb_y - z->y) {
				dist = (z->x - kb_x)*(z->x - kb_x) + (z->y - kb_y)*(z->y - kb_y);
				if(dist < best_y) {
					best = z;
					best_y = dist;
				}
			}
	}
	return best;
}

Zone *Inter::kb_find_right() {
	int best_y = 9999999, dist;
	Zone *best = NULL;
	for(int i = nzone()-1; i >= first_zone; i--) {
		Zone *z = zone[i];
		if(z->enabled >=0 && z->kb_focusable)
			if(z->x > kb_x && (abs(z->y - kb_y)>>1) < z->x - kb_x) {
				dist = (z->x - kb_x)*(z->x - kb_x) + (z->y - kb_y)*(z->y - kb_y);
				if(dist < best_y) {
					best = z;
					best_y = dist;
				}
			}
	}
	return best;
}

Zone *Inter::kb_find_left() {
	int best_y = 9999999, dist;
	Zone *best = NULL;
	for(int i = nzone()-1; i >= first_zone; i--) {
		Zone *z = zone[i];
		if(z->enabled >=0 && z->kb_focusable)
			if(z->x < kb_x && (abs(z->y - kb_y)>>1) < kb_x - z->x) {
				dist = (z->x - kb_x)*(z->x - kb_x) + (z->y - kb_y)*(z->y - kb_y);
				if(dist < best_y) {
					best = z;
					best_y = dist;
				}
			}
	}
	return best;
}

Zone *Inter::kb_find_closest() {
	int best_y = 9999999, dist;
	Zone *best = NULL;
	for(int i = nzone()-1; i >= first_zone; i--) {
		Zone *z = zone[i];
		if(z->enabled >=0 && z->kb_focusable) {
			dist = (z->x - kb_x)*(z->x - kb_x) + (z->y - kb_y)*(z->y - kb_y);
			if(dist < best_y) {
				best = z;
				best_y = dist;
			}
		}
	}
	return best;
}

Zone *Inter::kb_find_prev() {
	int i;
	int debut =nzone()-1;

	// trouve la zone actuellement focuser...
	for(i = nzone()-1; i >= first_zone; i--) {
		Zone *z = zone[i];
		if(z == kb_focus) {
			debut = i;
			break;
		}
	}
	// ensuite trouve la 'previous' zone focusable (a partir de celle trouve)
	for(i = debut-1; i >= first_zone; i--) {
		Zone *z = zone[i];
		if(z->enabled >=0 && z->kb_focusable)
			return z;
	}
	// si rien de bon, repart de la fin (pour looper)
	for(i = nzone()-1; i >= first_zone; i--) {
		Zone *z = zone[i];
		if(z->enabled >=0 && z->kb_focusable)
			return z;
	}
	return NULL; // si rien pentoute
}

Zone *Inter::kb_find_next() {
	int i;
	int debut =0;

	// trouve la zone actuellement focuser...
	for(i = nzone()-1; i >= first_zone; i--) {
		Zone *z = zone[i];
		if(z == kb_focus) {
			debut = i;
			break;
		}
	}
	// ensuite trouve la 'next' zone focusable (a partir de celle trouve)
	for(i = debut+1; i < nzone(); i++) {
		Zone *z = zone[i];
		if(z->enabled >=0 && z->kb_focusable)
			return z;
	}
	// si rien de bon, repart du debut (pour looper)
	for(i = first_zone; i < nzone(); i++) {
		Zone *z = zone[i];
		if(z->enabled >=0 && z->kb_focusable)
			return z;
	}
	return NULL; // si rien pentoute
}

void Inter::select_zone(Zone *z, int quel) {
	if(focus) {
		if(focus != z) {
			focus->lost_focus(0);
			if(in != focus)
				focus->leaved();
			focus = NULL;
		}
	}
	if(z->focusable) {
		focus = z;
		input->key_pending = 0;
	}
	if(z->parent) {
		z->parent->clicked(quel);
		clicked = z->parent;
	} else {
		if(z->child)
			if(z->child->focusable) {
				focus = z->child;
				input->key_pending = 0;
			}
		clicked = z;
		z->clicked(quel);
	}

	// double-click (TODO: ne supporte pas les 'child' Zone)
	if(double_clicked_first == z &&	double_click_delay > 0) {
		double_clicked = z;
		double_clicked_first = NULL;
		z->double_clicked();
	} else {
		double_clicked_first = z;
		double_click_delay = 70; // delai fix de 70/100 de seconde
	}

	if(kb_visible && kb_focus != z) {
		if(kb_focus)
			de_tag(kb_focus);
		kb_focus = z;
		tag(kb_focus);
	}
	input->mouse.quel = -1; // clear pour empecher multiple click dans la meme frame
}

void Inter::kb_draw_focus() {
	if(kb_visible && kb_focus) {
		int x,y,w,h;
		x = kb_focus->x;
		y = kb_focus->y;
		w = kb_focus->w;
		h = kb_focus->h;
		/*video->vb->hline(y, x, w, 255);
		video->vb->hline(y+h-1, x, w, 255);
		video->vb->vline(x, y, h, 255);
		video->vb->vline(x+w-1, y, h, 255);*/
		//kb_anim++;
		int seed = kb_anim & 7;
		bool draw;
		if(kb_anim & 8)
			draw = true;
		else
			draw = false;
		int side = 0;
		int x2 = x;
		int y2 = y;
		do {
			switch(side) {
				case 0: x2++;
					if(x2 >= x+w) {
						x2 = x+w-1;
						side++;
					}
					break;
				case 1: y2++;
					if(y2 >= y+h) {
						y2 = y+h-1;
						side++;
					}
					break;
				case 2: x2--;
					if(x2 < x) {
						x2 = x;
						side++;
					}
					break;
				case 3: y2--;
					if(y2 < y) {
						y2 = y;
						side++;
					}
					break;
			}
			if(draw)
				video->vb->put_pel(x2, y2, 255);
			seed++;
			//if(seed == 8) {
				draw = !draw;
				seed = 0;
			//}
		} while(x2 != x || y2 != y);
	}
}

