/* pfint.c - pfint */

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

extern int page_replace_policy;

/*-------------------------------------------------------------------------
 * pfint - paging fault ISR
 *-------------------------------------------------------------------------
 */
SYSCALL pfint()
{
	STATWORD ps;
	unsigned long v_addr;
	unsigned int pg_offset;
	unsigned int pt_offset;
	unsigned int pd_offset;
	unsigned long pdbr; 
	pd_t *pd_entry; 
	pt_t *pt_entry; 
	pt_t *page_table_entry;
	int i;
	int frame_number; 
	int bs_store; 
	int page;
	disable(ps);

	v_addr = read_cr2();
//	kprintf("In pfint for address 0x%08x\n", v_addr);	
	pg_offset = v_addr&0x00000FFF;
	pt_offset = (v_addr&0x003FF000)>>12;
	pd_offset = (v_addr&0xFFC00000)>>22;
	

	pdbr = proctab[currpid].pdbr;
	pd_entry = pdbr+(pd_offset*sizeof(pd_t)); 

    if(pd_entry->pd_pres == 0)
	{
		get_frm(&frame_number);

		page_table_entry = (FRAME0 + frame_number)*NBPG;


		for(i=0;i<1024;i++)
		{
			page_table_entry[i].pt_acc = 0;
			page_table_entry[i].pt_avail = 0;
			page_table_entry[i].pt_base = 0;
			page_table_entry[i].pt_dirty = 0;
			page_table_entry[i].pt_global = 0;
			page_table_entry[i].pt_mbz = 0;
			page_table_entry[i].pt_pcd = 0;
			page_table_entry[i].pt_pres = 0;
			page_table_entry[i].pt_pwt = 0;
			page_table_entry[i].pt_user = 0;
			page_table_entry[i].pt_write = 0;
			//page_table_entry++;
		}
		pd_entry->pd_pres = 1;
		pd_entry->pd_base = frame_number+FRAME0;

		frm_tab[frame_number].fr_status = FRM_MAPPED;
		frm_tab[frame_number].fr_type = FR_TBL;
		frm_tab[frame_number].fr_pid = currpid;

		restore(ps);
		return OK;

	}


	pt_entry = (pd_entry->pd_base*NBPG)+(pt_offset*sizeof(pt_t));

	if(pt_entry->pt_pres == 0)
	{

		get_frm(&frame_number);
//		kprintf("pfint for addr 0x%08x \n", v_addr);
		pt_entry->pt_pres = 1;
		pt_entry->pt_write = 1;
		pt_entry->pt_base = (FRAME0+frame_number);
//		kprintf("Before frame tab\n");
		frm_tab[pd_entry->pd_base-FRAME0].fr_refcnt++;

		frm_tab[frame_number].fr_status = FRM_MAPPED;
		frm_tab[frame_number].fr_type = FR_PAGE;
		frm_tab[frame_number].fr_pid = currpid;
		frm_tab[frame_number].fr_vpno = v_addr/NBPG;
//		kprintf("Inside pfint before bsm_lookup \n");
		bsm_lookup(currpid,v_addr,&bs_store,&page);
		if(page>=bsm_tab[bs_store].bs_npages)
		{
			kprintf("Page accessed is not allocated");
			kill(currpid);
			restore(ps);
			return SYSERR;
		}
//		kprintf("Read from page %d\n", page);
		read_bs(((FRAME0+frame_number)*NBPG),bs_store,page);
		if(page_replace_policy == FIFO)
			insert_frm_fifo(frame_number);
		else if(page_replace_policy == LRU)
			update_frms_LRU();

		if(page_replace_policy == LRU)
			frm_tab[frame_number].fr_loadtime = timeCount;
//		kprintf("pfint done\n");		
		restore(ps);
		return OK;
		
	}
	//kprintf("To be implemented!\n");
}

