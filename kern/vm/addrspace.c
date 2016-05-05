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

#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <addrspace.h>
#include <vm.h>
#include <proc.h>
#include<machine/vm.h>
/*
 * Note! If OPT_DUMBVM is set, as is the case until you start the VM
 * assignment, this file is not compiled or linked or in any way
 * used. The cheesy hack versions in dumbvm.c are used instead.
 */
struct spinlock splock_addr;

struct addrspace *as_create(void)
{
	spinlock_init(&splock_addr);
	struct addrspace *as;
	
	as = kmalloc(sizeof(struct addrspace));
	if (as == NULL) {
		return NULL;
	}
	
	/*
	 * Initialize as needed.
	 */
	as->region_info = NULL;
        as->stackTop = USERSTACK;
        as->stackBot =  USERSTACK - PAGE_SIZE*1024;
        as->heap_start = 0;
        as->heap_end = 0;
        as->pg_entry = NULL;


	return as;
}

int
as_copy(struct addrspace *old, struct addrspace **ret)
{
	struct addrspace *new = kmalloc(sizeof(struct addrspace));

	new = as_create();
	if (new==NULL) {
		return ENOMEM;
	}

	/*
	 * Write this.
	 */
	//old->region_info = newas->region_info;
        new->stackTop = old->stackTop;
        new->stackBot = old->stackBot;
        new->heap_start = old->heap_start;
        new->heap_end = old->heap_end;

	struct regions *start1 = old->region_info ;
	struct regions *previous=NULL;
	new->region_info = NULL;

	while(start1!=NULL)
	{
    		 struct regions *temp =(struct regions *) kmalloc(sizeof(struct regions));
		if (temp==NULL) {
        	        return ENOMEM;
	        }

                 temp->start = start1->start;
                 temp->end = start1->end;
                 temp->size = start1->size;
                 temp->perm = kmalloc(sizeof(struct permission));
		if (temp->perm==NULL) {
	                return ENOMEM;
       		 }

                 temp->perm->Read = start1->perm->Read;
                 temp->perm->Write = start1->perm->Write;
                 temp->perm->Execute = start1->perm->Execute;
		 temp->bk_perm = kmalloc(sizeof(struct permission));
		if (temp->bk_perm==NULL) {
        	        return ENOMEM;
	        }

                 temp->bk_perm->Read = start1->perm->Read;
                 temp->bk_perm->Write = start1->perm->Write;
                 temp->bk_perm->Execute = start1->perm->Execute;
		 temp->next=NULL;

		if(new->region_info==NULL)
        	{
        		new->region_info=temp;
	        	previous=temp;
    		}
    		else
    		{
    		    	previous->next=temp;
        		previous=temp;          
    		}
    		start1=start1->next;
	}
	//	(void)old;

	struct page_table_entry *pg_old = old->pg_entry ;
        struct page_table_entry *previous_pg = NULL;
        
        new->pg_entry=NULL;

        while(pg_old!=NULL)
        {
                struct page_table_entry *temp = (struct page_table_entry *)kmalloc(sizeof(struct page_table_entry));
		if (temp==NULL) {
        	        return ENOMEM;
	        }

                temp->vpn = pg_old->vpn;
                temp->ppn =getppages(1);////////////////////////////////////////Change//////////////////////
		if(!temp->ppn)
		{
			return ENOMEM;
		}
		memmove((void*)PADDR_TO_KVADDR(temp->ppn),(void const*)PADDR_TO_KVADDR(pg_old->ppn),PAGE_SIZE);
                temp->perm = (struct permission *)kmalloc(sizeof(struct permission));
		if (temp->perm==NULL) {
	                return ENOMEM;
        	}

                temp->perm->Read = pg_old->perm->Read;
                temp->perm->Write = pg_old->perm->Write;
                temp->perm->Execute = pg_old->perm->Execute;
              //  struct permission *perm = kmalloc(sizeof(struct permission));
               /* temp->bk_perm = (struct permission *) kmalloc(sizeof(struct permission));
                temp->bk_perm->Read = pg_old->bk_perm->Read;
                temp->bk_perm->Write = pg_old->bk_perm->Write;
                temp->bk_perm->Execute = pg_old->bk_perm->Execute; */
                temp->state = pg_old->state;
 
                temp->next=NULL;

                if(new->pg_entry==NULL)
                {
                        new->pg_entry=temp;
                        previous_pg=temp;
                }
                else
                {
                        previous_pg->next=temp;
                        previous_pg=temp;
                }
                pg_old=pg_old->next;
        }


	*ret = new;
	return 0;
}

