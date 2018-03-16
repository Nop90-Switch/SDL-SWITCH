/*

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

	audoutExit();

	SDL_free(device->hidden);
	SDL_free(device);
}

static SDL_AudioDevice *SWITCH_CreateDevice(int devindex)
{
	SDL_AudioDevice *this;

	if(audoutInitialize()) return (0);

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

}

static void SWITCH_PlayAudio(_THIS)
{
    if(	this->hidden->next > this->hidden->released_count)
		audoutWaitPlayFinish(&this->hidden->released_buffer, &this->hidden->released_count, U64_MAX);
	audoutAppendAudioOutBuffer(&this->hidden->waveBuf[this->hidden->next%NUM_BUFFERS]);
	this->hidden->next++;
}

static Uint8 *SWITCH_GetAudioBuf(_THIS)
{
	return this->hidden->waveBuf[this->hidden->next%NUM_BUFFERS].buffer;
}

static void SWITCH_CloseAudio(_THIS)
{
	int i;
	audoutStopAudioOut();
	
	for(i=0; i<NUM_BUFFERS; i++)
	{
		if 	( this->hidden->waveBuf[i].buffer!= NULL )
			free(this->hidden->waveBuf[i].buffer);
		this->hidden->waveBuf[i].buffer= NULL;
	}
}

static int SWITCH_OpenAudio(_THIS, SDL_AudioSpec *spec)
{	
	int i;
	if(audoutStartAudioOut()) return (-1);

	spec->channels = 2;
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
			case AUDIO_S16LSB:
				/* Signed 16-bit audio supported */
				this->hidden->format=PcmFormat_Int16;
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
        return (-1);
    }

	/* Update the fragment size as size in bytes */
	SDL_CalculateAudioSpec(spec);

	/* Allocate mixing buffer */
	this->hidden->mixlen = spec->size;
	
	int size_aligned = (spec->size + 0xfff) & ~0xfff;
	
	for(i=0; i<NUM_BUFFERS; i++) 
	{
		this->hidden->waveBuf[i].buffer = memalign(0x1000,size_aligned);
		this->hidden->waveBuf[i].buffer_size = size_aligned;
		this->hidden->waveBuf[i].data_size = spec->size;
		this->hidden->waveBuf[i].next = NULL;
		this->hidden->waveBuf[i].data_offset = 0;
		memset(this->hidden->waveBuf[i].buffer,0,size_aligned);
    }
	this->hidden->released_count=0;
	this->hidden->next = 0;

//	this->hidden->channels = spec->channels;
//	this->hidden->samplerate = spec->freq;
	
	for(i=0; i<NUM_BUFFERS; i++) 
	{
		audoutAppendAudioOutBuffer(&this->hidden->waveBuf[i]);
	}
	// lets wait for the first buffer to be released 
	audoutWaitPlayFinish(&this->hidden->released_buffer, &this->hidden->released_count, U64_MAX);
	
	/* We're ready to rock and roll. :-) */
	return(0);
}
