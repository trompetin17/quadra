/* -*- Mode: C++; c-basic-offset: 2; tab-width: 2; indent-tabs-mode: nil -*-
 * Copyright (c) 1998-2000 Ludus Design enr.
 * All Rights Reserved.
 * Tous droits r�serv�s.
 */

#include "pcx.h"

Pcx::Pcx(Res& res) {
	Error *error;
	res.read(&h, sizeof(h));
	if(h.nplane != 1)
		error = new Error("This PCX file has more than 1 plane!");
	if(h.version != 5)
		error = new Error("PCX file must be version 5");
	width_ = h.x2-h.x1+1;
	height_ = h.y2-h.y1+1;
	pic_=new Byte[width_*height_];
	if(pic_==NULL)
		error = new Error("Not enough memory to load PCX");
	pal_=new Byte[256*3];
	if(pal_==NULL)
		error = new Error("Not enough memory to load PCX");
	Byte* buf = (Byte *) res.buf();
	Byte c, num;
	Byte* out = pic_;
	int x,i;
	if(width_ == h.byteperline) { // depack plus rapidement si bonne largeur
		for(i=0; i < height_; i++) {
			x = 0;
			while(x < width_) {
				c = *buf++;
				if(0xC0 == (0xC0 & c)) {
					num = (Byte) (0x3F & c);
					c = *buf++;
					x += num;
					while(num--)
						*out++ = c;
				} else {
					*out++ = c;
					x++;
				}
			}
		}
	} else { // maudit PCX de merde avec des mauvais bytes au bout des lignes!!
		for(i=0; i < height_; i++) {
			x = 0;
			while(x < h.byteperline) {
				c = *buf++;
				if(0xC0 == (0xC0 & c)) {
					num = (Byte) (0x3F & c);
					c = *buf++;
					x += num;
					if(x > width_) {
						num = (Byte) (num - (x-width_));
					}
					while(num--)
						*out++ = c;
				} else {
					if(x < width_)
						*out++ = c;
					x++;
				}
			}
		}
	}
	if(*buf++ != 12)
		error = new Error("Can't find palette in PCX file");
	cpy(pal_, buf, 768);
}

Pcx::~Pcx() {
	if(pic_)
		delete[] pic_;
	if(pal_)
		delete[] pal_;
}