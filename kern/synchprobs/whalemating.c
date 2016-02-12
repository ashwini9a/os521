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
static struct cv *allincv;
static struct lock *malelock;
static struct lock *femalelock;
static struct lock *matchmakerlock;
static struct lock *semlock;
static struct semaphore *allinsem;

void whalemating_init() {
	allincv = cv_create("allincv");
	if (allincv == NULL) {
		panic("allincv create failed in matingwhale.c \n");
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
	semlock = lock_create("semlock");
	if (semlock == NULL) {
		panic("semlock create failed in matingwhale.c \n");
	}
	allinsem = sem_create("allinsem" , 3);
	if (allinsem == NULL) {
		panic("could not create allinsem in matingwhale.c");
	}
	return;
}

/*
 * Called by the driver during teardown.
 */

void
whalemating_cleanup() {
	cv_destroy(allincv);
	lock_destroy(malelock);
	lock_destroy(femalelock);
	lock_destroy(matchmakerlock);
	lock_destroy(semlock);
	sem_destroy(allinsem);
	return;
}

void
male(uint32_t index)
{
	(void)index;
	// Lock so that only one male is in the semaphore
	lock_acquire(malelock);
	//decrease semaphore count.....1 male is in
	P(allinsem);
	//Check semaphore to see if everyone in in
	lock_acquire(semlock);
	//If everyone is in broadcast and start else wait for everyone...
	if (allinsem->sem_count==0) {
		cv_broadcast(allincv , semlock);
	}
	else{
		cv_wait(allincv , semlock);	
	}
	male_start(index);
	lock_release(semlock);
	//Increase semaphore count to signal male has started
	V(allinsem);
	//acquire another lock to check if everyone has started
	lock_acquire(semlock);
	//if sem count is 3 again, everyone has started. and now can exit
	if (allinsem->sem_count!=3) {
		cv_wait(allincv, semlock);
	}
	else {
		cv_broadcast(allincv,semlock);
	}
	lock_release(semlock);
	male_end(index);
	lock_release(malelock);
	return;
}

void
female(uint32_t index)
{
	(void)index;
	// Lock so that only one female is in the semaphore
	lock_acquire(femalelock);
	//decrease semaphore count.....1 female is in
	P(allinsem);
	//Check semaphore to see if everyone in in
	lock_acquire(semlock);
	//If everyone is in broadcast and start else wait for everyone...
	if (allinsem->sem_count==0) {
		cv_broadcast(allincv , semlock);
	}
	else{
		cv_wait(allincv , semlock);	
	}
	female_start(index);
	lock_release(semlock);
	//Increase semaphore count to signal female has started
	V(allinsem);
	//acquire another lock to check if everyone has started
	lock_acquire(semlock);
	//if sem count is 3 again, everyone has started. and now can exit
	if (allinsem->sem_count!=3) {
		cv_wait(allincv, semlock);
	}
	else {
		cv_broadcast(allincv,semlock);
	}
	lock_release(semlock);
	female_end(index);
	lock_release(femalelock);
	return;
}

void
matchmaker(uint32_t index)
{
	(void)index;
	// Lock so that only one matchmaker is in the semaphore
	lock_acquire(matchmakerlock);
	//decrease semaphore count.....1 matchmaker is in
	P(allinsem);
	//Check semaphore to see if everyone in in
	lock_acquire(semlock);
	//If everyone is in broadcast and start else wait for everyone...
	if (allinsem->sem_count==0) {
		cv_broadcast(allincv , semlock);
	}
	else{
		cv_wait(allincv , semlock);	
	}
	matchmaker_start(index);
	lock_release(semlock);
	//Increase semaphore count to signal male has started
	V(allinsem);
	//acquire another lock to check if everyone has started
	lock_acquire(semlock);
	//if sem count is 3 again, everyone has started. and now can exit
	if (allinsem->sem_count!=3) {
		cv_wait(allincv, semlock);
	}
	else {
		cv_broadcast(allincv,semlock);
	}
	lock_release(semlock);
	matchmaker_end(index);
	lock_release(matchmakerlock);
	return;
}
