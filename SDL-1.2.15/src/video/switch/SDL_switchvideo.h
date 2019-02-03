/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2012 Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Sam Lantinga
    slouken@libsdl.org
*/  
#include "SDL_config.h"

#ifndef _SDL_switchvideo_h
#define _SDL_switchvideo_h

#include "../SDL_sysvideo.h"

#include "switch.h"

/* Hidden "this" pointer for the video functions */
#define _THIS	SDL_VideoDevice *this


/* Private display data */

struct SDL_PrivateVideoData {
    int w, h;
    int pw, ph; // phisical framebuffer dimensions
    int bpp;
    void *buffer;
	unsigned int flags; // backup of create device flags
	u8 clearbuffer; // number of full hw buffer to clear at startup and after toggling fullscreen (and maybe other situations in future)
};

#endif /* _SDL_nullvideo_h */
