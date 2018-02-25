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
#include <switch.h>

/* SWITCH SDL video driver implementation; 
 */

#include "SDL_video.h"
#include "SDL_mouse.h"
#include "../SDL_sysvideo.h"
#include "../SDL_pixels_c.h"
#include "../../events/SDL_events_c.h"

#include "SDL_switchvideo.h"
#include "SDL_switchevents_c.h"
#include "SDL_switchmouse_c.h"

#define SWITCHVID_DRIVER_NAME "switch"

/* Initialization/Query functions */
static int SWITCH_VideoInit(_THIS, SDL_PixelFormat *vformat);
static SDL_Rect **SWITCH_ListModes(_THIS, SDL_PixelFormat *format, Uint32 flags);
static SDL_Surface *SWITCH_SetVideoMode(_THIS, SDL_Surface *current, int width, int height, int bpp, Uint32 flags);
static int SWITCH_SetColors(_THIS, int firstcolor, int ncolors, SDL_Color *colors);
static void SWITCH_VideoQuit(_THIS);

/* Hardware surface functions */
static int SWITCH_AllocHWSurface(_THIS, SDL_Surface *surface);
static int SWITCH_LockHWSurface(_THIS, SDL_Surface *surface);
static void SWITCH_UnlockHWSurface(_THIS, SDL_Surface *surface);
static void SWITCH_FreeHWSurface(_THIS, SDL_Surface *surface);
static int SWITCH_FlipHWSurface (_THIS, SDL_Surface *surface);

/* etc. */
static void SWITCH_UpdateRects(_THIS, int numrects, SDL_Rect *rects);

/* SWITCH driver bootstrap functions */

static int SWITCH_Available(void)
{
	return(1);
}

static void SWITCH_DeleteDevice(SDL_VideoDevice *device)
{
	SDL_free(device->hidden);
	SDL_free(device);
	device=NULL;
}

static SDL_VideoDevice *SWITCH_CreateDevice(int devindex)
{
	SDL_VideoDevice *device;

	/* Initialize all variables that we clean on shutdown */
	device = (SDL_VideoDevice *)SDL_malloc(sizeof(SDL_VideoDevice));
	if ( device ) {
		SDL_memset(device, 0, (sizeof *device));
		device->hidden = (struct SDL_PrivateVideoData *)
				SDL_malloc((sizeof *device->hidden));
	}
	if ( (device == NULL) || (device->hidden == NULL) ) {
		SDL_OutOfMemory();
		if ( device ) {
			SDL_free(device);
		}
		return(0);
	}
	SDL_memset(device->hidden, 0, (sizeof *device->hidden));

	/* Set the function pointers */
	device->VideoInit = SWITCH_VideoInit;
	device->ListModes = SWITCH_ListModes;
	device->SetVideoMode = SWITCH_SetVideoMode;
	device->CreateYUVOverlay = NULL;
	device->SetColors = SWITCH_SetColors;
	device->UpdateRects = SWITCH_UpdateRects;
	device->VideoQuit = SWITCH_VideoQuit;
	device->AllocHWSurface = SWITCH_AllocHWSurface;
	device->CheckHWBlit = NULL;
	device->FillHWRect = NULL;
	device->SetHWColorKey = NULL;
	device->SetHWAlpha = NULL;
	device->LockHWSurface = SWITCH_LockHWSurface;
	device->UnlockHWSurface = SWITCH_UnlockHWSurface;
	device->FlipHWSurface = SWITCH_FlipHWSurface;
	device->FreeHWSurface = SWITCH_FreeHWSurface;
	device->SetCaption = NULL;
	device->SetIcon = NULL;
	device->IconifyWindow = NULL;
	device->GrabInput = NULL;
	device->GetWMInfo = NULL;
	device->InitOSKeymap = SWITCH_InitOSKeymap;
	device->PumpEvents = SWITCH_PumpEvents;

	device->free = SWITCH_DeleteDevice;

	return device;
}

VideoBootStrap SWITCH_bootstrap = {
	SWITCHVID_DRIVER_NAME, "SDL Switch video driver",
	SWITCH_Available, SWITCH_CreateDevice
};


int SWITCH_VideoInit(_THIS, SDL_PixelFormat *vformat)
{
	gfxInitDefault();
	vformat->BitsPerPixel = 32;
	vformat->BytesPerPixel = 4;
	vformat->Rmask = 0xff000000;
	vformat->Gmask = 0x00ff0000;
	vformat->Bmask = 0x0000ff00; 
	vformat->Amask = 0x000000ff; 

	/* We're done! */
	return(0);
}

