/*
 * Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2008, 2009
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
 * Synchronization primitives.
 * The specifications of the functions are in synch.h.
 */

#include <types.h>
#include <lib.h>
#include <spinlock.h>
#include <wchan.h>
#include <thread.h>
#include <current.h>
#include <synch.h>
#include<spl.h>
////////////////////////////////////////////////////////////
//
// Semaphore.

struct semaphore *
sem_create(const char *name, unsigned initial_count)
{
	struct semaphore *sem;

	sem = kmalloc(sizeof(*sem));
	if (sem == NULL) {
		return NULL;
	}

	sem->sem_name = kstrdup(name);
	if (sem->sem_name == NULL) {
		kfree(sem);
		return NULL;
	}

	sem->sem_wchan = wchan_create(sem->sem_name);
	if (sem->sem_wchan == NULL) {
		kfree(sem->sem_name);
		kfree(sem);
		return NULL;
	}

	spinlock_init(&sem->sem_lock);
	sem->sem_count = initial_count;

	return sem;
}

void
sem_destroy(struct semaphore *sem)
{
	KASSERT(sem != NULL);

	/* wchan_cleanup will assert if anyone's waiting on it */
	spinlock_cleanup(&sem->sem_lock);
	wchan_destroy(sem->sem_wchan);
	kfree(sem->sem_name);
	kfree(sem);
}

void
P(struct semaphore *sem)
{
	KASSERT(sem != NULL);

	/*
	 * May not block in an interrupt handler.
	 *
	 * For robustness, always check, even if we can actually
	 * complete the P without blocking.
	 */
	KASSERT(curthread->t_in_interrupt == false);

	/* Use the semaphore spinlock to protect the wchan as well. */
	spinlock_acquire(&sem->sem_lock);
	while (sem->sem_count == 0) {
		/*
		 *
		 * Note that we don't maintain strict FIFO ordering of
		 * threads going through the semaphore; that is, we
		 * might "get" it on the first try even if other
		 * threads are waiting. Apparently according to some
		 * textbooks semaphores must for some reason have
		 * strict ordering. Too bad. :-)
		 *
		 * Exercise: how would you implement strict FIFO
		 * ordering?
		 */
		wchan_sleep(sem->sem_wchan, &sem->sem_lock);
	}
	KASSERT(sem->sem_count > 0);
	sem->sem_count--;
	spinlock_release(&sem->sem_lock);
}

void
V(struct semaphore *sem)
{
	KASSERT(sem != NULL);

	spinlock_acquire(&sem->sem_lock);

	sem->sem_count++;
	KASSERT(sem->sem_count > 0);
	wchan_wakeone(sem->sem_wchan, &sem->sem_lock);

	spinlock_release(&sem->sem_lock);
}

////////////////////////////////////////////////////////////
//
// Lock.

struct lock *
lock_create(const char *name)
{
	struct lock *lock;

	lock = kmalloc(sizeof(*lock));
	if (lock == NULL) {
		return NULL;
	}

	lock->lk_name = kstrdup(name);
	if (lock->lk_name == NULL) {
		kfree(lock);
		return NULL;
	}
	lock->lock_wchan = wchan_create(lock->lk_name);
	if (lock->lock_wchan == NULL) {
	kfree(lock->lk_name);  
		kfree(lock);
		return NULL;           
	}

	lock->lock_thread = NULL;
	//To check if it has been acquired or not
	lock->acquired = false;

	spinlock_init(&lock->lock_spinlock);

	return lock;
}

void
lock_destroy(struct lock *lock)
{
	KASSERT(lock != NULL);
	KASSERT(!lock->acquired);
	// add stuff here as needed

	/* wchan_cleanup will assert if anyone's waiting on it */
	spinlock_cleanup(&lock->lock_spinlock);
	wchan_destroy(lock->lock_wchan);

	kfree(lock->lk_name);
	kfree(lock->lock_thread);
	kfree(lock);
}

