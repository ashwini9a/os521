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
 * Driver code is in kern/tests/synchprobs.c We will replace that file. This
 * file is yours to modify as you see fit.
 *
 * You should implement your solution to the stoplight problem below. The
 * quadrant and direction mappings for reference: (although the problem is, of
 * course, stable under rotation)
 *
 *   |0 |
 * -     --
 *    01  1
 * 3  32
 * --    --
 *   | 2|
 *
 * As way to think about it, assuming cars drive on the right: a car entering
 * the intersection from direction X will enter intersection quadrant X first.
 * The semantics of the problem are that once a car enters any quadrant it has
 * to be somewhere in the intersection until it call leaveIntersection(),
 * which it should call while in the final quadrant.
 *
 * As an example, let's say a car approaches the intersection and needs to
 * pass through quadrants 0, 3 and 2. Once you call inQuadrant(0), the car is
 * considered in quadrant 0 until you call inQuadrant(3). After you call
 * inQuadrant(2), the car is considered in quadrant 2 until you call
 * leaveIntersection().
 *
 * You will probably want to write some helper functions to assist with the
 * mappings. Modular arithmetic can help, e.g. a car passing straight through
 * the intersection entering from direction X will leave to direction (X + 2)
 * % 4 and pass through quadrants X and (X + 3) % 4.  Boo-yah.
 *
 * Your solutions below should call the inQuadrant() and leaveIntersection()
 * functions in synchprobs.c to record their progress.
 */

#include <types.h>
#include <lib.h>
#include <thread.h>
#include <test.h>
#include <synch.h>

/*
 * Called by the driver during initialization.
 */


static struct lock *quad0; 
static struct lock *quad1;
static struct lock *quad2;
static struct lock *quad3;
static struct lock *acqlock;

void
stoplight_init() {
	quad0 = lock_create("quad0");
        if (quad0 == NULL) {
                panic("quad0: lock_create failed\n");
        }
	quad1 = lock_create("quad1");
        if (quad1 == NULL) {
                panic("quad1: lock_create failed\n");
        }
	quad2 = lock_create("quad2");
        if (quad2 == NULL) {
                panic("quad2: lock_create failed\n");
        }
        quad3 = lock_create("quad3");
        if (quad3 == NULL) {
                panic("quad3: lock_create failed\n");
        }
	acqlock = lock_create("acqlock");
        if (acqlock == NULL) {
                panic("acqlock: lock_create failed\n");
        }


	return;
}

/*
 * Called by the driver during teardown.
 */

void stoplight_cleanup() {
	lock_destroy(quad0);
	lock_destroy(quad1);
	lock_destroy(quad2);
	lock_destroy(quad3);
	lock_destroy(acqlock);
	return;
}