void
as_destroy(struct addrspace *as)
{
	/*
	 * Clean up as needed.
	 */
	struct regions *next = as->region_info ;
	while(1)
	{
		//struct regions *next = as->region_info ;
		if( next!= NULL)
		{
			if(next->perm!=NULL)
			{
				kfree(next->perm);
			}
			if(next->bk_perm!=NULL)
                        {
                                kfree(next->bk_perm);
                        }
			struct regions *temp = next;
			next = next->next;
			kfree(temp);
			//next = next->next;

			
		}
		else
		{
			break;
		}
	}
	struct page_table_entry *pnext = as->pg_entry ;
	while(1)
        {
                //struct regions *next = as->region_info ;
                if( pnext!= NULL)
                {
                        if(pnext->perm!=NULL)
                        {
                                kfree(pnext->perm);
                        }
		/*	if(pnext->bk_perm!=NULL)
                        {
                                kfree(pnext->bk_perm);
                        }*/

                        struct page_table_entry *ptemp = pnext;
                        pnext = pnext->next;
                        kfree(ptemp);
                        //next = next->next;
                }

                else
                {
                        break;
                }
        }

//	if( as->pg_entry!=NULL)
//	{
//		kfree(as->pg_entry);
//	}
	
	as->stackTop = 0;
        as->stackBot = 0;
        as->heap_start = 0;
        as->heap_end = 0;
	kfree(as);
}

void
as_activate(void)
{
	struct addrspace *as;

	as = proc_getas();
	if (as == NULL) {
		/*
		 * Kernel thread without an address space; leave the
		 * prior address space in place.
		 */
		return;
	}
	spinlock_acquire(&splock_addr);

	for (int i=0; i<NUM_TLB; i++) {
		tlb_write(TLBHI_INVALID(i), TLBLO_INVALID(), i);
	}
	spinlock_release(&splock_addr);
		
	/*
	 * Write this.
	 */
}

void
as_deactivate(void)
{
	/*
	 * Write this. For many designs it won't need to actually do
	 * anything. See proc.c for an explanation of why it (might)
	 * be needed.
	 */
}

/*
 * Set up a segment at virtual address VADDR of size MEMSIZE. The
 * segment in memory extends from VADDR up to (but not including)
 * VADDR+MEMSIZE.
 *
 * The READABLE, WRITEABLE, and EXECUTABLE flags are set if read,
 * write, or execute permission should be set on the segment. At the
 * moment, these are ignored. When you write the VM system, you may
 * want to implement them.
 */
