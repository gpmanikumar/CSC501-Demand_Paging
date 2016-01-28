/* bsm.c - manage the backing store mapping*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

/*-------------------------------------------------------------------------
 * init_bsm- initialize bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_bsm()
{
	STATWORD ps;
	disable(ps);

    int i = 0;
	for(i = 0; i <= MAX_ID; i++)
		{
		bsm_tab[i].bs_npages = 0;
		bsm_tab[i].bs_pid = -1;
		bsm_tab[i].bs_sem = 0;
		bsm_tab[i].bs_status = BSM_UNMAPPED;
		bsm_tab[i].bs_vpno = BASE_VIRTUAL_PAGE;
		bsm_tab[i].bs_type = PUBLIC;
		}

	restore(ps);
	return OK;
}

/*-------------------------------------------------------------------------
 * get_bsm - get a free entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL get_bsm(int* avail)
{
	STATWORD ps;
	disable(ps);
	int i = 0;
	for(i = 0; i <= MAX_ID; i++)
		{
		if(bsm_tab[i].bs_status == BSM_MAPPED)
			continue;
		else
			{
			bsm_tab[i].bs_npages = 0;
			bsm_tab[i].bs_pid = -1;
			bsm_tab[i].bs_sem = 0;
			bsm_tab[i].bs_status = BSM_MAPPED;
			bsm_tab[i].bs_vpno = BASE_VIRTUAL_PAGE;
			bsm_tab[i].bs_type = PRIVATE;
			*avail = i;
			restore(ps);
			return OK;
			}
		}
	kprintf("SYS ERROR Backing store unavailable");
	restore(ps);
	return SYSERR;
}


/*-------------------------------------------------------------------------
 * free_bsm - free an entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL free_bsm(int i)
{
	STATWORD ps;
	disable(ps);

	bsm_tab[i].bs_npages = 0;
	bsm_tab[i].bs_pid = -1;
	bsm_tab[i].bs_sem = 0;
	bsm_tab[i].bs_status = BSM_UNMAPPED;
	bsm_tab[i].bs_vpno = BASE_VIRTUAL_PAGE;
	bsm_tab[i].bs_type = PUBLIC;

	restore(ps);
	return OK;
}

/*-------------------------------------------------------------------------
 * bsm_lookup - lookup bsm_tab and find the corresponding entry
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_lookup(int pid, long vaddr, int* store, int* pageth)
{
	STATWORD ps;
	disable(ps);
	int i = 0;
	for (i = 0; i <= MAX_ID; i++)
		{
		if(bsm_tab[i].bs_pid == pid)
			break;
		}
	*store = i;
	*pageth = (vaddr/NBPG) - bsm_tab[i].bs_vpno;
//	kprintf("bsm id %d and page %d \n", *store, *pageth);
	restore(ps);
	return OK;
}


/*-------------------------------------------------------------------------
 * bsm_map - add an mapping into bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_map(int pid, int vpno, int source, int npages)
{
}



/*-------------------------------------------------------------------------
 * bsm_unmap - delete an mapping from bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_unmap(int pid, int vpno, int flag)
{
}


