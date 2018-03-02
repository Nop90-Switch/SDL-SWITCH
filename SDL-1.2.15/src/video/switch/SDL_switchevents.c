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

/* Being a null driver, there's no event stream. We just define stubs for
   most of the API. */

#include "switch.h"
#include "../../events/SDL_sysevents.h"
#include "../../events/SDL_events_c.h"

#include "SDL_switchvideo.h"
#include "SDL_switchevents_c.h"

static SDLKey keymap[SWITCH_NUMKEYS];
char keymem[SWITCH_NUMKEYS];

void SWITCH_PumpEvents(_THIS)
{
	if(!appletMainLoop())
	{
		SDL_Event sdlevent;
		sdlevent.type = SDL_QUIT;
		SDL_PushEvent(&sdlevent);
	} 
	
	int i;
	u32 keys;
	SDL_keysym keysym;
	keysym.mod = KMOD_NONE;

	hidScanInput();
	keys = hidKeysHeld(CONTROLLER_P1_AUTO);

	for (i = 0; i < SWITCH_NUMKEYS; i++) {
		keysym.scancode = i;
		keysym.sym = keymap[i];

		if (keys & (1 << i) && !keymem[i]) {
			keymem[i] = 1;

			SDL_PrivateKeyboard (SDL_PRESSED, &keysym);
		}

		if (!(keys & (1 << i)) && keymem[i]) {
			keymem[i] = 0;

			SDL_PrivateKeyboard (SDL_RELEASED, &keysym);
		}
	}
	
	if (hidKeysHeld(CONTROLLER_P1_AUTO) & KEY_TOUCH) {
			
		touchPosition touch;

		hidTouchRead (&touch,0);
		
		int xtouch, ytouch;
		
		xtouch = (touch.px / 1280.0) * this->hidden->pw - (this->hidden->pw-this->hidden->w)/2;
		if (xtouch >= this->hidden->w) xtouch = -1;

		ytouch = (touch.py / 720.0) * this->hidden->ph - (this->hidden->ph-this->hidden->h)/2;
		if (ytouch > this->hidden->h) ytouch = -1;
		
		if (xtouch >= 0 && xtouch >= 0 ) {
			SDL_PrivateMouseMotion (0, 0, xtouch, ytouch);
			if (!SDL_GetMouseState (NULL, NULL))
				SDL_PrivateMouseButton (SDL_PRESSED, 1, 0, 0);
		}
	} else {
		if (SDL_GetMouseState (NULL, NULL))
			SDL_PrivateMouseButton (SDL_RELEASED, 1, 0, 0);
	}
}

void SWITCH_InitOSKeymap(_THIS)
{
	SDL_memset(keymem,1,SWITCH_NUMKEYS);
	keymap[0]=SDLK_a; //KEY_A
	keymap[1]=SDLK_b; // KEY_B
	keymap[2]=SDLK_x; //KEY_X 
	keymap[3]=SDLK_y; //KEY_Y
	keymap[4]=SDLK_LSHIFT;  //left stick button 
	keymap[5]=SDLK_RSHIFT;  //right stick button
	keymap[6]=SDLK_l; //KEY_L
	keymap[7]=SDLK_r; //KEY_R
	keymap[8]=SDLK_j; //KEY_ZL
	keymap[9]=SDLK_k; //KEY_ZR
	keymap[10]=SDLK_MINUS; //KEY_MINUS 
	keymap[11]=SDLK_PLUS; //KEY_PLUS
	keymap[12]=SDLK_LEFT; //KEY_LEFT
	keymap[13]=SDLK_UP; // KEY_UP
	keymap[14]=SDLK_RIGHT; //KEY_RIGHT
	keymap[15]=SDLK_DOWN; //KEY_DOWN
	keymap[16]=SDLK_UNKNOWN; 
	keymap[17]=SDLK_UNKNOWN; 
	keymap[18]=SDLK_UNKNOWN; 
	keymap[19]=SDLK_UNKNOWN; 
	keymap[20]=SDLK_UNKNOWN; 
	keymap[21]=SDLK_UNKNOWN; 
	keymap[22]=SDLK_UNKNOWN; 
	keymap[23]=SDLK_UNKNOWN; 
	keymap[24]=SDLK_UNKNOWN; 
	keymap[25]=SDLK_UNKNOWN; 
	keymap[26]=SDLK_UNKNOWN; 
	keymap[27]=SDLK_UNKNOWN; 
	keymap[28]=SDLK_UNKNOWN; 
	keymap[29]=SDLK_UNKNOWN; 
	keymap[30]=SDLK_UNKNOWN; 
	keymap[31]=SDLK_UNKNOWN; 

// init the key state
	int i;
	hidScanInput();
	for (i = 0; i < SWITCH_NUMKEYS; i++)
		keymem[i] = (hidKeysHeld(CONTROLLER_P1_AUTO) & (1 << i))?1:0;
		
}

/* end of SDL_nullevents.c ... */