int
as_define_region(struct addrspace *as, vaddr_t vaddr, size_t memsize,
		 int readable, int writeable, int executable)
{
	/*
	 * Write this.
	 */
//	struct regions *region_info = kmalloc(sizeof(struct regions));
//	size_t npages;
	memsize += vaddr & ~(vaddr_t)PAGE_FRAME;
	vaddr &= PAGE_FRAME;

	/* ...and now the length. */
	memsize = (memsize + PAGE_SIZE - 1) & PAGE_FRAME;
	int npages;
	if(memsize%PAGE_SIZE==0)
	{
		npages = memsize/PAGE_SIZE;
	}
	else
	{
		npages =(memsize/PAGE_SIZE)+1;
	}
//	npages = memsize / PAGE_SIZE;
	struct regions *region_info = kmalloc(sizeof(struct regions));
	if(region_info == NULL)
	{
		return ENOMEM;
	}
	region_info->start = vaddr;
        region_info->end = vaddr + npages*PAGE_SIZE;
	region_info->size = memsize;
	region_info->perm = kmalloc(sizeof(struct permission));
	if (region_info->perm==NULL) {
                return ENOMEM;
        }

	region_info->bk_perm = kmalloc(sizeof(struct permission));
	if (region_info->bk_perm==NULL) {
                return ENOMEM;
        }

//	struct permission *perm = kmalloc(sizeof(struct permission));
	region_info->perm->Read= false;
	region_info->perm->Write= false;
	region_info->perm->Execute= false;	
	region_info->bk_perm->Read= false;
        region_info->bk_perm->Write= false;
        region_info->bk_perm->Execute= false;
//	region_info.perm = &perm;
	region_info->next = NULL;

	if(readable>0)
	{
		region_info->perm->Read= true;
		region_info->bk_perm->Read= true;
	}
        if(writeable>0)
        {
                region_info->perm->Write= true;
		region_info->bk_perm->Write= true;
        }
	if(executable>0)
        {
                region_info->perm->Execute= true;
		region_info->bk_perm->Execute= true;
        }

        //struct regions *next = as->region_info;
	as->heap_start = vaddr + memsize ;
	if(as->heap_end > as->heap_start)
	{
		as->heap_end = as->heap_end;
	}
	else
	{
		as->heap_end = as->heap_start;
	}
//	as->heap_end = as->heap_start;
	as->stackTop = USERSTACK;
	as->stackBot = USERSTACK - PAGE_SIZE*1024;
	
/*	struct regions *next = as->region_info;
	while(1)
	{
		if(next==NULL)
		{
			next = region_info;
			return 0;
		}
		else
		{
			next = next->next;
		}
	}*/
	struct regions *next = as->region_info ;
	struct regions *prev;
//        struct regions *previous = NULL;


	if(as->region_info == NULL)
	{
		as->region_info = region_info;
		return 0;
	}
	else
	{
		while(next!=NULL)
		{
			prev = next;
			next = next->next;
		}
		prev->next = region_info;
		return 0;
	}
	
	
	return ENOSYS;
}





int
as_prepare_load(struct addrspace *as)
{
	/*
	 * Write this.
	 */
	struct regions *next = as->region_info;

	while(next!=NULL)
	{

			next->perm->Read = 1;
			next->perm->Write = 1;
			next = next->next;	
		
	}
//	(void)as;
	return 0;
}

int
as_complete_load(struct addrspace *as)
{
	/*
	 * Write this.
	 */

	struct regions *next = as->region_info;

        while(next!=NULL)
        {
         
        	next->perm->Read =  next->bk_perm->Read;
                next->perm->Write =  next->bk_perm->Write;
		next = next->next;
         
        }

//	(void)as;
	return 0;
}

int
as_define_stack(struct addrspace *as, vaddr_t *stackptr)
{
	/*
	 * Write this.
	 */

	(void)as;
	/* Initial user-level stack pointer */
	*stackptr = USERSTACK;

	return 0;
}
int region_walk(vaddr_t faultaddress, struct addrspace *as, struct permission **perm1)
{
        struct regions *next = as->region_info ;
	 struct permission *perm = kmalloc(sizeof(struct permission));
	if (perm==NULL) {
                return ENOMEM;
        }


        while(1)
        {
                //struct regions *next = as->region_info ;
                if( next!= NULL)
                {

                        if((next->start<= faultaddress) && (faultaddress < next->end ))
                        {
                                perm->Read = next->perm->Read;
                                perm->Write = next->perm->Write;
                                perm->Execute = next->perm->Execute;
				*perm1 = perm; 
                                return 0;
                        }
                        next = next->next;

                        //next = next->next;
                }
                else
                        break;

        }
        if((faultaddress >= as->heap_start) && (faultaddress < as->heap_end ) )
        {
                perm->Read = true;
                perm->Write = true;
                perm->Execute = false;
		*perm1 = perm;
                return 0;
        }
        if((as->stackTop >= faultaddress) && (as->stackBot < faultaddress))
        {
                perm->Read = true;
                perm->Write = true;
                perm->Execute = false;
		*perm1 = perm;
                return 0;

        }
        return -1;

}

