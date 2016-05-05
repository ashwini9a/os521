#include <vm.h>
#include <types.h>
#include<lib.h>

//extern struct spinlock splock_coremap;

//extern unsigned coremapsize;

struct spinlock splock_coremap;
unsigned int c_used_bytes;
unsigned coremapsize;
struct coremap_entry *coremap;

void vm_bootstrap(void)
{
	spinlock_init(&splock_coremap);
	paddr_t lastpaddr = ram_getsize();
	coremapsize = lastpaddr/PAGE_SIZE;
//	struct coremap_entry cm[coremapsize];
	paddr_t firstpaddr = ram_getfirstfree();
	coremap = (struct coremap_entry*)PADDR_TO_KVADDR(firstpaddr);
	firstpaddr+= sizeof(struct coremap_entry)*coremapsize;
//	coremapsize = lastpaddr/PAGE_SIZE;
	unsigned reserved;
	c_used_bytes= 0;
//	coremap = (struct coremap_entry*)PADDR_TO_KVADDR(firstpaddr);
//	struct coremap_entry cm[coremapsize];
//	coremap = cm;
	if(firstpaddr % PAGE_SIZE ==0)
	{
		 reserved = firstpaddr/PAGE_SIZE;	
	}
	else
	{
		 reserved = (firstpaddr/PAGE_SIZE)+1;
	}
	
//	unsigned reserved = firstpaddr/PAGE_SIZE;
	for(unsigned i=0;i<reserved;i++)
	{
		struct coremap_entry cm;
		cm.state=fixed;
		cm.chunksize=1;
		coremap[i]=cm;
		c_used_bytes= c_used_bytes + PAGE_SIZE;
	}
	for(unsigned i=reserved;i<coremapsize;i++)
	{
		struct coremap_entry cm;
		cm.state=free;
		cm.chunksize=0;
		coremap[i]=cm;
	}
	
	//coremap=cm;
}


paddr_t getppages(unsigned long npages)
{
	struct coremap_entry *tcoremap = coremap;
	paddr_t addr;
	unsigned long pg=npages, count=0;
//	bool status=true;
	unsigned coreindex=0;
	spinlock_acquire(&splock_coremap);
	unsigned i=0;
	for(i=0;i<coremapsize;i++,tcoremap++)
	{
		if(tcoremap->state==free)
		{
			if(count==0)
			{
				coreindex=i;
			}
			count++;
			if(count==pg)
			{break;}
		}
		else
		{count=0;}

	}
	if(count==0 || count<pg || (i==coremapsize && count<pg))
	{
		 addr= 0;
	}			
	if(i<coremapsize){	
	tcoremap = coremap;
	for(unsigned ind=0;ind<coreindex;ind++){tcoremap++;}
	for(unsigned j=coreindex;j<coreindex+pg;j++,tcoremap++)
	{
		tcoremap->state=fixed;
		tcoremap->chunksize=pg;
	}
	addr=(coreindex)*PAGE_SIZE;
	c_used_bytes= c_used_bytes + pg*PAGE_SIZE;
	}
	else
	{addr =0;}
	spinlock_release(&splock_coremap);
	bzero((void *)PADDR_TO_KVADDR(addr),npages*PAGE_SIZE);
	return addr;
}
/* Allocate/free kernel heap pages (called by kmalloc/kfree) */
vaddr_t alloc_kpages(unsigned npages)
{
	//struct coremap_entry *tcoremap = coremap;

	paddr_t pa;

	pa = getppages(npages);
	if (pa==0) {
		return 0;
	}
	//c_used_bytes= c_used_bytes + npages*PAGE_SIZE;
	return PADDR_TO_KVADDR(pa);
}
void free_kpages(vaddr_t addr)
{
	paddr_t pa = addr - MIPS_KSEG0;
	struct coremap_entry *tcoremap = coremap;

	unsigned long pg=0;
	unsigned coreindex = (pa / PAGE_SIZE);
	for(unsigned ind=0;ind<coreindex;ind++){tcoremap++;}
	spinlock_acquire(&splock_coremap);
	pg = tcoremap->chunksize;
	for(unsigned i=coreindex;i<coreindex+pg;i++,tcoremap++)
	{
		tcoremap->state = free;
		tcoremap->chunksize=0;
	}
	c_used_bytes= c_used_bytes - pg*PAGE_SIZE;
	spinlock_release(&splock_coremap);
	
}

/*

 * there are ongoing allocations, this value could change after it is returned
 * to the caller. But it should have been correct at some point in time.
 */
unsigned int coremap_used_bytes(void)
{
/*        struct coremap_entry *tcoremap = coremap;
	unsigned int nbytes=0;
	unsigned long count=0;
	spinlock_acquire(&splock_coremap);

	for(unsigned i=0;i<coremapsize;i++,tcoremap++)
        {
                if(tcoremap->state==fixed)
                {
                        count++;
                }

        }
	spinlock_release(&splock_coremap);

	nbytes=count * PAGE_SIZE;
	return nbytes; */
	return c_used_bytes;
	
}

