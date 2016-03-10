#include <file.h>

int sys_open(const_userptr_t file, int flags, mode_t mode, int *returnvalue) {
	int index;
	char *filename = kmalloc(PATH_MAX+1);
	struct vnode *vnode;
	off_t offset = 0;
	size_t length;
	int result;
	
	if (filename == NULL) {
//		kprintf("kmalloc failed to  assign space for filename in file.c\n");
		return ENOSPC;
	}
	result =  copyinstr(file, filename, PATH_MAX, &length);
	if (result) {
		return result;
	}

//	kprintf("filename according to kernel is %s:\n",filename);
	index = 3;
	while(curproc->filedescriptor[index]!=NULL) {
		index++;
	}

	if (index == OPEN_MAX) {
//		kprintf("Can't open more files. Close some");
		filename = NULL;
		kfree(filename);
		return EMFILE;
	}

	curproc->filedescriptor[index] =(struct filehandle*)kmalloc(sizeof(struct filehandle));
	if (curproc->filedescriptor[index] == NULL) {
//		kprintf("could not allocate memory to filehandle");
		filename = NULL;
		kfree(filename);
		return ENFILE;
	}
	result = vfs_open(filename, flags, mode, &vnode);
	if (result) {
//		kprintf("file open failed");
		filename = NULL;
		kfree(filename);
		kfree(curproc->filedescriptor[index]);
		return result;
	}
	curproc->filedescriptor[index]->filename = filename;
	curproc->filedescriptor[index]->flags = flags;
 	curproc->filedescriptor[index]->offset = offset;
	curproc->filedescriptor[index]->refcount = 1;
	curproc->filedescriptor[index]->filelock = lock_create(filename);
	curproc->filedescriptor[index]->vnode = vnode;

//	filedescriptor[index] = filehandle;
	filename = NULL;
	kfree(filename);
	*returnvalue = index;
	return 0;
}

int filedescriptor_init(void) {
	
	char *filename;
	filename = kstrdup("con:");
	char *out = kstrdup("con:");
	char *err = kstrdup("con:");
	struct vnode *vn1;
	struct vnode *vn2;
	struct vnode *vn3;

	if (curproc->filedescriptor[0] == NULL) {
		curproc->filedescriptor[0] =(struct filehandle*) kmalloc(sizeof(struct filehandle));
		if (curproc->filedescriptor[0] == NULL) {
//			kprintf("could not allocate memory for std in");
			return ENFILE;
		}
	
		int result = vfs_open(filename, O_RDONLY, 0664, &vn1);
		if (result) {
//			kprintf("Console 0 open failed");
			return EINVAL;
		}

		curproc->filedescriptor[0]->filename = filename;
		curproc->filedescriptor[0]->flags = O_RDONLY;
		curproc->filedescriptor[0]->offset = 0;
		curproc->filedescriptor[0]->refcount = 1;
		curproc->filedescriptor[0]->filelock = lock_create("stdin_lock");
		curproc->filedescriptor[0]->vnode = vn1; 
	}

	if (curproc->filedescriptor[1] == NULL) {
		curproc->filedescriptor[1] = (struct filehandle*) kmalloc(sizeof(struct filehandle));
		if (curproc->filedescriptor[1] == NULL) {
//			kprintf("Could not allocate memory for std out");
			return ENFILE;
		}
		int result = vfs_open(out, O_WRONLY, 0664, &vn2);
		if (result) {
//			kprintf("could not open std output");
			return EINVAL;
		}

		curproc->filedescriptor[1]->filename = out;
		curproc->filedescriptor[1]->flags = O_WRONLY;
		curproc->filedescriptor[1]->offset = 0;
		curproc->filedescriptor[1]->refcount = 1;
		curproc->filedescriptor[1]->filelock = lock_create("stdout_lock");
		curproc->filedescriptor[1]->vnode = vn2;
	}

	if (curproc->filedescriptor[2] == NULL) {
		curproc->filedescriptor[2] = (struct filehandle*) kmalloc(sizeof(struct filehandle));
		if (curproc->filedescriptor[2] == NULL) {
//			kprintf("Could not allocate memory for std err");
			return ENFILE;
		}
		int result = vfs_open(err, O_WRONLY, 0664, &vn3);
		if (result) {
//			kprintf("could not open std err");
			return EINVAL;
		}

		curproc->filedescriptor[2]->filename = err;
		curproc->filedescriptor[2]->flags = O_WRONLY;
		curproc->filedescriptor[2]->offset = 0;
		curproc->filedescriptor[2]->refcount = 1;
		curproc->filedescriptor[2]->filelock = lock_create("stderr_lock");
		curproc->filedescriptor[2]->vnode = vn2;
	}
	return 0;
}

struct filehandle* getfileHandle(int fd)
{
	return curproc->filedescriptor[fd];
}