void
lock_acquire(struct lock *lock)
{
	//make sure the thread is not an interrupt thread
	KASSERT(curthread->t_in_interrupt == false);

	/* Use the lock spinlock to protect the wchan as well. */
	spinlock_acquire(&lock->lock_spinlock);
	//sleep if someone else has the lock
	while (lock->acquired) {

		wchan_sleep(lock->lock_wchan, &lock->lock_spinlock);
	}
	// make sure the lock is released
	KASSERT(lock->acquired==false);
	lock->acquired = true;
	lock->lock_thread = curthread;
	//free the spinlock
	spinlock_release(&lock->lock_spinlock);
}

void
lock_release(struct lock *lock)
{

	KASSERT(lock != NULL);
	KASSERT(lock_do_i_hold(lock));
	spinlock_acquire(&lock->lock_spinlock);

	lock->acquired = false;
	lock->lock_thread = NULL;
	
	wchan_wakeone(lock->lock_wchan, &lock->lock_spinlock);

	spinlock_release(&lock->lock_spinlock);

//	(void)lock;  // suppress warning until code gets written
}

bool
lock_do_i_hold(struct lock *lock)
{
	KASSERT(lock != NULL);

	// Check if I own the lock
	if (lock->lock_thread == curthread)
		return true;
	else
		return false;

}

////////////////////////////////////////////////////////////
//
// CV


struct cv *
cv_create(const char *name)
{
	struct cv *cv;

	cv = kmalloc(sizeof(*cv));
	if (cv == NULL) {
		return NULL;
	}

	cv->cv_name = kstrdup(name);
	if (cv->cv_name==NULL) {
		kfree(cv);
		return NULL;
	}

	// add stuff here as needed

	cv->cv_wchan = wchan_create(cv->cv_name);
	if (cv->cv_wchan == NULL) {
	kfree(cv->cv_name);  
		kfree(cv);
		return NULL;           
	}

	spinlock_init(&cv->cv_spinlock);
	return cv;
}

void
cv_destroy(struct cv *cv)
{
	KASSERT(cv != NULL);
	spinlock_cleanup(&cv->cv_spinlock);
	wchan_destroy(cv->cv_wchan);
	kfree(cv->cv_name);
	kfree(cv);
}

void
cv_wait(struct cv *cv, struct lock *lock)
{
	KASSERT(cv!=NULL);
	KASSERT(lock!=NULL);
	KASSERT(lock_do_i_hold(lock));
	spinlock_acquire(&cv->cv_spinlock);
	lock_release(lock);		
	wchan_sleep(cv->cv_wchan, &cv->cv_spinlock);
	spinlock_release(&cv->cv_spinlock);
	lock_acquire(lock);

}

void
cv_signal(struct cv *cv, struct lock *lock)
{
	KASSERT(cv!=NULL);
	KASSERT(lock!=NULL);
	KASSERT(lock_do_i_hold(lock));
	spinlock_acquire(&cv->cv_spinlock);
	wchan_wakeone(cv->cv_wchan, &cv->cv_spinlock);
	spinlock_release(&cv->cv_spinlock);
}

void
cv_broadcast(struct cv *cv, struct lock *lock)
{
	KASSERT(lock_do_i_hold(lock));
	KASSERT(cv!=NULL);
	KASSERT(lock!=NULL);
	spinlock_acquire(&cv->cv_spinlock);
	wchan_wakeall(cv->cv_wchan, &cv->cv_spinlock);
	spinlock_release(&cv->cv_spinlock);	
}


struct rwlock * rwlock_create(const char *name)
{
		
	struct rwlock *rwlock;

	rwlock = kmalloc(sizeof(*rwlock));
	if (rwlock == NULL) {
		return NULL;
	}

	rwlock->rwlock_name = kstrdup(name);
	if (rwlock->rwlock_name==NULL) {
		kfree(rwlock);
		return NULL;
	}

	rwlock->rw_wchan = wchan_create(rwlock->rwlock_name);
	if (rwlock->rw_wchan == NULL) {
		kfree(rwlock->rwlock_name);  
		kfree(rwlock);
		return NULL;           
	}

