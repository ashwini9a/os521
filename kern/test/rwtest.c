/*
 * All the contents of this file are overwritten during automated
 * testing. Please consider this before changing anything in this file.
 */

#include <types.h>
#include <lib.h>
#include <clock.h>
#include <thread.h>
#include <synch.h>
#include <test.h>
#include <kern/secret.h>
#include <spinlock.h>


#define NTHREADS 32

static bool status = FAIL;
struct rwlock * rwlock;
int readcount;
int writecount;
struct semaphore *donesem;
struct lock *lock;

static void init() {
	rwlock = rwlock_create("rwlock");
	if (rwlock == NULL) {
		panic("rwlock create failed \n");
	}
	donesem = sem_create("donesem" , 0);
	if (donesem == NULL) {
		panic("donesem create failed \n");
	}
	readcount = 0;
	writecount = 0;
	lock = lock_create("lock");
	if (lock == NULL) {
		panic("lock create failed \n");
	}
	return;	
}

static void cleanup() {
	rwlock_destroy(rwlock);
	lock_destroy(lock);
	return;
}

static void rwtestthread(void *junk, unsigned long num) {
	(void)junk;
	//int i;
	random_yielder(4);
	if (num % 3 == 0) {
		rwlock_acquire_write(rwlock);
//		lock_acquire(lock);
		writecount++;
		kprintf("Write lock acquired... Writers:%d  Readers:%d \n",writecount, readcount);
		writecount--;
		random_yielder(4);
		writecount++;
		writecount--;
		rwlock_release_write(rwlock);
		kprintf("Write lock released... Writers:%d  Readers:%d \n",writecount, readcount);
//		lock_release(lock);

	}
	else {
		rwlock_acquire_read(rwlock);
//		lock_acquire(lock);
		readcount++;
		kprintf("read lock acquired... Writers:%d  Readers:%d \n",writecount, readcount);
		readcount--;
		random_yielder(4);
		readcount++;
		readcount--;
		rwlock_release_read(rwlock);
		kprintf("read lock released... Writers:%d  Readers:%d \n",writecount, readcount);
//		lock_release(lock);
	}
	V(donesem);
}

/*
 * Use these stubs to test your reader-writer locks.
 */


int rwtest(int nargs, char **args) {
	(void)nargs;
	(void)args;

	int i,result;
	init();
	status = SUCCESS;
	for (i=0; i<NTHREADS; i++) {
		result = thread_fork("rwtest", NULL, rwtestthread, NULL, i);
		if (result) {
			panic("rw thread fork failed %s \n" , strerror(result));
		}
	}
	
	for (i=0;i<NTHREADS;i++) {
		P(donesem);
	}
	
	cleanup();
	kprintf_n("rwt1 unimplemented\n");
	success(status, SECRET, "rwt1");

	return 0;
}

int rwtest2(int nargs, char **args) {
	(void)nargs;
	(void)args;

	kprintf_n("rwt2 unimplemented\n");
	success(FAIL, SECRET, "rwt2");

	return 0;
}

int rwtest3(int nargs, char **args) {
	(void)nargs;
	(void)args;

	kprintf_n("rwt3 unimplemented\n");
	success(FAIL, SECRET, "rwt3");

	return 0;
}

int rwtest4(int nargs, char **args) {
	(void)nargs;
	(void)args;

	kprintf_n("rwt4 unimplemented\n");
	success(FAIL, SECRET, "rwt4");

	return 0;
}

int rwtest5(int nargs, char **args) {
	(void)nargs;
	(void)args;

	kprintf_n("rwt5 unimplemented\n");
	success(FAIL, SECRET, "rwt5");

	return 0;
}