int sys_lseek(int fd, off_t pos, int whence, off_t *returnvalue) {
	off_t present_offset = 0;
	off_t new_pos = 0;
	struct stat temp;
	int result;

	if (fd < 0) {
		return EBADF;
	}
	
	if (fd == 0 || fd == 1 || fd == 2) {
//		kprintf("The user is mad.... Trying to seek in con :(\n");
		return ESPIPE;
	}
	if (fd >= OPEN_MAX || fd < 0) {
//		kprintf("Stupid malicious user... file descriptor\n");
		return EBADF;
	}

	if (curproc->filedescriptor[fd] == NULL) {
//		kprintf("Stupid malicious user...NULL\n");
		return EBADF;
	}
	lock_acquire(curproc->filedescriptor[fd]->filelock);
//	kprintf("offset is %llu\n" , pos);
	if (VOP_ISSEEKABLE(curproc->filedescriptor[fd]->vnode)) {
		present_offset = curproc->filedescriptor[fd]->offset;	
		switch(whence) {
	
			case SEEK_SET:
				new_pos = pos;
				if (new_pos <0) {
					
					lock_release(curproc->filedescriptor[fd]->filelock);
					return EINVAL;
				}
				break;
		
			case SEEK_CUR:	
				new_pos = present_offset + pos;
				if (new_pos < 0) {
					lock_release(curproc->filedescriptor[fd]->filelock);
					return EINVAL;
				}
				break;

			case SEEK_END:
				result = VOP_STAT(curproc->filedescriptor[fd]->vnode, &temp);
				if (result) {
//					kprintf("the kernel screwed up... couldn't vop_stat");
					lock_release(curproc->filedescriptor[fd]->filelock);
					return result;
				}
				present_offset = temp.st_size;
				new_pos = present_offset + pos;
				if (new_pos < 0) {
					lock_release(curproc->filedescriptor[fd]->filelock);
					return EINVAL;
				}
				break;

			default :
//				kprintf("whence is invalid");
				lock_release(curproc->filedescriptor[fd]->filelock);
				return EINVAL;
		}
	
		curproc->filedescriptor[fd]->offset = new_pos;
		*returnvalue =new_pos;
		lock_release(curproc->filedescriptor[fd]->filelock);
		return 0;
	}
	lock_release(curproc->filedescriptor[fd]->filelock);
	return ESPIPE;
}	


bool isFdValid(int fd)
{
	if (fd >= OPEN_MAX) {
		return false;
	}

	if(curproc->filedescriptor[fd]==NULL) {
		return false;
	}
	return true;
}



int sys_read(int fd, void *buf, size_t buflen, int *returnvalue)
{
        if(!(isFdValid(fd))) {
                *returnvalue = 0;
		return EBADF;
	}
	if (fd <0) {
		return EBADF;
	}
	if (curproc->filedescriptor[fd]->flags == (O_WRONLY | O_CREAT | O_TRUNC)||curproc->filedescriptor[fd]->flags == (O_WRONLY) || curproc->filedescriptor[fd]->flags == (O_WRONLY | O_EXCL | O_TRUNC) || curproc->filedescriptor[fd]->flags == (O_WRONLY | O_CREAT | O_APPEND) || curproc->filedescriptor[fd]->flags == (O_WRONLY | O_EXCL | O_APPEND) || curproc->filedescriptor[fd]->flags == (O_WRONLY | O_TRUNC) || curproc->filedescriptor[fd]->flags == (O_WRONLY |O_APPEND)) {
		return EBADF;
	}
        struct uio uiotemp;
	struct iovec iov;
	int result;
	char kbuf[buflen];

	lock_acquire(curproc->filedescriptor[fd]->filelock);
	uio_kinit(&iov, &uiotemp, kbuf, buflen, curproc->filedescriptor[fd]->offset,UIO_READ );

        result = VOP_READ(curproc->filedescriptor[fd]->vnode, &uiotemp);
	if(result) {
//		kprintf("Some read error occoured \n");
		*returnvalue = 0;
		lock_release(curproc->filedescriptor[fd]->filelock);
		return EIO;
	}
	curproc->filedescriptor[fd]->offset = uiotemp.uio_offset;
	result = copyout(kbuf, buf, buflen);
	if (result) {
		lock_release(curproc->filedescriptor[fd]->filelock);
		return EFAULT;
	}
	*returnvalue = buflen - uiotemp.uio_resid;
	lock_release(curproc->filedescriptor[fd]->filelock);

        return 0;
}
//ssize_t

