#ifndef OSMM_H
#define OSMM_H

#define MM_PAGING
#define PAGING_MAX_MMSWP 4 /* max number of supported swapped space */
#define PAGING_MAX_SYMTBL_SZ 30

#include <pthread.h>
#include <sys/types.h>  

typedef char BYTE;
typedef uint32_t addr_t;
//typedef unsigned int uint32_t;

struct pgn_t{
   int pgn; // page number
   struct pgn_t *pg_next; 
};

/*
 *  Memory region struct
 */
struct vm_rg_struct {
   unsigned long rg_start; // region start
   unsigned long rg_end; // region end

   struct vm_rg_struct *rg_next;
};

/*
 *  Memory area struct
 */
struct vm_area_struct {
   unsigned long vm_id; // memory id
   unsigned long vm_start; // memory start
   unsigned long vm_end; // memory end

   unsigned long sbrk; // break point
/*
 * Derived field
 * unsigned long vm_limit = vm_end - vm_start
 */
   struct mm_struct *vm_mm; // memory management
   struct vm_rg_struct *vm_freerg_list; // free region list
   struct vm_area_struct *vm_next; // next memory area
};

/* 
 * Memory management struct
 */
struct mm_struct {
   uint32_t *pgd; // page directory

   struct vm_area_struct *mmap; // memory map

   /* Currently we support a fixed number of symbol */
   struct vm_rg_struct symrgtbl[PAGING_MAX_SYMTBL_SZ]; // symbol region table

   pthread_mutex_t lock;

   /* list of free page */
   struct pgn_t *fifo_pgn; // first in first out page number
};

/*
 * FRAME/MEM PHY struct
 */
struct framephy_struct { 
   int fpn;
   struct framephy_struct *fp_next; // next frame

   /* Resereed for tracking allocated framed */
   struct mm_struct* owner; // owner of the frame
};

struct memphy_struct {
   /* Basic field of data and size */
   BYTE *storage;
   int maxsz; 
   
   /* Sequential device fields */ 
   int rdmflg; // read mode flag
   int cursor; // read cursor

   /* Management structure */
   struct framephy_struct *free_fp_list; // free frame list
   struct framephy_struct *used_fp_list; // used frame list
   
   struct framephy_struct *fifo_fp_list; // first in first out frame list

   /*Mutex Lock*/
   pthread_mutex_t lock;
   pthread_mutex_t fifo_lock;
   
};

#endif
