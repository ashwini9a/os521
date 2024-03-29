/*
 * Copyright (c) 2001, 2002, 2009
 *	The President and Fellows of Harvard College.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Driver code is in kern/tests/synchprobs.c We will
 * replace that file. This file is yours to modify as you see fit.
 *
 * You should implement your solution to the whalemating problem below.
 */

#include <types.h>
#include <lib.h>
#include <thread.h>
#include <test.h>
#include <synch.h>

/*
 * Called by the driver during initialization.
 */
struct semaphore *malesem;
struct semaphore *femalesem;
struct semaphore *matchmakersem;
struct lock *malelock; 
struct lock *femalelock; 
struct lock *matchmakerlock; 
void whalemating_init() {
	malesem = sem_create("malesem" , 0);
	if(malesem == NULL) {
		panic("malesem create failed in matingwhale.c \n");
	}

	femalesem = sem_create("femalesem" , 0);
	if(femalesem == NULL) {
		panic("femalesem create failed in matingwhale.c \n");
	}
	matchmakersem = sem_create("matchmakersem" , 0);
	if(matchmakersem == NULL) {
		panic("matchmakersem create failed in matingwhale.c \n");
	}
	malelock = lock_create("malelock");
	if (malelock == NULL) {
		panic("malelock create failed in matingwhale.c \n");
	}
	femalelock = lock_create("femalelock");
	if (femalelock == NULL) {
		panic("femalelock create failed in matingwhale.c \n");
	}
	matchmakerlock = lock_create("matchmakerlock");
	if (matchmakerlock == NULL) {
		panic("matchmakerlock create failed in matingwhale.c \n");
	}
	return;
}

/*
 * Called by the driver during teardown.
 */

void
whalemating_cleanup() {
	sem_destroy(malesem);
	sem_destroy(femalesem);
	sem_destroy(matchmakersem);
	lock_destroy(malelock);
	lock_destroy(femalelock);
	lock_destroy(matchmakerlock);
	return;
}

void
male(uint32_t index)
{
	male_start(index);
	lock_acquire(malelock);
	V(malesem);
	V(malesem);
	P(femalesem);
	P(matchmakersem);
	lock_release(malelock);
	male_end(index);
	return;
}

void
female(uint32_t index)
{
	female_start(index);
	lock_acquire(femalelock);
	V(femalesem);
	V(femalesem);
	P(malesem);
	P(matchmakersem);
	lock_release(femalelock);
	female_end(index);
	return;
}

void
matchmaker(uint32_t index)
{
	matchmaker_start(index);
	lock_acquire(matchmakerlock);
	V(matchmakersem);
	V(matchmakersem);
	P(femalesem);
	P(malesem);
	lock_release(matchmakerlock);
	matchmaker_end(index);
	return;
}
