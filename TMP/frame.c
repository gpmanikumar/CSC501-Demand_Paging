/* frame.c - manage physical frames */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

extern int page_replace_policy;
/*-------------------------------------------------------------------------
 * init_frm - initialize frm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_frm()
{
//  STATWORD ps;
//  disable(ps);
  int i = 0;
  for(i = 0; i < NFRAMES; i++)
  	{
  	frm_tab[i].cookie = 0;
	frm_tab[i].fr_dirty = 0;
	frm_tab[i].fr_loadtime = 0;
	frm_tab[i].fr_pid = -1;
	frm_tab[i].fr_refcnt = 0;
	frm_tab[i].fr_status = FRM_UNMAPPED;
	frm_tab[i].fr_type = FR_PAGE;
	frm_tab[i].fr_vpno = 0;
  	}

//  kprintf("Frame initialization done\n");
//  restore(ps);
  return OK;
}

/*-------------------------------------------------------------------------
 * get_frm - get a free frame according page replacement policy
 *-------------------------------------------------------------------------
 */
SYSCALL get_frm(int* avail)
{
  //STATWORD ps;
  //disable(ps);
  int i = 0;
  int frame_number;
//  kprintf("get_frame called by pid %d and name %s\n", currpid, proctab[currpid].pname);
  for(i = 0; i < NFRAMES; i++)
  	{
//	kprintf("Inside for loop in get frame\n");
  	if(frm_tab[i].fr_status == FRM_UNMAPPED)
  		{
  		*avail = i;
//		kprintf("Frame %d is found UNMAPPED and returned\n", i);
		return OK;
  		}
  	}


  	{
  	if(page_replace_policy == FIFO)
  		{
//		kprintf("Frame to be removed\n");
  		frame_number = remove_frm_fifo();
		if(frame_number == -1)
			return SYSERR;
		free_frm(frame_number);
		*avail = frame_number;
//		kprintf("Returned frame is %d\n", *avail);
		return OK;
  		}
	else
		{
		frame_number = get_frm_LRU();
		free_frm(frame_number);
		*avail = frame_number;
		return OK;
		}
  	}
  //kprintf("To be implemented!\n");
  //restore(ps);
  return OK;
}

/*-------------------------------------------------------------------------
 * free_frm - free a frame 
 *-------------------------------------------------------------------------
 */
SYSCALL free_frm(int i)
{
	STATWORD ps;
	disable(ps);
	unsigned long v_addr;	
	unsigned int pt_offset;
	unsigned int pd_offset;
	unsigned long pdbr;
	pd_t *pd_entry; 
	pt_t *pt_entry;
	int bs_store;
	int page_num;
	if(frm_tab[i].fr_type == FR_PAGE)
	{
//		kprintf("Inside free frame starting \n");
		v_addr = frm_tab[i].fr_vpno;
		pdbr = proctab[frm_tab[i].fr_pid].pdbr;			
		pd_offset = v_addr>>10;
		pt_offset = v_addr & 0x000003FF;
		pd_entry = pdbr+(pd_offset*sizeof(pd_t));
		pt_entry = (pd_entry->pd_base*NBPG)+(pt_offset*sizeof(pt_t));
		bs_store = proctab[frm_tab[i].fr_pid].store;
		page_num = frm_tab[i].fr_vpno-proctab[frm_tab[i].fr_pid].vhpno;
//		kprintf("Before write bs bs_store %d and page_num %d\n", bs_store, page_num);
		write_bs((i+FRAME0)*NBPG, bs_store, page_num);
//		kprintf("After write bs\n");
		pt_entry->pt_pres = 0;
		pt_entry->pt_write = 0;
		pt_entry->pt_base = 0;
		frm_tab[i].cookie = 0;
  		frm_tab[i].fr_dirty = 0;
  		frm_tab[i].fr_loadtime = 0;
  		frm_tab[i].fr_pid = -1;
 		frm_tab[i].fr_refcnt = 0;
 		frm_tab[i].fr_status = FRM_UNMAPPED;
 		frm_tab[i].fr_type = FR_PAGE;
 		frm_tab[i].fr_vpno = BASE_VIRTUAL_PAGE;
 		frm_tab[i].next_frame = -1;
		frm_tab[pd_entry->pd_base-FRAME0].fr_refcnt--;
		if(frm_tab[pd_entry->pd_base-FRAME0].fr_refcnt == 0)
			{
			frm_tab[pd_entry->pd_base-FRAME0].cookie = 0;
			frm_tab[pd_entry->pd_base-FRAME0].fr_dirty = 0;
			frm_tab[pd_entry->pd_base-FRAME0].fr_loadtime = 0;
			frm_tab[pd_entry->pd_base-FRAME0].fr_pid = -1;
			frm_tab[pd_entry->pd_base-FRAME0].fr_refcnt = 0;
			frm_tab[pd_entry->pd_base-FRAME0].fr_status = FRM_UNMAPPED;
			frm_tab[pd_entry->pd_base-FRAME0].fr_type = FR_PAGE;
			frm_tab[pd_entry->pd_base-FRAME0].fr_vpno = BASE_VIRTUAL_PAGE;
			frm_tab[pd_entry->pd_base-FRAME0].next_frame = -1;
			pd_entry->pd_pres = 0;
			}
	}
	//pd_entry->pd_pres = 0;
//	kprintf("Free done\n");
 	restore(ps);
	return OK;
}

