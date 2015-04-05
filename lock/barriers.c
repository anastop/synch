#ifndef LOCKS_H_
#define LOCKS_H_

#include <asm/unistd.h>

typedef volatile unsigned int spin_t;

#define SPIN_LOCK_UNLOCKED  1 
#define SPIN_LOCK_LOCKED    0

typedef struct spin_barrier_s {
    spin_t lock;
    spin_t release_flag;
    int current_count;
    int nthreads;
} spin_barrier_t;

/*
 * with local sense (per-thread variable)
 *
 * example:
 *  spin_barrier_t gbarrier;    //global
 *  ...
 *  spin_barrier_init(&gbarrier, nthreads); //init global barrier
 *  for(i=0; i<nthreads; i++)               //init local sense
 *      spin_barrier_init_lsense(&targs[i].lsense);
 */ 
static inline void spin_barrier_init(spin_barrier_t *sb, int nthreads)
{
    spin_lock_init_fast(&(sb->lock));
    sb->release_flag = 1;
    sb->current_count = 0;
    sb->nthreads = nthreads;
}

//lsense: local var to each thread
static inline void spin_barrier_init_lsense(unsigned int *lsense)
{
    *lsense = 1;
}


#if 0
shared int count = P;
shared bool sense = true;
private bool local_sense = true;

void central_barrier() {
    local_sense = !local_sense;

    if (fetch_and_decrement(&count) == 1) {
        count = P;
        sense = local_sense;
    } else {
        while (sense != local_sense);
    }
}
#endif


static inline void spin_barrier_lsense(spin_barrier_t *sb, unsigned int *lsense)
{
    *lsense = !(*lsense);

    __asm__ __volatile__(
            "lock; incl %0" 
            :"=m" (sb->current_count)
            :"m" (sb->current_count));

    if(sb->current_count == sb->nthreads) {
        sb->current_count = 0;
        sb->release_flag = *lsense;
    } else
        spin_on_condition(&(sb->release_flag), *lsense);
}


static inline void spin_barrier_lsense_cpuhalt(spin_barrier_t *sb, unsigned int *lsense)
{
    *lsense = !(*lsense);

    __asm__ __volatile__(
            "lock; incl %0" 
            :"=m" (sb->current_count)
            :"m" (sb->current_count));

    if(sb->current_count == sb->nthreads) {
        sb->current_count = 0;
        sb->release_flag = *lsense;
    } else
        spin_on_condition_cpuhalt(&(sb->release_flag), *lsense);
}

#include "cpuctrl.h"
extern int cpuctrl_fd;
static inline void spin_barrier_lsense_cpuhalt_sendIPI(spin_barrier_t *sb, unsigned int *lsense)
{
    *lsense = !(*lsense);

    __asm__ __volatile__(
            "lock; incl %0" 
            :"=m" (sb->current_count)
            :"m" (sb->current_count));

    if(sb->current_count == sb->nthreads) {
        sb->current_count = 0;
        sb->release_flag = *lsense;
        ioctl(cpuctrl_fd, CPUCTRL_IOC_WAKEUP_ALLBUTSELF, 0); 
    } else
        spin_on_condition_cpuhalt(&(sb->release_flag), *lsense);
}



/*
 * without local sense:
 * einai dynaton na prokalesei deadlock metaxy diadoxikwn klhsewn 
 * tou apo to idio thread, p.x. mesa se ena loop (vlepe CA book)
 * 
 *  lock(counterlock);      //ensure update atomic
 *  if(count==0)
 *      release_flag=0;         //first to come=> reset release_flag
 *  count++;                //count the arrivals
 *  unlock(counterlock);    //release lock
 *  if(count==nthreads) {       //all arived
 *      count=0;            //reset counter
 *      release_flag=1;         //release processes
 *  } else {
 *      spin(release_flag==1);  //wait for arrivals 
 *  }
 */
static inline void spin_barrier(spin_barrier_t *sb)
{
    spin_lock_fast(&(sb->lock));
    if(sb->current_count == 0) {
        sb->release_flag = 0;
    }
    (sb->current_count)++;
    spin_unlock_fast(&(sb->lock));

    if(sb->current_count == sb->nthreads) {
        sb->current_count = 0;
        sb->release_flag = 1;
    } else {
        spin_on_condition(&(sb->release_flag), 1);
    }       
}   

#endif
