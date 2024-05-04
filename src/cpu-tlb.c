/*
 * Copyright (C) 2024 pdnguyen of the HCMC University of Technology
 */
/*
 * Source Code License Grant: Authors hereby grants to Licensee 
 * a personal to use and modify the Licensed Source Code for 
 * the sole purpose of studying during attending the course CO2018.
 */

/*
 * CPU TLB
 * TLB module cpu/cpu-tlb.c
 */
 
#include "mm.h"
#include <stdlib.h>
#include <stdio.h>
#ifdef CPU_TLB

int tlb_change_all_page_tables_of(struct pcb_t *proc,  struct memphy_struct * mp)
{
  /* TODO update all page table directory info 
   *      in flush or wipe TLB (if needed)
   */ 

    //  Chắc không cần phần này lắm, 


  return 0;
}

int tlb_flush_tlb_of(struct pcb_t *proc, struct memphy_struct * mp)
{
  /* TODO flush tlb cached*/
  if(mp == NULL) return -1; 
  int index = 0; 
  int max_index = proc->tlb->maxsz; 
  int tlb_id; 
  for(index = 0; index < max_index; index += 10) 
  { 
      tlb_id = mp->storage[tlbnb] << 24 | mp->storage[tlbnb + 1] << 16 | mp->storage[tlbnb + 2] << 8 | mp->storage[tlbnb + 3];
      if(tlb_id == proc->pid) flush_rg(mp,tlbnb); 
  }
  return 0;
}

/*tlballoc - CPU TLB-based allocate a region memory
 *@proc:  Process executing the instruction
 *@size: allocated size 
 *@reg_index: memory region ID (used to identify variable in symbole table)
 */
int tlballoc(struct pcb_t *proc, uint32_t size, uint32_t reg_index)
{
  printf("tlballoc!\n");
  if(proc->tlb == NULL) return -1; 
  int addr, val;

  /* By default using vmaid = 0 */
  val = __alloc(proc, 0, reg_index, size, &addr);

  /* TODO update TLB CACHED frame num of the new allocated page(s)*/
  /* by using tlb_cache_read()/tlb_cache_write()*/
  int pgn = PAGING_PGN(addr); 
  //get the PTE
  uint32_t pte = proc->mm->pgd[pgn];  

  //check for write
  if(tlb_cache_write(proc->tlb,proc->pid,pgn,pte) == -1) return -1; 

  return val;
}

/*pgfree - CPU TLB-based free a region memory
 *@proc: Process executing the instruction
 *@size: allocated size 
 *@reg_index: memory region ID (used to identify variable in symbole table)
 */
int tlbfree_data(struct pcb_t *proc, uint32_t reg_index)
{
  __free(proc, 0, reg_index);

  /* TODO update TLB CACHED frame num of freed page(s)*/
  /* by using tlb_cache_read()/tlb_cache_write()*/

  // 

  return 0;
}


/*tlbread - CPU TLB-based read a region memory
 *@proc: Process executing the instruction
 *@source: index of source register
 *@offset: source address = [source] + [offset]
 *@destination: destination storage
 */
int tlbread(struct pcb_t * proc, uint32_t source,
            uint32_t offset, 	uint32_t destination) 
{
  //done 
  printf("read!\n");
  BYTE data; 
  int frmnum = -1;
  BYTE check = 0; 
  int addr = proc->regs[source] + offset;
  //getting the page number 
  int pgn = PAGING_PGN(addr);

  /* TODO retrieve TLB CACHED frame num of accessing page(s)*/
  /* by using tlb_cache_read()/tlb_cache_write()*/
  /* frmnum is return value of tlb_cache_read/write value*/
  frmnum = tlb_cache_read(proc->tlb,proc->pid,pgn,&check);

  //check if the address is exists on RAM or not! 
  if(check == -1) {

      //if not exists -> get PAGE!, if not exists -> ERROR!
      if(pg_getpage(proc->mm,pgn,&frmnum,proc) != 0) // Need page is loaded into RAM  
        return -1; /* invalid page access */

  } 
	
#ifdef IODUMP
  if (frmnum >= 0)
    printf("TLB hit at read region=%d offset=%d\n", 
	         source, offset);
  else 
    printf("TLB miss at read region=%d offset=%d\n", 
	         source, offset);
#ifdef PAGETBL_DUMP
  print_pgtbl(proc, 0, -1); //print max TBL
#endif
  MEMPHY_dump(proc->mram);
#endif

   //if hit 
  if(frmnum > 0) { 
      //get the physic pos 
      int phyaddr = (frmnum << PAGING_ADDR_FPN_LOBIT) + offset;
      //read the TLB memphy, if there is no data -> error 
      if(TLBMEMPHY_read(proc->mram,phyaddr,&data) == -1) 
        return -1;  
  }  
  //if Miss 
  else{  
        /* TODO update TLB CACHED with frame num of recent accessing page(s)*/
        /* by using tlb_cache_read()/tlb_cache_write()*/ 
        __read(proc, 0, source, offset, &data);
        //get the PTE from the table 
          uint32_t pte = proc->mm->pgd[pgn];  
        //perform writting on cache 
          if(tlb_cache_write(proc->tlb,proc->pid,pgn,pte) == -1) 
            return -1; /*cannot write~*/
  } 

  //get it into the process register! 
  proc->regs[destination] = (uint32_t)data; 

  return 0; 
}

/*tlbwrite - CPU TLB-based write a region memory
 *@proc: Process executing the instruction
 *@data: data to be wrttien into memory
 *@destination: index of destination register
 *@offset: destination address = [destination] + [offset]
 */
int tlbwrite(struct pcb_t * proc, BYTE data,
             uint32_t destination, uint32_t offset)
{
  int val;
  int frmnum = -1;
  BYTE check = 0; 

  int addr = proc->regs[destination] + offset;
  //getting the page number and offset 
    int pgn = PAGING_PGN(addr);
    int off = PAGING_OFFST(addr);
 
  /* TODO retrieve TLB CACHED frame num of accessing page(s))*/
  /* by using tlb_cache_read()/tlb_cache_write()
  frmnum is return value of tlb_cache_read/write value*/
  frmnum = tlb_cache_read(proc->tlb,proc->pid,pgn,&check);

  if(check == -1) {
      //if not exists -> get PAGE!, if not exists -> ERROR!
      if(pg_getpage(proc->mm,pgn,&frmnum,proc) != 0) //Need page is loaded into RAM  
        return -1; /* invalid page access */
  } 

#ifdef IODUMP
  if (frmnum >= 0)
    printf("TLB hit at write region=%d offset=%d value=%d\n",
	          destination, offset, data);
	else
    printf("TLB miss at write region=%d offset=%d value=%d\n",
            destination, offset, data);
#ifdef PAGETBL_DUMP
  print_pgtbl(proc, 0, -1); //print max TBL
#endif
  MEMPHY_dump(proc->mram);
#endif
  
  // do if hit
  if(frmnum >= 0) {
      //get physical address
      int phyaddr = (frmnum << PAGING_ADDR_FPN_LOBIT) + off;
      //perform write 
      val = MEMPHY_write(proc->mram,phyaddr,data); 
  }

  // do if miss   
  else { 
      /* TODO update TLB CACHED with frame num of recent accessing page(s)*/
      /* by using tlb_cache_read()/tlb_cache_write()*/
      val = __write(proc, 0, destination, offset, data);
      uint32_t pte = proc->mm->pgd[pgn];  
      //perform writting on cache 
        if(tlb_cache_write(proc->tlb,proc->pid,pgn,pte) == -1) 
            return -1; /*cannot write~*/
  }
  return val;
}

#endif