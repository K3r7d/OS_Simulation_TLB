/*
 * Copyright (C) 2024 pdnguyen of the HCMC University of Technology
 */
/*
 * Source Code License Grant: Authors hereby grants to Licensee 
 * a personal to use and modify the Licensed Source Code for 
 * the sole purpose of studying during attending the course CO2018.
 */

/*
 * Memory physical based TLB Cache
 * TLB cache module tlb/tlbcache.c
 *
 * TLB cache is physically memory phy
 * supports random access 
 * and runs at high speed
 */


#include "mm.h"
#include <stdlib.h>
#include <stdio.h>
#ifdef CPU_TLB
#define init_tlbcache(mp,sz,...) init_memphy(mp, sz, (1, ##__VA_ARGS__))

/*
 *  tlb_cache_read read TLB cache device
 *  @mp: memphy struct
 *  @pid: process id
 *  @pgnum: page number
 *  @value: obtained value
 */

 // 0x AA BB CC DD: PID (4 bytes) 
 // 0x AA BB: Pgnumber (14 bits store in 16 bits or 2 byte) 
 // 0x AA BB CC DD: PTE (32 bits or 4 bytes)
void flush_rg(struct memphy_struct *mp, int tlbnb){
   if(tlbnb*10 + 10 >= mp->maxsz) return; 
   int index; 
   for(index = 0; index < 10; index++){
      mp->storage[tlbnb*10 + index] = 0; 
   } 

} 

uint32_t tlb_pid(struct memphy_struct *mp, int pgnum){ 
   int szof = mp->maxsz / 10; 
   int tlbnb = pgnum % szof;
   return mp->storage[tlbnb] << 24 | mp->storage[tlbnb + 1] << 16 | mp->storage[tlbnb + 2] << 8 | mp->storage[tlbnb + 3]; 
}

uint32_t tlb_pgn(struct memphy_struct *mp, int pgnum){ 
   int szof = mp->maxsz / 10; 
   int tlbnb = pgnum % szof;
   return mp->storage[tlbnb + 4] << 8 | mp->storage[tlbnb + 5];
}

uint32_t tlb_pte(struct memphy_struct *mp, int pgnum){ 
   int szof = mp->maxsz / 10; 
   int tlbnb = pgnum % szof;
   return mp->storage[tlbnb + 6] << 24 | mp->storage[tlbnb + 7] << 16 | mp->storage[tlbnb + 8] << 8 | mp->storage[tlbnb + 9];
}

int tlb_cache_read(struct memphy_struct * mp, int pid, int pgnum, BYTE *data)
{
   /* TODO: the identify info is mapped to 
    *      cache line by employing:
    *      direct mapped, associated mapping etc.
    */

   //check if the PLB exists 
   if(mp == NULL) return -1; 

   //implement the jump parameter, 10 storing byte  
   int szof = mp->maxsz / 10; 
   int tlbnb = pgnum % szof;

   //get the stored value in tlb 
   int plb_pid = mp->storage[tlbnb*10] << 24 | mp->storage[tlbnb*10 + 1] << 16 | mp->storage[tlbnb*10 + 2] << 8 | mp->storage[tlbnb*10 + 3];

   //get page number 
   int plb_pgnum = mp->storage[tlbnb*10 + 4] << 8 | mp->storage[tlbnb*10 + 5];

   //get PTE 
   int plb_pte = mp->storage[tlbnb*10 + 6] << 24 | mp->storage[tlbnb*10 + 7] << 16 | mp->storage[tlbnb*10 + 8] << 8 | mp->storage[tlbnb*10 + 9];

   int i;
   //READ
   // printf("-----------TLB CACHE READ:\n");
   // for(i = 0; i < 10;i++) printf("mp->storage[%d]: %02u\n",tlbnb*10+i, mp->storage[tlbnb*10+i] & 0xff);
   //check_again  

   if(pid == plb_pid) {
      if(pgnum == plb_pgnum) {

         //check if page is exists on RAM, if not then pg_getpage 
         if(!PAGING_PAGE_PRESENT(plb_pte)) {
            *data = -1; 
            return -1;
         }

         //get frame number
         int frgnb = PAGING_FPN(plb_pte);

         //return the frame number 
         return frgnb; 

      }
   }
   return -1;    
}

