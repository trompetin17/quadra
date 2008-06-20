/* -*- Mode: C++; c-basic-offset: 2; tab-width: 2; indent-tabs-mode: nil -*-
 *
 * Quadra, an action puzzle game
 * Copyright (C) 1998-2000  Ludus Design
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "video.h"

#include <assert.h>
#ifdef WIN32
#include <malloc.h>
#else
#include <alloca.h>
#endif
#include "sprite.h"
#include "command.h"
#include "version.h"
#include "image_png.h"
#include "cursor.h"

Video* video = NULL;

Video_bitmap::Video_bitmap(const int px, const int py, const int w,
                           const int h):
  Clipable(w, h),
  pos_x(px),
  pos_y(py) {
}

Video_bitmap::~Video_bitmap() {
}

void Video_bitmap::rect(int x, int y, int w, int h, int color) const {
  SDL_Rect rect;

  if(clip(x, y, w, h))
    return;

  rect.x = clip_x1 + pos_x;
  rect.y = clip_y1 + pos_y;
  rect.w = clip_x2 - clip_x1 + 1;
  rect.h = clip_y2 - clip_y1 + 1;

  SDL_FillRect(video->surface(), &rect, color);
  clip_dirty(x, y, w, h); 
}

void Video_bitmap::box(int x, int y, int w, int h, int c) const {
  hline(y, x, w, c);
  hline(y + h - 1, x, w, c);
  vline(x, y, h, c);
  vline(x + w - 1, y, h, c);
}

void Video_bitmap::put_pel(int x, int y, Byte c) const {
  rect(x, y, 1, 1, c);
}

void Video_bitmap::hline(int y, int x, int w, Byte c) const {
  rect(x, y, w, 1, c);
}

void Video_bitmap::vline(int x, int y, int h, Byte c) const {
  rect(x, y, 1, h, c);
}

void Video_bitmap::clip_dirty(int x, int y, int w, int h) const {
  if(clip(x, y, w, h))
    return;
  video->set_dirty(pos_x+clip_x1, pos_y+clip_y1, pos_x+clip_x2, pos_y+clip_y2);
}

void Video_bitmap::put_surface(SDL_Surface* surface, int dx, int dy) const {
  SDL_Rect rect;
  rect.x = rect.y = 0;
  rect.w = surface->w;
  rect.h = surface->h;

  put_surface(surface, rect, dx, dy);
}

void Video_bitmap::put_surface(SDL_Surface* surface, const SDL_Rect& _srcrect, int dx, int dy) const {
	assert(_srcrect.x >= 0 && _srcrect.y >= 0);
	assert(_srcrect.w > 0 && _srcrect.h > 0);
	assert(_srcrect.x < surface->w && _srcrect.y < surface->h);
	assert(_srcrect.x + _srcrect.w <= surface->w);
	assert(_srcrect.y + _srcrect.h <= surface->h);

  if (clip(dx, dy, _srcrect.w, _srcrect.h))
    return;

  SDL_Rect srcrect;
  srcrect.x = _srcrect.x + clip_x1 - dx;
  srcrect.y = _srcrect.y + clip_y1 - dy;
  srcrect.w = _srcrect.w + clip_x1 - dx;
  srcrect.h = _srcrect.h + clip_y1 - dy;

  SDL_Rect dstrect;
  dstrect.x = pos_x + clip_x1;
  dstrect.y = pos_y + clip_y1;

  clip_dirty(dx, dy, srcrect.w, srcrect.h);
  SDL_BlitSurface(surface, &srcrect, video->surface(), &dstrect);
}

void Video_bitmap::put_bitmap(const Bitmap &d, int dx, int dy) const {
  SDL_SetColors(d.surface, video->surface()->format->palette->colors, 0,
                video->surface()->format->palette->ncolors);
  put_surface(d.surface, dx, dy);
}

Video::Video():
  vb(0, 0, 640, 480),
  need_paint(),
  framecount(0),
  newpal(),
  pal(),
  needsleep(0),
  lastticks(SDL_GetTicks()),
  fullscreen(!command.token("nofullscreen")),
  mDirtyEmpty(true),
	mDirtyX1(),
	mDirtyY1(),
	mDirtyX2(),
	mDirtyY2(),
  display(NULL) {
  SetVideoMode();
}

Video::~Video() {
}

void Video::end_frame() {
  if (newpal) {
    SDL_SetColors(display, pal.pal, 0, pal.size);
    newpal = false;
  }

	// Draw and convert only the dirty region to screen
	if(!mDirtyEmpty)
	{
		SDL_Rect rect;
		rect.x = mDirtyX1;
		rect.y = mDirtyY1;
		rect.w = mDirtyX2 - mDirtyX1 + 1;
		rect.h = mDirtyY2 - mDirtyY1 + 1;
		SDL_UpdateRect(display, rect.x, rect.y, rect.w, rect.h);
		mDirtyEmpty = true;
	}

	// RV: Make system sleep a little bit to keep framerate around 100 FPS
	{
		int CurrentTicks = SDL_GetTicks();
		int LastDeltaTicks = CurrentTicks - lastticks;
		// If a fairly large delay occured, ignore it and clamp it down to a 1/60.
		// This prevents application from trying to skip several seconds in case
		// system was halted or user dragged the window
		if(LastDeltaTicks > 100)
		{
			LastDeltaTicks = 10;
		}
		lastticks = CurrentTicks;

		// Try to get about 100 FPS (1000 msec / 100 = 10)
		needsleep += (1000/100)-LastDeltaTicks;
		// If needed, sleep a bit to slow down execution speed
		if(needsleep > 0)
		{
			SDL_Delay(needsleep);
		}
		else // Else, simply briefly yield to system
		{
			SDL_Delay(0);
		}
	}

  ++framecount;
}

void Video::setpal(const Palette &p) {
  pal = p;
  newpal = true;
}

void Video::snap_shot(int, int, int, int) {
  assert(false);
}

void Video::toggle_fullscreen() {
  fullscreen = !fullscreen;
  SetVideoMode();
}

void Video::SetVideoMode()
{
	// Set window title and window icon using SDL
	{
		char st[256];
		sprintf(st, "Quadra %s", VERSION_STRING);
		SDL_WM_SetCaption(st, NULL);
		Res_doze res("window_newicon.png");
		Png img(res);
    SDL_Surface* surf = img.new_surface();

		// Fetch colorkey from top-left pixel value
		SDL_SetColorKey(surf, SDL_SRCCOLORKEY, img.pic()[0]);

		SDL_WM_SetIcon(surf, NULL);
		SDL_FreeSurface(surf);
	}

  int flags = SDL_HWSURFACE;
  if(fullscreen) flags |= SDL_FULLSCREEN;
  display = SDL_SetVideoMode(640, 480, 8, flags);
  assert(display);
  need_paint = 2;
  newpal = true;
}

void Video::set_dirty(int x1, int y1, int x2, int y2)
{
	if(mDirtyEmpty)
	{
		mDirtyX1 = x1;
		mDirtyY1 = y1;
		mDirtyX2 = x2;
		mDirtyY2 = y2;
		mDirtyEmpty = false;
	}
	else
	{
		if(x1 < mDirtyX1) mDirtyX1 = x1;
		if(y1 < mDirtyY1) mDirtyY1 = y1;
		if(x2 > mDirtyX2) mDirtyX2 = x2;
		if(y2 > mDirtyY2) mDirtyY2 = y2;
	}
}
