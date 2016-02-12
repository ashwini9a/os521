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
struct cv *malecv;
struct cv *femalecv;
struct cv *matchmakercv;
struct lock *malecountlock;
struct lock *femalecountlock;
struct lock *matchmakercountlock;
static int malecount,femalecount,matchmakercount;


void whalemating_init() {
	malecount = 0;
	femalecount = 0;
	matchmakercount = 0;
	malecv = cv_create("malecv");
	if (malecv == NULL) {
		panic("malecv create failed in matingwhale.c \n");
	}
	femalecv = cv_create("femalecv");
	if (femalecv == NULL) {
		panic("femalecv create failed in matingwhale.c \n");
	}
	matchmakercv = cv_create("matchmakercv");	
	if (matchmakercv == NULL) {
		panic("matchmakercv create failed in matingwhale.c \n");
	}
	malecountlock = lock_create("malecountlock");
	if (malecountlock == NULL) {
		panic("malecountlock create failed in matingwhale.c \n");
	}
	
	femalecountlock = lock_create("femalecountlock");
	if (femalecountlock == NULL) {
		panic("femalecountlock create failed in matingwhale.c \n");
	}
	matchmakercountlock = lock_create("matchmakercountlock");
	if (matchmakercountlock == NULL) {
		panic("matchmakercountlock create failed in matingwhale.c \n");
	}
	return;
}

/*
 * Called by the driver during teardown.
 */

void
whalemating_cleanup() {
	cv_destroy(malecv);
	cv_destroy(femalecv);
	cv_destroy(matchmakercv);
	lock_destroy(malecountlock);
	lock_destroy(femalecountlock);
	lock_destroy(matchmakercountlock);
	return;
}

void
male(uint32_t index)
{
	(void)index;
	/*
	 * Implement this function by calling male_start and male_end when
	 * appropriate.
	 */
	lock_acquire(malecountlock);
	malecount++;
	cv_signal(femalecv,malecountlock);
	cv_signal(matchmakercv,malecountlock);
	while (femalecount == 0 || matchmakercount == 0) {
		cv_wait(malecv , malecountlock);
	}
	KASSERT(matchmakercount>0);
	KASSERT(femalecount>0);
	male_start(index);
	malecount--;
	male_end(index);
	lock_release(malecountlock);
//	female_start();
//	matchmaker_start();
	return;
}

void
female(uint32_t index)
{
	(void)index;
	/*
	 * Implement this function by calling female_start and female_end when
	 * appropriate.
	 */
	lock_acquire(femalecountlock);
	femalecount++;
	cv_signal(malecv,femalecountlock);
	cv_signal(matchmakercv,femalecountlock);
	while (malecount == 0 || matchmakercount == 0) {
		cv_wait(femalecv , femalecountlock);
	}
	KASSERT(matchmakercount>0);
	KASSERT(malecount>0);
	female_start(index);
	femalecount--;
	female_end(index);
	lock_release(femalecountlock);
	return;
}

void
matchmaker(uint32_t index)
{
	(void)index;
	/*
	 * Implement this function by calling matchmaker_start and matchmaker_end
	 * when appropriate.
	 */
	lock_acquire(matchmakercountlock);
	matchmakercount++;
	cv_signal(malecv,matchmakercountlock);
	cv_signal(femalecv,matchmakercountlock);
	while (femalecount == 0 || malecount == 0) {
		cv_wait(matchmakercv , matchmakercountlock);
	}
	KASSERT(femalecount>0);
	KASSERT(malecount>0);
	matchmaker_start(index);
	matchmakercount--;
	matchmaker_end(index);
	lock_release(matchmakercountlock);
	return;
}
