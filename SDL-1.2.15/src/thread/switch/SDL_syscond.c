/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2014 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
//#include "../../SDL_internal.h"

/* An implementation of condition variables using semaphores and mutexes */
/*
   This implementation borrows heavily from the BeOS condition variable
   implementation, written by Christopher Tate and Owen Smith.  Thanks!
 */
/* New libnx fix from iyenal */

#include "SDL_config.h"

#include "SDL_thread.h"

#include <switch.h>

struct SDL_cond
{
	CondVar cond;
	Mutex mutex;
};

struct SDL_mutex 
{
	Mutex mutex;
};

SDL_cond *
SDL_CreateCond(void)
{
    SDL_cond *cond;
    Mutex *mutex;

    cond = (SDL_cond *) SDL_malloc(sizeof(*cond));
    if (!cond) {
        SDL_OutOfMemory();
    }
    return (cond);
}

void
SDL_DestroyCond(SDL_cond * cond)
{
    if (cond) {
        SDL_free(cond);
    }
}

int
SDL_CondSignal(SDL_cond * cond)
{
    if (!cond) {
        SDL_SetError("Passed a NULL condition variable");
        return -1;
    }
    condvarWakeOne(&cond->cond);
    return 0;
}

int
SDL_CondBroadcast(SDL_cond * cond)
{
    if (!cond) {
        SDL_SetError("Passed a NULL condition variable");
        return -1;
    }

    condvarWakeAll(&cond->cond);
    return 0;
}
 
int
SDL_CondWaitTimeout(SDL_cond * cond, SDL_mutex * mutex, Uint32 ms)
{
    int retval;

    if (!cond) {
        SDL_SetError("Passed a NULL condition variable");
        return -1;
    }

     condvarInit(&cond->cond);

    SDL_UnlockMutex(mutex);

    retval =  condvarWaitTimeout(&cond->cond, &mutex->mutex, (ms == SDL_MUTEX_MAXWAIT) ? U64_MAX : (signed long long)ms*1000000);

    SDL_LockMutex(mutex);

    return retval;
}

int
SDL_CondWait(SDL_cond * cond, SDL_mutex * mutex)
{
   int retval;

    if (!cond) {
        SDL_SetError("Passed a NULL condition variable");
        return -1;
    }

    condvarInit(&cond->cond);
	
    SDL_UnlockMutex(mutex);

    retval =  condvarWait(&cond->cond, &mutex->mutex);

    SDL_LockMutex(mutex);

    return retval;
}