	rwlock->write_wchan = wchan_create("write_wchan");
	if (rwlock->write_wchan == NULL) {
		kfree(rwlock);
		return NULL;
	}
	
	//rcount signifies the number of readers and can be greater than 1
	//wcount signifies the number of writers and has a max limit of 1
	rwlock->rcount = 0;
	rwlock->wcount = 0;
	rwlock->wreq = 0;
	rwlock->rreq = 0;
	spinlock_init(&rwlock->rw_spinlock);
	return rwlock;
}

void rwlock_destroy(struct rwlock * rwlock) 
{
	
	KASSERT(rwlock != NULL);
	KASSERT(rwlock->rcount == 0);
	KASSERT(rwlock->wcount == 0);
	KASSERT(rwlock->wreq == 0);
	// add stuff here as needed

	spinlock_cleanup(&rwlock->rw_spinlock);
	wchan_destroy(rwlock->rw_wchan);
	wchan_destroy(rwlock->write_wchan);
	kfree(rwlock->rwlock_name);
//	kfree(rwlock->rcount);
//	kfree(rwlock->wcount);
	kfree(rwlock);
}

void rwlock_acquire_read(struct rwlock *rwlock) 
{
		
	KASSERT(curthread->t_in_interrupt == false);
	/* Use the lock spinlock to protect the wchan as well. */
	spinlock_acquire(&rwlock->rw_spinlock);
	//sleep if a writer has the lock
	rwlock->rreq++;
	if (rwlock->rreq <3) {
		while (rwlock->wcount > 0 || rwlock->wreq > 0) {
	
			wchan_sleep(rwlock->rw_wchan, &rwlock->rw_spinlock);
		}
	}
	else {
		while(rwlock->wcount > 0) {
			wchan_sleep(rwlock->rw_wchan, &rwlock->rw_spinlock);
		}
	}
	KASSERT(rwlock->wcount == 0);
	rwlock->rcount++;
	rwlock->rreq--;
	//rwlock->rw_thread = curthread;
	//free the spinlock
	spinlock_release(&rwlock->rw_spinlock);
	
}

void rwlock_release_read(struct rwlock *rwlock)
{
	

	KASSERT(rwlock != NULL);
//	KASSERT(lock_do_i_hold(lock));
	spinlock_acquire(&rwlock->rw_spinlock);
	KASSERT(rwlock->rcount>0);
	rwlock->rcount--;
	if (rwlock->rcount == 0)	
		wchan_wakeall(rwlock->rw_wchan, &rwlock->rw_spinlock);

	spinlock_release(&rwlock->rw_spinlock);
} 

void rwlock_acquire_write(struct rwlock *rwlock)
{
	KASSERT(curthread->t_in_interrupt == false);
	spinlock_acquire(&rwlock->rw_spinlock);
//	while(rwlock->wreq >0) {
//		wchan_sleep(rwlock->write_wchan, &rwlock->rw_spinlock);
//	}
	rwlock->wreq++;
	while(rwlock->wcount>0 || rwlock->rcount>0)
	{
		wchan_sleep(rwlock->rw_wchan,&rwlock->rw_spinlock );
	}
	KASSERT(rwlock->rcount == 0 && rwlock->wcount == 0);
	rwlock->wcount++;	
	rwlock->wreq--;
	//wchan_wakeall(rwlock->rw_wchan, &rwlock->rw_spinlock);
//	wchan_wakeone(rwlock->write_wchan,&rwlock-> rw_spinlock);
	spinlock_release(&rwlock->rw_spinlock);
}

void rwlock_release_write(struct rwlock *rwlock)
{
	KASSERT(rwlock!=NULL);
	spinlock_acquire(&rwlock->rw_spinlock);
	KASSERT(rwlock->wcount > 0);
	rwlock->wcount--;
	wchan_wakeall(rwlock->rw_wchan,&rwlock->rw_spinlock);
	spinlock_release(&rwlock->rw_spinlock);
}