void vm_tlbshootdown_all(void)
{
	panic("dumbvm tried to do tlb shootdown?!\n");
}

void vm_tlbshootdown(const struct tlbshootdown *ts)
{
	(void)ts;
	panic("dumbvm tried to do tlb shootdown?!\n");
}


/*
int region_walk(vaddr_t faultaddress, struct addrspace *as, struct permission *perm)
{
	struct regions *next = as->region_info ;
	
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
		return 0;
	}
	if((as->stackTop >= faultaddress) && (as->stackBot < faultaddress))
	{
		perm->Read = true;
                perm->Write = true;
                perm->Execute = false;
                return 0;

	}
	return -1;
	

}
void pg_dir_walk(struct addrspace *as,vaddr_t faultaddress, struct permission *perm)
{
	int vpn = (faultaddress & 0xfffff000)>> 12;
	int ppn;
	struct regions *pnext = as->pg_entry ;
	 while(1)
        {
                //struct regions *next = as->region_info ;
                if( pnext!= NULL)
                {
			if(pnext->vpn == vpn)
			{
				ppn = pnext->ppn;
				write_to_tlb(faultaddress, perm,ppn);
				return;
			}
                        pnext = pnext->next;
                }

                else
                {
			struct page_table_entry temp = kmalloc(sizeof(struct page_table_entry));
		        temp.vpn = vpn;
		        temp.ppn = (KVADDR_TO_PADDR(kmalloc(PAGE_SIZE)) & 0xfffff000)>>12;//////////////////////////////////////////////////////////////
			ppn= temp.ppn;
		        temp.perm = kmalloc(sizeof(struct permission*));
		        struct permission perm1 = kmalloc(sizeof(struct permission));
		        temp.perm = &perm1;
		        perm1->Read =perm->Read;
		        perm1->Write = perm->Write;
		        perm1->Execute = perm->Execute;
		        struct permission perm2 = kmalloc(sizeof(struct permission));
		        temp.bk_perm = &perm2;
		        temp.bk_perm->Read = perm->Read;
		        temp.bk_perm->Write = perm->Write;
		        temp.bk_perm->Execute = perm->Execute;
		        temp.next = NULL;

                        pnext = &temp;
                        break;
                }
        }
	spinlock_acquire(&splock_addr);
	write_to_tlb(faultaddress, perm,ppn);
	spinlock_release(&splock_addr);
	
}


void write_to_tlb(vaddr_t faultaddress, struct permission *perm, int ppn)
{
	uint32_t ehi, elo;
	ehi = faultaddress;
	elo=0;
	if(perm-> Write)
		elo = (ppn << 12) | TLBLO_DIRTY | TLBLO_VALID;
	else
		elo = (ppn << 12) | TLBLO_VALID;
	://media.giphy.com/media/xT1XGLtNWBvkESUkzm/giphy.gifandom(ehi, elo);
}


int vm_fault(int faulttype, vaddr_t faultaddress)
{
//	vaddr_t vbase1, vtop1, vbase2, vtop2, stackbase, stacktop;
//	paddr_t paddr;
//	int i;
	//int vpn = (faultaddress & 0xfffff000)>> 12;
	uint32_t ehi, elo;
	struct addrspace *as;
	int spl;
	struct permission perm=kmalloc(sizeof(struct permission));

	faultaddress &= PAGE_FRAME;
	//int vpn = (faultaddress & 0xfffff000)>> 12;
        if (curproc == NULL) {
                *
                 * No process. This is probably a kernel fault early
                 * in boot. Return EFAULT so as to panic instead of
                 * getting into an infinite faulting loop.
                 *
                return EFAULT;
        }

        as = proc_getas();
        if (as == NULL) {
                *
                 * No address space set up. This is probably also a
                 * kernel fault early in boot.
                 *
                return EFAULT;
        }
	int result = region_walk(faultaddress, as, &perm); 
	if(result)
	{
		return -1;	
	}

	switch (faulttype) {
	    case VM_FAULT_READONLY:
		if(perm->Write)
		{
			spl = splhigh();

			ehi =faultaddress;
			uint32_t index = tlb_probe(ehi,elo);
			tlb_read(&nehi,&elo,index);
			elo = elo | TLBLO_DIRTY | TLBLO_VALID;
			set_page_Dirty(elo);
			tlb_write(nehi,elo,index);
			splx(spl);
		}
		else
			panic("\n Tried to write a readonly page");
		break;		
	    case VM_FAULT_READ:
	    case VM_FAULT_WRITE:
		pg_dir_walk(as,faultaddress,&perm);
	//	write_to_tlb(faultaddress, &perm);
		return 0;
		break;
	    default:
		return EINVAL;
	}

	
}*/

