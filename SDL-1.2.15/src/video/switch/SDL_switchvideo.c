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

static Uint32 switch_palette[256] = {0};


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
int SWITCH_ToggleFullScreen(_THIS, int on);


void SWITCH_SetResolution(_THIS)
{
	float rx,ry;
	rx = 1280.0/this->hidden->w;
	ry = 720.0/this->hidden->h;
	if(rx>ry) rx=ry;
	gfxConfigureResolution(1280/rx, 720/rx);
	this->hidden->pw=1280/rx;
	this->hidden->ph=720/rx;
	gfxSetMode(GfxMode_TiledDouble);
}

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
	
	device->ToggleFullScreen = SWITCH_ToggleFullScreen;	

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
//	gfxConfigureAutoResolutionDefault(true);
	vformat->BitsPerPixel = 32;
	vformat->BytesPerPixel = 4;
	vformat->Rmask = 0x000000ff;
	vformat->Gmask = 0x0000ff00;
	vformat->Bmask = 0x00ff0000; 
	vformat->Amask = 0xff000000; 

	/* We're done! */
	return(0);
}

SDL_Rect **SWITCH_ListModes(_THIS, SDL_PixelFormat *format, Uint32 flags)
{
   	 return (SDL_Rect **) -1;
}

int SWITCH_ToggleFullScreen(_THIS, int on){
	if ( this->hidden->flags & SDL_FULLSCREEN ) {
		this->hidden->flags &= ~SDL_FULLSCREEN;
		gfxConfigureResolution(0, 0);
		gfxSetMode(GfxMode_LinearDouble);
		this->hidden->pw=1280;
		this->hidden->ph=720;
	} else {
		this->hidden->flags |= SDL_FULLSCREEN;
		SWITCH_SetResolution(this);
	}
	return 1;
}

SDL_Surface *SWITCH_SetVideoMode(_THIS, SDL_Surface *current,
				int width, int height, int bpp, Uint32 flags)
{
	Uint32 Rmask, Gmask, Bmask, Amask; 

	switch(bpp) {
		case 0:
			bpp = 32;
		case 32:
			Rmask = 0x000000ff; 
			Gmask = 0x0000ff00;
			Bmask = 0x00ff0000;
			Amask = 0xff000000;
			break;
		case 24:
			Rmask = 0x0000ff; 
			Gmask = 0x00ff00;
			Bmask = 0xff0000;
			Amask = 0x0;
			break;
		case 16:
			Rmask = 0xF800;
            Gmask = 0x07E0;
            Bmask = 0x001F;
            Amask = 0x0000;
			break;
		case 8:
			Rmask = 0;
			Gmask = 0;
			Bmask = 0;
			Amask = 0;
			break;
		default:
			return NULL;
			break;
	}

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

	/* Set up the new mode framebuffer */
	this->hidden->flags = current->flags =  SDL_HWSURFACE | SDL_DOUBLEBUF | (flags & SDL_FULLSCREEN);
//	this->hidden->flags = current->flags =  flags;
	this->info.current_w = this->hidden->w = current->w = width;
	this->info.current_h = this->hidden->h = current->h = height;
	this->hidden->bpp = bpp;
	current->pitch = current->w * (bpp / 8);
	current->pixels = this->hidden->buffer;

	/* Allocate the new pixel format for the screen */
	if ( ! SDL_ReallocFormat(current, bpp, Rmask, Gmask, Bmask, Amask) ) {
		SDL_free(this->hidden->buffer);
		this->hidden->buffer = NULL;
		SDL_SetError("Couldn't allocate new pixel format for requested mode");
		return(NULL);
	}

	this->hidden->pw=1280;
	this->hidden->ph=720;
	
	if (flags & SDL_FULLSCREEN) SWITCH_SetResolution(this);

	// lets clead the phisical video buffer, in case we are changing to a smaller screen
	u32 w, h;
	u32* framebuf = (u32*) gfxGetFramebuffer((u32*)&w, (u32*)&h);
	memset(framebuf,0,w*h*4);
	
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
	SWITCH_FlipHWSurface (this, NULL);

} 

int SWITCH_SetColors(_THIS, int firstcolor, int ncolors, SDL_Color *colors)
{
	int i;
	for ( i = firstcolor; i < firstcolor + ncolors; ++i )
		switch_palette[i] = RGBA8(colors[i].r, colors[i].g, colors[i].b, 0xff);
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
	gfxConfigureResolution(0, 0);
	gfxExit();
}