SDL_Rect **SWITCH_ListModes(_THIS, SDL_PixelFormat *format, Uint32 flags)
{
   	 return (SDL_Rect **) -1;
}

SDL_Surface *SWITCH_SetVideoMode(_THIS, SDL_Surface *current,
				int width, int height, int bpp, Uint32 flags)
{
	Uint32 Rmask, Gmask, Bmask, Amask; 
	
	if ( this->hidden->buffer ) {
		SDL_free( this->hidden->buffer );
	}

	this->hidden->buffer = SDL_malloc(width * height * (bpp / 8));
	if ( ! this->hidden->buffer ) {
		SDL_SetError("Couldn't allocate buffer for requested mode");
		return(NULL);
	}

/* 	printf("Setting mode %dx%d\n", width, height); */

	SDL_memset(this->hidden->buffer, 0, width * height * (bpp / 8));

	Rmask = 0xff000000; 
	Gmask = 0x00ff0000;
	Bmask = 0x0000ff00;
	Amask = 0x000000ff;

	/* Allocate the new pixel format for the screen */
	if ( ! SDL_ReallocFormat(current, bpp, Rmask, Gmask, Bmask, Amask) ) {
		SDL_free(this->hidden->buffer);
		this->hidden->buffer = NULL;
		SDL_SetError("Couldn't allocate new pixel format for requested mode");
		return(NULL);
	}

	/* Set up the new mode framebuffer */
	current->flags =  SDL_HWSURFACE | SDL_DOUBLEBUF;
	this->info.current_w = this->hidden->w = current->w = width;
	this->info.current_h = this->hidden->h = current->h = height;
	current->pitch = current->w * (bpp / 8);
	current->pixels = this->hidden->buffer;
	
	/* We're done */
	return(current);
}

/* We don't actually allow hardware surfaces other than the main one */
static int SWITCH_AllocHWSurface(_THIS, SDL_Surface *surface)
{
	return(-1);
}

static void SWITCH_FreeHWSurface(_THIS, SDL_Surface *surface)
{
	return;
}

/* We need to wait for vertical retrace on page flipped displays */
static int SWITCH_LockHWSurface(_THIS, SDL_Surface *surface)
{
	return(0);
}

static void SWITCH_UnlockHWSurface(_THIS, SDL_Surface *surface)
{
	return;
}

static void SWITCH_UpdateRects(_THIS, int numrects, SDL_Rect *rects)
{
	u32 width, height;
	u32 pos_src, pos_dst;
	u32* framebuf = (u32*) gfxGetFramebuffer((u32*)&width, (u32*)&height);
	u8* videobuf = (u8*) this->hidden->buffer;

	u32 x, y;

	for (y=0; y<((height< this->info.current_h)?height:this->info.current_h); y++)//Access the buffer linearly.
	{
		for (x=0; x<((width < this->info.current_w)?width:this->info.current_w); x++)
		{
			pos_dst = y * width + x;
			pos_src = (y * this->info.current_w + x)*4;
			framebuf[pos_dst] = RGBA8(videobuf[pos_src+0], videobuf[pos_src+1], videobuf[pos_src+2], videobuf[pos_src+3]);
		}
	}

	gfxFlushBuffers();
	gfxSwapBuffers();
	gfxWaitForVsync();

}

int SWITCH_SetColors(_THIS, int firstcolor, int ncolors, SDL_Color *colors)
{
	/* do nothing of note. */
	return(1);
}

/* Note:  If we are terminated, this could be called in the middle of
   another SDL video routine -- notably UpdateRects.
*/
void SWITCH_VideoQuit(_THIS)
{
	if (this->hidden->buffer != NULL)
	{
		SDL_free(this->hidden->buffer);
		this->hidden->buffer = NULL;
	}
	gfxExit();
}

static int SWITCH_FlipHWSurface (_THIS, SDL_Surface *surface) {
	u32 width, height;
	u32 pos_src, pos_dst;
	u32* framebuf = (u32*) gfxGetFramebuffer((u32*)&width, (u32*)&height);
	u8* videobuf = (u8*) this->hidden->buffer;

	u32 x, y;

	for (y=0; y<((height< this->info.current_h)?height:this->info.current_h); y++)//Access the buffer linearly.
	{
		for (x=0; x<((width < this->info.current_w)?width:this->info.current_w); x++)
		{
			pos_dst = y * width + x;
			pos_src = (y * this->info.current_w + x)*4;
			framebuf[pos_dst] = RGBA8(videobuf[pos_src+0], videobuf[pos_src+1], videobuf[pos_src+2], videobuf[pos_src+3]);
		}
	}

	gfxFlushBuffers();
	gfxSwapBuffers();
	gfxWaitForVsync();

	return (0);
}
