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
#include <kern/unistd.h>
#include <kern/seek.h>
#include <stat.h>
#include <uio.h>

struct vnode;
struct lock;


struct filehandle {
	char *filename;
	int flags;
	off_t offset;
	int refcount;
	struct lock  *filelock;
	struct vnode *vnode;
};

int sys_open(const_userptr_t filename, int flags, mode_t mode, int *returnvalue);
int filedescriptor_init(void);
int sys_lseek(int fd, off_t pos, int whence, int *returnvalue);
struct filehandle* getfileHandle(int fd);
bool isFdReadValid(int fd);
bool isFdWriteValid(int fd);
int sys_read(int fd, void *buf, size_t buflen, int *returnvalue);
int sys_write(int fd, const void *buf, size_t nbytes, int *returnvalue);
int sys_close(int fd);
#endif