int pg_dir_walk(struct addrspace *as,vaddr_t faultaddress, struct permission *perm)
{
        vaddr_t vpn = faultaddress;
        paddr_t ppn;
        struct page_table_entry *pnext = as->pg_entry ;
         while(1)
        {
                //struct regions *next = as->region_info ;
                if( pnext!= NULL)
                {
                        if(pnext->vpn == vpn)
                        {
                                ppn = pnext->ppn;
                                write_to_tlb(faultaddress, perm,ppn);
                                return 0;
                        }
                        pnext = pnext->next;
                }

                else
                {
                        struct page_table_entry *temp = (struct page_table_entry *)kmalloc(sizeof(struct page_table_entry));
			if (temp==NULL) {
                		return ENOMEM;
		        }

                        temp->vpn = vpn;
			temp->ppn=getppages(1);
			if (!temp->ppn) {
                                return ENOMEM;
                        }

                       // temp->ppn = KVADDR_TO_PADDR(vaddr1);////////////////////////////////////////******************Change**************//////////////////////
                        ppn= temp->ppn;
                        temp->perm = (struct permission *)kmalloc(sizeof(struct permission));
			if (temp->perm==NULL) {
		                return ENOMEM;
        		}

                        //struct permission perm1 = kmalloc(sizeof(struct permission));
                        temp->perm->Read =perm->Read;
                        temp->perm->Write = perm->Write;
                        temp->perm->Execute = perm->Execute;
          /*              temp->bk_perm = (struct permission *)kmalloc(sizeof(struct permission));
                        temp->bk_perm->Read = perm->Read;
                        temp->bk_perm->Write = perm->Write;
                        temp->bk_perm->Execute = perm->Execute;*/
                        temp->next = NULL;

                    
                        break;
                }
        }
        spinlock_acquire(&splock_addr);
        write_to_tlb(faultaddress, perm,ppn);
        spinlock_release(&splock_addr);
	return 0;

}

void write_to_tlb(vaddr_t faultaddress, struct permission *perm, paddr_t ppn)
{
        uint32_t ehi, elo;
        ehi = faultaddress;
        elo=0;
        if(perm-> Write)
                elo = ppn | TLBLO_DIRTY | TLBLO_VALID;
        else
                elo = ppn | TLBLO_VALID;
//	spinlock_acquire(&splock_addr);
        tlb_random(ehi, elo);
//	spinlock_release(&splock_addr);

}

int vm_fault(int faulttype, vaddr_t faultaddress)
{
//      vaddr_t vbase1, vtop1, vbase2, vtop2, stackbase, stacktop;
//      paddr_t paddr;
//      int i;
        //int vpn = (faultaddress & 0xfffff000)>> 12;
        uint32_t ehi, elo;
        struct addrspace *as;
//        int spl;
        struct permission *perm;

        faultaddress &= PAGE_FRAME;
        //int vpn = (faultaddress & 0xfffff000)>> 12;
        if (curproc == NULL) {
                /*
                 * No process. This is probably a kernel fault early
                 * in boot. Return EFAULT so as to panic instead of
                 * getting into an infinite faulting loop.
                 */
                return EFAULT;
        }

        as = proc_getas();
        if (as == NULL) {
                /*
                 * No address space set up. This is probably also a
                 * kernel fault early in boot.
                 */
                return EFAULT;
        }
        int result = region_walk(faultaddress, as, &perm);
        if(result)
        {
                return result;
        }

        switch (faulttype) {
            case VM_FAULT_READONLY:
                if(perm->Write)
                {
                       // spl = splhigh();
			spinlock_acquire(&splock_addr);
                        ehi =faultaddress;
                        uint32_t index = tlb_probe(ehi,elo);
			uint32_t nehi;
                        tlb_read(&nehi,&elo,index);
                        elo = elo | TLBLO_DIRTY | TLBLO_VALID;
                        //set_page_Dirty(elo);
			tlb_write(nehi,elo,index);
			spinlock_release(&splock_addr);
//			set_page_Dirty(elo);		//////////////////////////////////////////////////////////////////
                      //  splx(spl);
                }
                else
                        panic("\n Tried to write a readonly page");
                break;
            case VM_FAULT_READ:
            case VM_FAULT_WRITE:
                result = pg_dir_walk(as,faultaddress,perm);
		if(result)
	        {
        	        return result;
        	}

        //      write_to_tlb(faultaddress, &perm);
                return 0;
                break;
            default:
                return EINVAL;
        }

	return 0;

}
