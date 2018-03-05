#include "SDL_config.h"

#include <sys/time.h>
#include <switch.h>
#include <sys/select.h>


#include "SDL_thread.h"
#include "SDL_timer.h"
#include "SDL_error.h"
#include "../SDL_timer_c.h"

static struct timeval start;

void SDL_StartTicks (void) {
	gettimeofday (&start, NULL);
}

Uint32 SDL_GetTicks (void) {
//	return (u32)(svcGetSystemTick()>>32);

	Uint32 ticks;
	struct timeval now;

	gettimeofday (&now, NULL);
	ticks = (now.tv_sec - start.tv_sec) * 1000 + (now.tv_usec - start.tv_usec) / 1000;

	return (ticks);
}

void SDL_Delay (Uint32 ms) {
#ifdef SDL_THREAD_SWITCH
//	if (threadGetCurrent != NULL)
	if (SDL_ThreadID()>0) // Don't use svcSleepThread for main process. Ddirty and could not work every time. Find a betetr way. maybe using a const value if exist
		svcSleepThread((Uint64)ms * 1000000);
	else {
#endif
	int was_error;
	struct timeval tv;
	Uint32 then, now, elapsed;

	then = SDL_GetTicks();

	do {
		now = SDL_GetTicks ();
		elapsed = (now - then);
		then = now;
		if (elapsed >= ms) {
			break;
		}
		ms -= elapsed;

		tv.tv_sec = ms / 1000;
		tv.tv_usec = (ms % 1000) * 1000;

		was_error = select(0, NULL, NULL, NULL, &tv);
	} while (was_error);
#ifdef SDL_THREAD_SWITCH
	}
#endif
}

int SDL_SYS_TimerInit (void) {
	return 0;
}

void SDL_SYS_TimerQuit (void) {
}

int SDL_SYS_StartTimer (void) {
	SDL_SetError ("Timers not implemented on Switch");

	return -1;
}

void SDL_SYS_StopTimer (void) {
}
