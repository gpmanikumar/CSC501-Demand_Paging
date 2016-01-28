/* xm.c = xmmap xmunmap */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

extern int page_replace_policy;
/*-------------------------------------------------------------------------
 * xmmap - xmmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmmap(int virtpage, bsd_t source, int npages)
{
  /* sanity check ! */
  if ( (virtpage < 4096) || ( source < 0 ) || ( source > MAX_ID) ||(npages < 1) || ( npages >128)){
	kprintf("xmmap call error: parameter error! \n");
	return SYSERR;
  }

  bsm_tab[source].bs_npages = npages;
  bsm_tab[source].bs_pid = currpid;
  bsm_tab[source].bs_sem = 1;
  bsm_tab[source].bs_status = BSM_MAPPED;
  bsm_tab[source].bs_type = PUBLIC;
  bsm_tab[source].bs_vpno = virtpage;
  proctab[currpid].vhpno = virtpage;
  //kprintf("xmmap - to be implemented!\n");
  return OK;
}



/*-------------------------------------------------------------------------
 * xmunmap - xmunmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmunmap(int virtpage )
{
  /* sanity check ! */
  int bs_store;
  int page;
  unsigned long vaddr;
  if ( (virtpage < 4096) ){ 
	kprintf("xmummap call error: virtpage (%d) invalid! \n", virtpage);
	return SYSERR;
  }

  int i = 0;
  for(i = 0; i < NFRAMES; i++)
  	{
  	if(frm_tab[i].fr_pid == currpid && frm_tab[i].fr_type == FR_PAGE)
  		{
  		vaddr = virtpage*NBPG;
  		bsm_lookup(currpid,vaddr,&bs_store,&page);
		write_bs( (i+NFRAMES)*NBPG, bs_store, page);
  		if(page_replace_policy == FIFO)
                                update_frm_fifo(i);
                free_frm(i);
		}
  	}

  bsm_tab[bs_store].bs_npages = 0;
  bsm_tab[bs_store].bs_pid = -1;
  bsm_tab[bs_store].bs_sem = 0;
  bsm_tab[bs_store].bs_status = BSM_UNMAPPED;
  bsm_tab[bs_store].bs_type = PUBLIC;
  bsm_tab[bs_store].bs_vpno = BASE_VIRTUAL_PAGE;
//  kprintf("xmunmap done\n");

  return OK;
}

