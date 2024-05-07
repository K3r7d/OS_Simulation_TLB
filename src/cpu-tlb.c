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
  

  return 0;
}

int tlb_flush_tlb_of(struct pcb_t *proc, struct memphy_struct * mp)
{
  /* TODO flush tlb cached*/
  if(mp == NULL) return -1; 

  //get all the pte of:
  struct vm_area_struct *cur_vma = get_vma_by_num(proc->mm, 0);
  int start = 0; 
  int end = PAGING_PGN(cur_vma->vm_end);
  int pgit; 
  int check; 
  for(pgit = start; pgit < end; pgit++)
  {
     int frm = tlb_cache_read(proc->tlb,proc->pid,pgit,&check); 
     if(frm >= 0) { 
      int szof = mp->maxsz / 10; 
      int tlbnb = pgit % szof;
      flush_rg(proc->tlb,tlbnb);
     }
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
  printf("TLB alloc function have entered!, size of alloc: %d, reg_index: %d call by pid: %d \n", size,reg_index,proc->pid);
  if(proc->tlb == NULL) return -1; 
  int addr, val;

  /* By default using vmaid = 0 */
  val = __alloc(proc, 0, reg_index, size, &addr);

  /* TODO update TLB CACHED frame num of the new allocated page(s)*/
  /* by using tlb_cache_read()/tlb_cache_write()*/

  //get the regs check 
  proc->regs[reg_index] = addr; 

  //check for write
  //get total space required 
  int inc_amt = PAGING_PAGE_ALIGNSZ(size);

  //get total page need 
  int incnumpage =  inc_amt / PAGING_PAGESZ;

  int i; 

  //allocate the page 
  for( i = 0; i < incnumpage; i++) {

    uint32_t new_addr = addr+i*PAGING_PAGESZ;

    //get page
    int pgn = PAGING_PGN(new_addr);

    //printf("pgn from alloc: %d\n",pgn);

    //get the PTE
    uint32_t pte = proc->mm->pgd[pgn];  
    //printf("pid: ,png of address: %d, having PTE: %u\n",pgn,pte);

    //write the data on 
    if(tlb_cache_write(proc->tlb,proc->pid,pgn,pte) == -1) return -1; 

  }

  TLBMEMPHY_dump(proc->tlb);

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

  //Test free() 
  //printf("DEBUG: =====\n Free regionID: %d, of procID: %d\n",reg_index,proc->pid);

  //get the reg_start  
  int reg_start = proc->mm->symrgtbl[reg_index].rg_start; 

  //get the reg_end 
  int reg_end = proc->mm->symrgtbl[reg_index].rg_end; 

  if(reg_end == 0) {printf("Aborted - Free region that is not allocated!\n"); return -1;} 

  //get the size 
  int size = reg_end - reg_start;  

  //get number of page 
  int nbpg = size / PAGING_PAGESZ; 

  //check state 
  int pgn = PAGING_PGN(reg_start); 
  int i; 
  BYTE dummy;
  for(i = 0; i < nbpg; i++){
      if(tlb_cache_read(proc->tlb,proc->pid,pgn + i,&dummy) >= 0){
          int szof = proc->tlb->maxsz / 10; 
          int tlbnb = (pgn + i) % szof;
          flush_rg(proc->tlb,tlbnb); 
      }
  } 

  //get every thing done by it's nature 
  proc->mm->symrgtbl[reg_index].rg_start = 0; 
  proc->mm->symrgtbl[reg_index].rg_end = 0; 

  //recheck 
  // int des; 
  // printf("==================check_AFTER_FREE=====================\n");
  // for(des = 0; des < PAGING_MAX_SYMTBL_SZ; des++){
  //   int reg_start = proc->mm->symrgtbl[des].rg_start; 
  //   int reg_end = proc->mm->symrgtbl[des].rg_end; 
  //   printf("regID: %d, reg_start: %d, reg_end: %d\n",des,reg_start,reg_end);
  // }
  // printf("======================DONE============================\n");

  return 0;
}

/*tlbread - CPU TLB-based read a region memory
 *@proc: Process executing the instruction
 *@source: index of source register
 *@offset: source address = [source] + [offset]
 *@destination: destination storage
 */
int tlbread(struct pcb_t * proc, uint32_t source,
            uint32_t offset,  uint32_t destination) 
{
  //done 
  printf("TLB-read function entered by PID: %d, at the source: %d and offset: %d\n",proc->pid,source,offset);
  BYTE data; 
  int frmnum = -1;
  BYTE check = 0; 

  int addr = proc->regs[source] + offset;

  //CHECK IF IT IS ALLOC OR NOT !!!!!!!!!!!!!
  struct vm_rg_struct *alloc_loc = &proc->mm->symrgtbl[destination]; 

  if(alloc_loc->rg_end == alloc_loc->rg_start || alloc_loc->rg_end == 0) {printf("Segmentation Fault - Accessing the address is not allocated\n"); return -1;}
  
  int des; int good = -1; 
  for(des = 0; des < PAGING_MAX_SYMTBL_SZ; des++){
    int reg_start = proc->mm->symrgtbl[des].rg_start; 
    int reg_end = proc->mm->symrgtbl[des].rg_end; 

    if (addr <= reg_end && addr >= reg_start) {
        good = 1; 
        break; 
    }
  }
  if(good == -1) {printf("Segmentation Fault - Accessing the address is not allocated\n"); return -1;}

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
      printf("Need to getpage \n\n\n"); 

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
           printf (
           "Old value PLB of PID: %d, PNG: %d, having PTE: %u\n New value PLB of PID: %d, PNG: %d, having PTE: %u\n",tlb_pid(proc->tlb,pgn),pgn,tlb_pte(proc->tlb,pgn), proc->pid,pgn,pte
            );

          if(tlb_cache_write(proc->tlb,proc->pid,pgn,pte) == -1) 
            return -1; /*cannot write~*/
  } 

  TLBMEMPHY_dump(proc->tlb);
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
  printf("TLB-write function entered by PID: %d, at the source: %d and offset: %d to records: %c\n",proc->pid,destination,offset,data);
  int val;
  int frmnum = -1;
  BYTE check = 0; 

  int addr = proc->regs[destination] + offset;

  //CHECK IF IT IS ALLOC OR NOT !!!!!!!!!!!!!
  struct vm_rg_struct *alloc_loc = &proc->mm->symrgtbl[destination]; 

  if(alloc_loc->rg_end == alloc_loc->rg_start || alloc_loc->rg_end == 0) {printf("Segmentation Fault - Address is not allocated\n"); return -1;}
  
  int des; int good = -1; 
  for(des = 0; des < PAGING_MAX_SYMTBL_SZ; des++){
    int reg_start = proc->mm->symrgtbl[des].rg_start; 
    int reg_end = proc->mm->symrgtbl[des].rg_end; 

    if (addr <= reg_end && addr >= reg_start) {
        good = 1; 
        break; 
    }
  }
  if(good == -1) {printf("Segmentation Fault - Address is not allocated\n"); return -1;}

  //getting the page number and offset 
    int pgn = PAGING_PGN(addr);
    int off = PAGING_OFFST(addr);
  printf("PGN: %d\n", pgn);
 
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
      printf (
        "Old value PLB of PID: %d, PNG: %d, having PTE: %u\n New value PLB of PID: %d, PNG: %d, having PTE: %u\n",tlb_pid(proc->tlb,pgn),pgn,tlb_pte(proc->tlb,pgn), proc->pid,pgn,pte
      );

      //perform writting on cache 
        if(tlb_cache_write(proc->tlb,proc->pid,pgn,pte) == -1) 
            return -1; /*cannot write~*/
  }

  TLBMEMPHY_dump(proc->tlb);
  return val;
}

#endif