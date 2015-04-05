#ifndef LOCK_H_
#define LOCK_H_

typedef volatile unsigned int spinlock_t;

#define SPIN_LOCK_UNLOCKED  1 
#define SPIN_LOCK_LOCKED    0

static inline void spin_lock_init(spinlock_t *spin_var)
{
    *spin_var = SPIN_LOCK_UNLOCKED; 
}

static inline void spin_lock(spinlock_t *spin_var)
{
    __asm__ __volatile__ ("\n"
        "1: lock; decl %0\n\t"
        "jne 2f\n\t"
        "2:\t cmpl $0, %0\n\t"
        "jg 1b\n\t"
        "jmp 2b\n\t"
        : "=m" (*spin_var) 
        : "m" (*spin_var) 
        : "memory");

}

static inline void spin_unlock(spinlock_t *spin_var)
{
    __asm__ __volatile__("movl $1,%0" 
            :"=m" (*spin_var) 
            :
            : "memory");
}

static inline void spin_lock_aligned(spinlock_t *spin_var)
{
    __asm__ __volatile__ ("\n"
        "1: lock; decl %0\n\t"
        "jne 2f\n\t"
        ".subsection 1\n\t"
        ".align 16\n"
        "2:\t cmpl $0, %0\n\t"
        "jg 1b\n\t"
        "jmp 2b\n\t"
        ".previous"
        : "=m" (*spin_var) 
        : "m" (*spin_var) 
        : "memory");
}

static inline void spin_lock_aligned_pause(spinlock_t *spin_var)
{
    __asm__ __volatile__ ("\n"
        "1: lock; decl %0\n\t"
        "jne 2f\n\t"
        ".subsection 1\n\t"
        ".align 16\n"
        "2:\t pause\n\t"
        "cmpl $0, %0\n\t"
        "jg 1b\n\t"
        "jmp 2b\n\t"
        ".previous"
        : "=m" (*spin_var) 
        : "m" (*spin_var) 
        : "memory");
}

/*
 *  All threads use a swap instruction that atomically
 *  _reads the old value of spin_var and stores a 0 to the
 *  spin_var_.  The single winner will see the 1, and the
 *  losers will see a 0 that was placed there by the winner
 *  (the losers will continue to set the spin_var to 0 but
 *  that doesn't matter). When the winner finishes with the
 *  critical section, it sets the spin_var to 1 to release
 *  the lock.
 *
 */ 

static inline void spin_lock_cas(spinlock_t *spin_var)
{
    __asm__ __volatile__ ( "\n"
        "1:\t movl $0, %%eax\n\t"            
        // Atomically set spin_var=0 and read its previous value.
        "xchgl %0,%%eax\n\t"  
        
        // 1st check to avoid the spinning in case we can 
        // immediately grab the lock
        "cmpl $1,%%eax\n\t"   
        
        // Is lock free?  
        "jne 2f\n\t" // No, go to spin.  
        "jmp 3f\n\t"   // Yes, we have acquired the lock! 

        // Spin loop. Place it in a separate section
        ".subsection 1\n\t"
        ".align 16\n"
        "2:\t cmpl $1,%0\n\t"    
        "jne 2b\n\t"  
        // If lock free, go back to 1 and to try to lock again
        // This time we should succeed.
        "jmp 1b\n\t" 
        ".previous\n\t"
        // Enter critical section
        "3:\n\t"   
        : "=m" (*spin_var) 
        : "m" (*spin_var)
        : "%eax","memory");
}


static inline void spin_lock_cas_pause(spinlock_t *spin_var)
{
    __asm__ __volatile__ ( "\n"
        "1:\t movl $0, %%eax\n\t"            
        "xchgl %0,%%eax\n\t"  
        "cmpl $1,%%eax\n\t"   
        "jne 2f\n\t" 
        "jmp 3f\n\t"   
        ".subsection 1\n\t"
        ".align 16\n"
        "2:\tpause\n\t" 
        "cmpl $1,%0\n\t"    
        "jne 2b\n\t"  
        "jmp 1b\n\t" 
        ".previous\n\t"
        "3:\n\t"   
        : "=m" (*spin_var) 
        : "m" (*spin_var)
        : "%eax","memory");
}


#endif
