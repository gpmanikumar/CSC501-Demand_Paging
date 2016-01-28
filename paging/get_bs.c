#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

int get_bs(bsd_t bs_id, unsigned int npages) {

  /* requests a new mapping of npages with ID map_id */

    STATWORD ps;
    disable(ps);
    if((npages<1)||(npages > 128)||(bs_id>MAX_ID)||(bs_id<0))
    	{
	    	kprintf("Requested backing store or pages do not match conditions");
		restore(ps);
		return SYSERR;
    	}

	if(bsm_tab[bs_id].bs_type == PRIVATE)
		{
		kprintf("Requested backing store is used for private heap");
		restore(ps);
		return SYSERR;
		}
	
	if(bsm_tab[bs_id].bs_sem == 1)
		{
		kprintf("Requested backing store is being use by different process");
		restore(ps);
		return SYSERR;
		}
	
	bsm_tab[bs_id].bs_npages = npages;
	bsm_tab[bs_id].bs_pid = currpid;
	proctab[currpid].store = bs_id;
//	bsm_tab[bs_id].bs_status = BSM_MAPPED;	
	restore(ps);
    	return npages;

}