/*
 *  tlb_cache_write write TLB cache device
 *  @mp: memphy struct
 *  @pid: process id
 *  @pgnum: page number
 *  @value: obtained value
 */
int tlb_cache_write(struct memphy_struct *mp,  
                   int pid, 
                   int pgnum, 
                   int pte
                   )
{
   /* TODO: the identify info is mapped to 
    *      cache line by employing:
    *      direct mapped, associated mapping etc.
    */
   
    //check if the PLB exists 
   if(mp == NULL || pgnum < 0) return -1; 

   //implement the jump parameter, 10 storing byte  
   int szof = mp->maxsz / 10; 
   int tlbnb = pgnum % szof;
   // printf("tlbnb: %d\n",tlbnb*10);

        //perform of writing pid onto PLB 
        mp->storage[tlbnb*10] = (pid >> 24) & 0xff; //HIGH byte of holding PLB
        mp->storage[tlbnb*10 + 1] = (pid >> 16) & 0xff; 
        mp->storage[tlbnb*10 + 2] = (pid >> 8) & 0xff; 
        mp->storage[tlbnb*10 + 3] = (pid) & 0xff; //LOW byte of holding PLB


        //HOLDING of pg number 
        mp->storage[tlbnb*10 + 4] = (pgnum >> 8) & 0xff; //HIGH byte holding pg number 
        mp->storage[tlbnb*10 + 5] = (pgnum) & 0xff;  //LOW byte holding pg number 

        //Holding of PTE
        mp->storage[tlbnb*10 + 6] = (pte >> 24) & 0xff; //HIGH byte holding pg number 
        mp->storage[tlbnb*10 + 7] = (pte >> 16) & 0xff;  
        mp->storage[tlbnb*10 + 8] = (pte >> 8) & 0xff;
        mp->storage[tlbnb*10 + 9] = (pte) & 0xff;  //LOW byte holding pg number 

      //   printf("PTE: %d\n",pte);
      //   printf("The data: %d %d %d %d\n",(pte >> 24) & 0xff,(pte >> 16) & 0xff,(pte >> 8) & 0xff,(pte) & 0xff);
      //   printf("It records: %d\n",mp->storage[tlbnb*10 + 6] << 24 | mp->storage[tlbnb*10 + 7] << 16 | mp->storage[tlbnb*10 + 8] << 8 | mp->storage[tlbnb*10 + 9]);
   return 0; 
}

/*
 *  TLBMEMPHY_read natively supports MEMPHY device interfaces
 *  @mp: memphy struct
 *  @addr: address
 *  @value: obtained value 
 */
int TLBMEMPHY_read(struct memphy_struct * mp, int addr, BYTE *value)
{
   if (mp == NULL)
     return -1;

   /* TLB cached is random access by native */
   *value = mp->storage[addr];

   return 0;
}


/*
 *  TLBMEMPHY_write natively supports MEMPHY device interfaces
 *  @mp: memphy struct
 *  @addr: address
 *  @data: written data
 */
int TLBMEMPHY_write(struct memphy_struct * mp, int addr, BYTE data)
{
   if (mp == NULL)
     return -1;

   /* TLB cached is random access by native */
   mp->storage[addr] = data;
   return 0;
}

/*
 *  TLBMEMPHY_format natively supports MEMPHY device interfaces, use for debug
 *  @mp: memphy struct
 */

int TLBMEMPHY_dump(struct memphy_struct * mp)
{
   /*TODO dump memphy contnt mp->storage 
    *     for tracing the memory content
    */
   printf("==============TLB DUMP!===============:\n"); 
   int nbpg = mp->maxsz / 10;
   int i; int j; 
   for(i = 0 ; i < 5; i++){ 
      for(j = 0; j < 10; j++) printf(" mp[%02d]: %u |",i*10+j,mp->storage[i*10 + j] & 0xff);
      printf("\n");
   }
   printf("=======================================\n");
   return 0;
}


/*
 *  Init TLBMEMPHY struct
 */
int init_tlbmemphy(struct memphy_struct *mp, int max_size)
{
   mp->storage = (BYTE *)malloc(max_size*sizeof(BYTE));
   mp->maxsz = max_size;

   mp->rdmflg = 1;

   return 0;
}

#endif