void
turnright(uint32_t direction, uint32_t index)
{
//	(void)direction;
//	(void)index;
	kprintf("\n Car: %d goin to: turn right in quad: %d ",index,direction);	
	switch(direction)
	{
		case 0:
			lock_acquire(acqlock);
			lock_acquire(quad0);
			lock_release(acqlock);
			inQuadrant(0,index);
			leaveIntersection(index);
			lock_release(quad0);		
//			kprintf("\n Lock released 0");
			break;

		case 1:
			lock_acquire(acqlock);
                        lock_acquire(quad1);
			lock_release(acqlock);
                        inQuadrant(1,index);        
                        leaveIntersection(index);
                        lock_release(quad1);
//			kprintf("\n Lock released 1");
			break;

		case 2:
			lock_acquire(acqlock);
                        lock_acquire(quad2);
			lock_release(acqlock);
                        inQuadrant(2,index);        
                        leaveIntersection(index);
                        lock_release(quad2);
//			kprintf("\n Lock released 2");
			break;

		case 3:
			lock_acquire(acqlock);
                        lock_acquire(quad3);
			lock_release(acqlock);
                        inQuadrant(3,index);        
                        leaveIntersection(index);
                        lock_release(quad3);
//			kprintf("\n Lock released 3");
			break;
		
		default:
			kprintf("\n Wrong direction for car: turn right !!!");
	}
	/*
	 * Implement this function.
	 */
	return;
}
void
gostraight(uint32_t direction, uint32_t index)
{
//	(void)direction;
//	(void)index;
//	kprintf("\n Car: %d goin to:%d ",index,direction);
	kprintf("\n Car: %d goin to: go straight in quad: %d ",index,direction);
        switch(direction)
        {
                case 0:
			lock_acquire(acqlock);
                        lock_acquire(quad0);
			lock_acquire(quad3);
			lock_release(acqlock);
                        inQuadrant(0,index);  
			inQuadrant(3,index);
			lock_release(quad0);
//			kprintf("\n Lock released 0");
			leaveIntersection(index);
			lock_release(quad3);
//			kprintf("\n Lock released 3");
                        break;

                case 1:
			lock_acquire(acqlock);
              		lock_acquire(quad1);
                        lock_acquire(quad0);
			lock_release(acqlock);
                        inQuadrant(1,index);
                        inQuadrant(0,index);
                        lock_release(quad1);
                        leaveIntersection(index);
                        lock_release(quad0);
                        break;

                case 2:
			lock_acquire(acqlock);
                        lock_acquire(quad2);
                        lock_acquire(quad1);
			lock_release(acqlock);
                        inQuadrant(2,index);
                        inQuadrant(1,index);
                        lock_release(quad2);
                        leaveIntersection(index);
                        lock_release(quad1);
                        break;

                case 3:
			lock_acquire(acqlock);
                        lock_acquire(quad3);
                        lock_acquire(quad2);
			lock_release(acqlock);
                        inQuadrant(3,index);
                        inQuadrant(2,index);
                        lock_release(quad3);
                        leaveIntersection(index);
                        lock_release(quad2);
                        break;

                default:
                        kprintf("\n Wrong direction for car: go straight !!!");
        }

	/*
	 * Implement this function.
	 */
	return;
}
void
turnleft(uint32_t direction, uint32_t index)
{
//	(void)direction;
//	(void)index;
//	kprintf("\n Car: %d goin to:%d ",index,direction);
	kprintf("\n Car: %d goin to: turn left in quad: %d ",index,direction);
	switch(direction)
        {
                case 0:
			lock_acquire(acqlock);
                        lock_acquire(quad0);
                        lock_acquire(quad3);
			lock_acquire(quad2);
			lock_release(acqlock);
                        inQuadrant(0,index);
                        inQuadrant(3,index);
                        lock_release(quad0);
			inQuadrant(2,index);
			lock_release(quad3);
                        leaveIntersection(index);
                        lock_release(quad2);
                        break;

                case 1:
			lock_acquire(acqlock);
                        lock_acquire(quad1);
                        lock_acquire(quad0);
                        lock_acquire(quad3);
			lock_release(acqlock);
                        inQuadrant(1,index);
                        inQuadrant(0,index);
                        lock_release(quad1);
                        inQuadrant(3,index);
                        lock_release(quad0);
                        leaveIntersection(index);
                        lock_release(quad3);
                        break;
		case 2:
			lock_acquire(acqlock);
                        lock_acquire(quad2);
                        lock_acquire(quad1);
                        lock_acquire(quad0);
			lock_release(acqlock);
                        inQuadrant(2,index);
                        inQuadrant(1,index);
                        lock_release(quad2);
                        inQuadrant(0,index);
                        lock_release(quad1);
                        leaveIntersection(index);
                        lock_release(quad0);
                        break;
		case 3:
			lock_acquire(acqlock);
 			lock_acquire(quad3);
                        lock_acquire(quad2);
                        lock_acquire(quad1);
			lock_release(acqlock);
                        inQuadrant(3,index);
                        inQuadrant(2,index);
                        lock_release(quad3);
                        inQuadrant(1,index);
                        lock_release(quad2);
                        leaveIntersection(index);
                        lock_release(quad1);
                        break;
		default:
			kprintf("\n Wrong direction: turn left, %d",direction);

	}
	/*
	 * Implement this function.
	 */
	return;
}
