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

    This file written by Ryan C. Gordon (icculus@icculus.org)
*/
#include "SDL_config.h"

/* Output audio to nowhere... */

#include "SDL_rwops.h"
#include "SDL_timer.h"
#include "SDL_audio.h"
#include "../SDL_audiomem.h"
#include "../SDL_audio_c.h"
#include "../SDL_audiodev_c.h"
#include "SDL_switchaudio.h"

#include <switch.h>
#include <malloc.h>

//extern volatile bool app_pause;
extern volatile bool app_exiting;

size_t stream_offset = 0;

/* The tag name used by SWITCH audio */
#define SWITCHAUD_DRIVER_NAME         "switch"

/* Audio driver functions */
static int SWITCH_OpenAudio(_THIS, SDL_AudioSpec *spec);
static void SWITCH_WaitAudio(_THIS);
static void SWITCH_PlayAudio(_THIS);
static Uint8 *SWITCH_GetAudioBuf(_THIS);
static void SWITCH_CloseAudio(_THIS);

/* Audio driver bootstrap functions */
static int SWITCH_Available(void)
{
	return(1);
}

static void SWITCH_DeleteDevice(SDL_AudioDevice *device)
{
	if ( device->hidden->mixbuf != NULL ) {
		SDL_FreeAudioMem(device->hidden->mixbuf);
		device->hidden->mixbuf = NULL;
	}
	if 	( device->hidden->waveBuf!= NULL )
		free(device->hidden->waveBuf);
	audoutStopAudioOut();
	audoutExit();
	SDL_free(device->hidden);
	SDL_free(device);
}

static SDL_AudioDevice *SWITCH_CreateDevice(int devindex)
{
	SDL_AudioDevice *this;

	/* Initialize all variables that we clean on shutdown */
	this = (SDL_AudioDevice *)SDL_malloc(sizeof(SDL_AudioDevice));
	if ( this ) {
		SDL_memset(this, 0, (sizeof *this));
		this->hidden = (struct SDL_PrivateAudioData *)
				SDL_malloc((sizeof *this->hidden));
	}
	if ( (this == NULL) || (this->hidden == NULL) ) {
		SDL_OutOfMemory();
		if ( this ) {
			SDL_free(this);
		}
		return(0);
	}
	SDL_memset(this->hidden, 0, (sizeof *this->hidden));

	/* Set the function pointers */
	this->OpenAudio = SWITCH_OpenAudio;
	this->WaitAudio = SWITCH_WaitAudio;
	this->PlayAudio = SWITCH_PlayAudio;
	this->GetAudioBuf = SWITCH_GetAudioBuf;
	this->CloseAudio = SWITCH_CloseAudio;
	this->free = SWITCH_DeleteDevice;

	return this;
}

AudioBootStrap SWITCHAUD_bootstrap = {
	SWITCHAUD_DRIVER_NAME, "SDL Switch audio driver",
	SWITCH_Available, SWITCH_CreateDevice
};

/* This function waits until it is possible to write a full sound buffer */
static void SWITCH_WaitAudio(_THIS)
{
	audoutWaitPlayFinish(&this->hidden->waveBuf[this->hidden->nextbuf], U64_MAX);
}

static void SWITCH_PlayAudio(_THIS)
{
	
	if (!appletMainLoop()) return; 

	memcpy(this->hidden->waveBuf[this->hidden->nextbuf].buffer,this->hidden->mixbuf,this->hidden->mixlen);

	audoutAppendAudioOutBuffer(&this->hidden->waveBuf[this->hidden->nextbuf]);
	this->hidden->nextbuf = (this->hidden->nextbuf+1)%2;
}

static Uint8 *SWITCH_GetAudioBuf(_THIS)
{
	return(this->hidden->mixbuf);
}

static void SWITCH_CloseAudio(_THIS)
{
	if ( this->hidden->mixbuf != NULL ) {
		SDL_FreeAudioMem(this->hidden->mixbuf);
		this->hidden->mixbuf = NULL;
	}
	if 	( this->hidden->waveBuf!= NULL )
		free(this->hidden->waveBuf);
}

static int SWITCH_OpenAudio(_THIS, SDL_AudioSpec *spec)
{	
	if(audoutInitialize()) return (-1);
	if(audoutStartAudioOut()) return (-1);

	int format = 0;
	spec->channels = 2;
	spec->size = (spec->size + 0xfff) & ~0xfff;
	spec->freq = 48000;

    Uint16 test_format = SDL_FirstAudioFormat(spec->format);
    int valid_datatype = 0;
    while ((!valid_datatype) && (test_format)) {
        spec->format = test_format;
        switch (test_format) {
/* 
			case AUDIO_S8:
				// Signed 8-bit audio actually unsupported 
				this->hidden->format=PcmFormat_INT8;
//				this->hidden->isSigned=1;
				this->hidden->bytePerSample = (spec->channels);
				   valid_datatype = 1;
				break;
*/
			case AUDIO_S16:
				/* Signed 16-bit audio supported */
				this->hidden->format=PcmFormat_INT16;
//				this->hidden->isSigned=1;
				this->hidden->bytePerSample = (spec->channels) * 2;
				   valid_datatype = 1;
				break;
			default:
				test_format = SDL_NextAudioFormat();
				break;
		}
	}

    if (!valid_datatype) {  /* shouldn't happen, but just in case... */
        SDL_SetError("Unsupported audio format");
		audoutStopAudioOut();
		audoutExit();
        return (-1);
    }

	/* Update the fragment size as size in bytes */
	SDL_CalculateAudioSpec(spec);

	/* Allocate mixing buffer */
	this->hidden->mixlen = spec->size;
	this->hidden->mixbuf = (Uint8 *) SDL_malloc(spec->size); 
	if ( this->hidden->mixbuf == NULL ) {
		audoutStopAudioOut();
		audoutExit();
		return(-1);
	}
	SDL_memset(this->hidden->mixbuf, spec->silence, spec->size);
	
    this->hidden->waveBuf[0].buffer = memalign(0x1000,this->hidden->mixlen);
    this->hidden->waveBuf[0].buffer_size = spec->size;
    this->hidden->waveBuf[0].data_size = spec->size*this->hidden->bytePerSample;
    this->hidden->waveBuf[1].buffer = memalign(0x1000,this->hidden->mixlen);
    this->hidden->waveBuf[1].buffer_size = spec->size;
    this->hidden->waveBuf[1].data_size = spec->size*this->hidden->bytePerSample;
	
	this->hidden->nextbuf = 0;
	this->hidden->channels = spec->channels;
	this->hidden->samplerate = spec->freq;

	memset(this->hidden->waveBuf,0,sizeof(AudioOutBuffer)*2);




	/* We're ready to rock and roll. :-) */
	return(0);
}
