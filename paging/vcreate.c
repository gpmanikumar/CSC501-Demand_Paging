/* vcreate.c - vcreate */
    
#include <conf.h>
#include <i386.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <paging.h>

/*
static unsigned long esp;
*/

LOCAL	newpid();
/*------------------------------------------------------------------------
 *  create  -  create a process to start running a procedure
 *------------------------------------------------------------------------
 */
SYSCALL vcreate(procaddr,ssize,hsize,priority,name,nargs,args)
	int	*procaddr;		/* procedure address		*/
	int	ssize;			/* stack size in words		*/
	int	hsize;			/* virtual heap size in pages	*/
	int	priority;		/* process priority > 0		*/
	char	*name;			/* name (for debugging)		*/
	int	nargs;			/* number of args that follow	*/
	long	args;			/* arguments (treated like an	*/
					/* array in the code)		*/
{
	STATWORD ps;
	disable(ps);
	int pid;
	pid = create(procaddr,ssize,priority,name,nargs,args);
	int bs_store;
//	kprintf("Create done and pid is %d", pid);
	if(get_bsm(&bs_store) == SYSERR)
		return SYSERR;
//	kprintf("get_bsm returns bs %d\n", bs_store);
	bsm_tab[bs_store].bs_pid = pid;
	bsm_tab[bs_store].bs_npages = hsize;

	proctab[pid].store = bs_store;
	proctab[pid].vhpno = BASE_VIRTUAL_PAGE;
	proctab[pid].vhpnpages = hsize;
	proctab[pid].vmemlist->mnext = BASE_VIRTUAL_PAGE*NBPG;
//	kprintf("vmemlist to be initialised\n");
	struct mblock *base;
	base = BACKING_STORE_BASE+(bs_store*BACKING_STORE_UNIT_SIZE);
	base->mlen = hsize*NBPG;
	base->mnext = NULL;
	//bsm_tab[bs_store].bs_npages = hsize;
//	kprintf("vcreate done\n");
	restore(ps);
	return pid;
}

/*------------------------------------------------------------------------
 * newpid  --  obtain a new (free) process id
 *------------------------------------------------------------------------
 */
LOCAL	newpid()
{
	int	pid;			/* process id to return		*/
	int	i;

	for (i=0 ; i<NPROC ; i++) {	/* check all NPROC slots	*/
		if ( (pid=nextproc--) <= 0)
			nextproc = NPROC-1;
		if (proctab[pid].pstate == PRFREE)
			return(pid);
	}
	return(SYSERR);
}
