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

#ifndef _VM_H_
#define _VM_H_

/*
 * VM system-related definitions.
 *
 * You'll probably want to add stuff here.
 */

//#include<addrspace.h>
#include <machine/vm.h>
#include<machine/tlb.h>
#include <spinlock.h>
/* Fault-type arguments to vm_fault() */
#define VM_FAULT_READ        0    /* A read was attempted */
#define VM_FAULT_WRITE       1    /* A write was attempted */
#define VM_FAULT_READONLY    2    /* A write to a readonly page was attempted*/
//Ashwini
#define KVADDR_TO_PADDR(vaddr) ((vaddr)-MIPS_KSEG0)



enum PageState
{
	free,
	fixed,
	clean,
	dirty
};

struct permission
{
        bool Read;
        bool Write;
        bool Execute;
};

struct regions {
        vaddr_t start;
        vaddr_t end;
        size_t size;
        struct permission *perm;
        struct regions *next;

};

struct page_table_entry
{
        vaddr_t vpn;
        paddr_t ppn;
        struct permission *perm;
        struct permission *bk_perm;
        enum PageState *state;
        //reference
        struct page_table_entry *next;
};



extern struct spinlock splock_coremap;
extern unsigned int c_used_bytes;
extern unsigned coremapsize;
extern struct coremap_entry *coremap;

struct coremap_entry{
	unsigned chunksize;
	enum PageState state;
};
//Ashwini
/* Initialization function */
void vm_bootstrap(void);

/* Fault handling function called by trap code */
int vm_fault(int faulttype, vaddr_t faultaddress);

/* Allocate/free kernel heap pages (called by kmalloc/kfree) */
paddr_t getppages(unsigned long npages);	//Ashwini
vaddr_t alloc_kpages(unsigned npages);
void free_kpages(vaddr_t addr);

/*
 * Return amount of memory (in bytes) used by allocated coremap pages.  If
 * there are ongoing allocations, this value could change after it is returned
 * to the caller. But it should have been correct at some point in time.
 */
unsigned int coremap_used_bytes(void);

/* TLB shootdown handling called from interprocessor_interrupt */
void vm_tlbshootdown_all(void);
void vm_tlbshootdown(const struct tlbshootdown *);
/*
int region_walk(vaddr_t faultaddress, struct addrspace *as, struct permission *perm);
void pg_dir_walk(struct addrspace *as,vaddr_t faultaddress, struct permission *perm);
void write_to_tlb(vaddr_t faultaddress, struct permission *perm, int ppn);
*/
#endif /* _VM_H_ */
