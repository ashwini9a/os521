#ifndef _FILE_H_
#define _FILE_H_

#include <types.h>
#include <thread.h>
#include <current.h>
#include <kern/errno.h>
#include <limits.h>
#include <vfs.h>
#include <vnode.h>
#include <synch.h>
#include <kern/fcntl.h>
#include <copyinout.h>
#include <proc.h>
struct vnode;
struct lock;


struct filehandle {
	char *filename;
	int flags;
	int offset;
	int refcount;
	struct lock  *filelock;
	struct vnode *vnode;
};

int sys_open(const_userptr_t filename, int flags, mode_t mode, int *returnvalue);
int filedescriptor_init(void);


#endif
