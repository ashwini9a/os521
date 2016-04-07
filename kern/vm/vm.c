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
 * Return amount of memory (in bytes) used by allocated coremap pages.  If
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

int vm_fault(int faulttype, vaddr_t faultaddress)
{
	(void)faulttype;
	(void)faultaddress;
	return 0;
}