static int SWITCH_FlipHWSurface (_THIS, SDL_Surface *surface) {
	u32 width, height;
	u32 pos_src, pos_dst;
	u32* framebuf = (u32*) gfxGetFramebuffer((u32*)&width, (u32*)&height);
	u8* videobuf = (u8*) this->hidden->buffer;
	u16* videobuf_16 = (u16*) this->hidden->buffer;

	u32 x, y, offx, offy;

	if(width > this->info.current_w)
		offx = (width - this->info.current_w) / 2;
	else 	
		offx = 0;

	if(height > this->info.current_h)
		offy = (height - this->info.current_h) / 2;
	else 	
		offy = 0;
		
	if(this->hidden->flags & SDL_FULLSCREEN) {

		pos_src = 0;
				
		switch(this->hidden->bpp) {
		
			case 32:
				for (y=0; y<((height< this->info.current_h)?height:this->info.current_h); y++)//Access the buffer linearly.
				{
					for (x=0; x<((width < this->info.current_w)?width:this->info.current_w); x++)
					{
						framebuf[gfxGetFramebufferDisplayOffset(x+offx,y+offy)] = RGBA8(videobuf[pos_src+0], videobuf[pos_src+1], videobuf[pos_src+2], videobuf[pos_src+3]);
						pos_src += 4;
					}
					pos_src = pos_src - x*4 + this->info.current_w*4;
				}
				break;
			case 24:
				for (y=0; y<((height< this->info.current_h)?height:this->info.current_h); y++)//Access the buffer linearly.
				{
					for (x=0; x<((width < this->info.current_w)?width:this->info.current_w); x++)
					{
						framebuf[gfxGetFramebufferDisplayOffset(x+offx,y+offy)] = RGBA8(videobuf[pos_src+0], videobuf[pos_src+1], videobuf[pos_src+2], 0xff);
						pos_src += 3;
					}
					pos_src = pos_src - x*3 + this->info.current_w*3;
				}
				break;
			case 16:
				for (y=0; y<((height< this->info.current_h)?height:this->info.current_h); y++)//Access the buffer linearly.
				{
					for (x=0; x<((width < this->info.current_w)?width:this->info.current_w); x++)
					{
						framebuf[gfxGetFramebufferDisplayOffset(x+offx,y+offy)] = RGBA8((videobuf_16[pos_src]&0xF800)>>8,(videobuf_16[pos_src]&0x07E0)>>3, (videobuf_16[pos_src]&0x001F)<<3,0xff);
						pos_src++;
					}
					pos_src = pos_src - x + this->info.current_w;
				}

				break;
			case 8:
				for (y=0; y<((height< this->info.current_h)?height:this->info.current_h); y++)//Access the buffer linearly.
				{
					for (x=0; x<((width < this->info.current_w)?width:this->info.current_w); x++)
					{
						framebuf[gfxGetFramebufferDisplayOffset(x+offx,y+offy)] = switch_palette[videobuf[pos_src]];
						pos_src++;
					}
					pos_src = pos_src - x + this->info.current_w;
				}
				break;
			default:
				break;
		}
	
	} else {
	
		pos_dst = offy * width + offx;
		pos_src = 0;

				
		switch(this->hidden->bpp) {
		
			case 32:
				for (y=0; y<((height< this->info.current_h)?height:this->info.current_h); y++)//Access the buffer linearly.
				{
					for (x=0; x<((width < this->info.current_w)?width:this->info.current_w); x++)
					{
						framebuf[pos_dst] = RGBA8(videobuf[pos_src+0], videobuf[pos_src+1], videobuf[pos_src+2], videobuf[pos_src+3]);
						pos_dst++;
						pos_src += 4;
					}
					pos_dst = pos_dst -x + width;
					pos_src = pos_src - x*4 + this->info.current_w*4;
				}
				break;
			case 24:
				for (y=0; y<((height< this->info.current_h)?height:this->info.current_h); y++)//Access the buffer linearly.
				{
					for (x=0; x<((width < this->info.current_w)?width:this->info.current_w); x++)
					{
						framebuf[pos_dst] = RGBA8(videobuf[pos_src+0], videobuf[pos_src+1], videobuf[pos_src+2], 0xff);
						pos_dst++;
						pos_src += 3;
					}
					pos_dst = pos_dst - x + width;
					pos_src = pos_src - x*3 + this->info.current_w*3;
				}
				break;
			case 16:
				for (y=0; y<((height< this->info.current_h)?height:this->info.current_h); y++)//Access the buffer linearly.
				{
					for (x=0; x<((width < this->info.current_w)?width:this->info.current_w); x++)
					{
						framebuf[pos_dst] = RGBA8((videobuf_16[pos_src]&0xF800)>>8,(videobuf_16[pos_src]&0x07E0)>>3, (videobuf_16[pos_src]&0x001F)<<3,0xff);
						pos_dst++;
						pos_src++;
					}
					pos_dst = pos_dst - x + width;
					pos_src = pos_src - x + this->info.current_w;
				}

				break;
			case 8:
				for (y=0; y<((height< this->info.current_h)?height:this->info.current_h); y++)//Access the buffer linearly.
				{
					for (x=0; x<((width < this->info.current_w)?width:this->info.current_w); x++)
					{
						framebuf[pos_dst] = switch_palette[videobuf[pos_src]];
						pos_dst++;
						pos_src++;
					}
					pos_dst = pos_dst - x + width;
					pos_src = pos_src - x + this->info.current_w;
				}
				break;
			default:
				break;
		}

	}
	
	gfxFlushBuffers();
	gfxSwapBuffers();
	gfxWaitForVsync();

	return (0);
}