void insert_frm_fifo(int frame_num)
{
//	STATWORD ps;
//	disable(ps);
	int nxt_frm = -1;
	int curr_frm = -1;
	if(fifo_head == -1)
		{
		fifo_head = frame_num;
		frm_tab[frame_num].next_frame = -1;
		}
	else
		{
		nxt_frm = frm_tab[fifo_head].next_frame;
		curr_frm = fifo_head;
		while(nxt_frm != -1)
			{
			curr_frm = nxt_frm;
			nxt_frm = frm_tab[nxt_frm].next_frame;
			}
		frm_tab[curr_frm].next_frame = frame_num;
		frm_tab[frame_num].next_frame = -1;
		}
//	kprintf("Frame inserted in fifo is %d\n", frame_num);
//	restore(ps);
	return OK;
	
}


int remove_frm_fifo()
{
//	STATWORD ps;
//	disable(ps);

	if(fifo_head == -1)
		return -1;
	int frame_number;
	frame_number = fifo_head;
	fifo_head = frm_tab[fifo_head].next_frame;
	frm_tab[frame_number].next_frame = -1;

//	restore(ps);
//	kprintf("Frame returned from remove fifo is %d\n", frame_number);
	return(frame_number);
}

int get_frm_LRU()
{
//	STATWORD ps;
	unsigned long int load_time=99999999;
	int i;
	int frame_number = -1;
	fr_map_t *frm;
//	kprintf("In LRU\n");
	for(i=0;i<NFRAMES;i++)
	{
		frm = &frm_tab[i];
		if(frm_tab[i].fr_type==FR_PAGE)
		{
			if(frm->fr_loadtime < load_time)
			{
				load_time = frm->fr_loadtime;
				frame_number = i;
			}

			else if(load_time == frm->fr_loadtime)
			{
				if(frm_tab[frame_number].fr_vpno < frm_tab[i].fr_vpno)
					frame_number = i;
			}
		}
	}

//	restore(ps);
//	kprintf("Frame %d is returned in LRU", frame_number);
	return frame_number;
}

void update_frms_LRU()
{
	STATWORD ps;
	disable(ps);
	timeCount++;
	unsigned long v_addr;	
	unsigned int pt_offset;
	unsigned int pd_offset;
	unsigned long pdbr;
	pd_t *pd_entry; 
	pt_t *pt_entry;
//	kprintf("In update LRU\n");
	int i = 0;
	for(i = 0; i < NFRAMES; i++)
		{
		if((frm_tab[i].fr_status == FRM_MAPPED)&&(frm_tab[i].fr_type == FR_PAGE))
			{
			v_addr = frm_tab[i].fr_vpno;
			pdbr = proctab[frm_tab[i].fr_pid].pdbr;			
			pd_offset = v_addr>>10;
                	pt_offset = v_addr & 0x000003FF;
			pd_entry = pdbr+(pd_offset*sizeof(pd_t));
			pt_entry = (pd_entry->pd_base*NBPG)+(pt_offset*sizeof(pt_t));
//			kprintf("pt entry address is 0x%08x\n", pt_entry);
			if(pt_entry->pt_acc == 1)
				{
//				kprintf("Page accesses is set and has to be reset\n");
				frm_tab[i].fr_loadtime = timeCount;
				pt_entry->pt_acc = 0;
				}
			}
		
		}
	restore(ps);
}

void update_frm_fifo(int i)
{
//	STATWORD ps;
//	disable(ps);
	int nxt_frm = -1;
	int curr_frm = -1;
	if(fifo_head == i)
		{
		fifo_head = frm_tab[i].next_frame;
		frm_tab[i].next_frame = -1;
		}
	else
		{
		nxt_frm = frm_tab[fifo_head].next_frame;
		curr_frm = fifo_head;
		while(nxt_frm != i)
			{
			curr_frm = nxt_frm;
			nxt_frm = frm_tab[nxt_frm].next_frame;
			}
		frm_tab[curr_frm].next_frame = frm_tab[nxt_frm].next_frame;
		frm_tab[nxt_frm].next_frame = -1;
		}
//	kprintf("fifo_head is %d\n", fifo_head );

//	restore(ps);
}