int sys_write(int fd, const void *buf, size_t nbytes, int *returnvalue) {
        if(!(isFdValid(fd)))
                return EBADF;

	if (fd <0) {
		return EBADF;
	}

	if (curproc->filedescriptor[fd]->flags == O_RDONLY) {
		return EBADF;
	}

        struct uio uiotemp;
	struct iovec iov;
	int result;
	char kbuf[nbytes];
	lock_acquire(curproc->filedescriptor[fd]->filelock);
	result = copyin((const_userptr_t)buf, kbuf, nbytes);
	if (result) {
		lock_release(curproc->filedescriptor[fd]->filelock);
		return EFAULT;
	}

	uio_kinit(&iov, &uiotemp, kbuf, nbytes, curproc->filedescriptor[fd]->offset,UIO_WRITE );
        result = VOP_WRITE(curproc->filedescriptor[fd]->vnode,&uiotemp);
	if(result) {
		*returnvalue = 0;
		lock_release(curproc->filedescriptor[fd]->filelock);
		return EIO;
	}
	curproc->filedescriptor[fd]->offset = uiotemp.uio_offset;
	*returnvalue = nbytes - uiotemp.uio_resid;
	lock_release(curproc->filedescriptor[fd]->filelock);
        return 0;
}

int sys_close(int fd) {
//	struct filehandle fh;	
	if (fd >= OPEN_MAX) {
		return EBADF;
	}

//	fh = getfileHandle(fd);

	if (curproc->filedescriptor[fd] == NULL) {
		return EBADF;	
	}
	
	if (fd < 0) {
		return EBADF;
	} 

	curproc->filedescriptor[fd]->refcount--;

	if (curproc->filedescriptor[fd]->refcount == 0) {
		vfs_close(curproc->filedescriptor[fd]->vnode);
//		lock_release(curproc->filedescriptor[fd]->filelock);
		lock_destroy(curproc->filedescriptor[fd]->filelock);
		kfree(curproc->filedescriptor[fd]);
		curproc->filedescriptor[fd] = NULL;
	}
	return 0;
}


int sys_chdir (const_userptr_t directory) {
	char *newdir;
	size_t len;
	
	newdir = kmalloc(sizeof(PATH_MAX +1));
	if (newdir == NULL) {
//		kprintf("kmalloc failed for newdir");
		return EFAULT;
	}
	int result = copyinstr(directory, newdir, PATH_MAX, &len);
	if (result) {
//		kprintf("could not copy user specified path to kernel space\n");
		return result;
	}
	result = vfs_chdir(newdir); 
	if (result) {
//		kprintf("chdir failed");
		return result;
	}
	return 0;	
}
int sys_dup2(int oldfd, int newfd)
{
        int result;
	if(oldfd==newfd)
		return 0;
        if(!(isFdValid(oldfd)))
	{
		//*returnvalue = EBADF;
                return EBADF;
	}
	if(newfd > OPEN_MAX)
		//*returnvalue = EBADF;
		return EBADF;
	
        if(curproc->filedescriptor[newfd]!= NULL)
                result = sys_close(newfd);
        if(result)
	//	*returnvalue= result;
                return result;
	
	lock_acquire(curproc->filedescriptor[oldfd]->filelock);
        //curproc->filedescriptor[newfd] =(struct filehandle*) kmalloc(sizeof(struct filehandle));
        //curproc->filedescriptor[newfd]->filename = curproc->filedescriptor[oldfd]->filename;
        //curproc->filedescriptor[newfd]->flags = curproc->filedescriptor[oldfd]->flags;
        //curproc->filedescriptor[newfd]->offset = curproc->filedescriptor[oldfd]->offset;
        //curproc->filedescriptor[newfd]->refcount = curproc->filedescriptor[oldfd]->refcount+1;
	curproc->filedescriptor[newfd] = curproc->filedescriptor[oldfd];
	curproc->filedescriptor[newfd]->refcount++;
	//curproc->filedescriptor[newfd]->vnode->vn_refcount++;
        //curproc->filedescriptor[newfd]->filelock = curproc->filedescriptor[oldfd]->filelock;
        //curproc->filedescriptor[newfd]->vnode = curproc->filedescriptor[oldfd]->vnode;
	lock_release(curproc->filedescriptor[oldfd]->filelock);
        return 0;
}

int sys__getcwd(void *buf, size_t buflen)
{
	int result;
//	if(buf == NULL)
//		return EFAULT;
	struct uio uiotemp;
        struct iovec iov;
	char kbuf[buflen];
	
	uio_kinit(&iov, &uiotemp, kbuf, buflen, 0,UIO_READ );
//        iov.iov_ubase = (userptr_t) buf;
//	iov.iov_len = buflen;
//      uiotemp.uio_iov = &iov;
//        uiotemp.uio_iovcnt = 1;
//        uiotemp.uio_offset = 0;
//        uiotemp.uio_resid = PATH_MAX;
//        uiotemp.uio_segflg = UIO_USERSPACE;
//        uiotemp.uio_rw = UIO_READ;
//        uiotemp.uio_space = curproc->p_addrspace;
	result = vfs_getcwd(&uiotemp);
	if (result) {
		return result;
	}
	result = copyout(kbuf,buf, buflen);
	if(result)
		return EFAULT;
	return 0;

}
