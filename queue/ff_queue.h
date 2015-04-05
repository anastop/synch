/**
 * @file
 * Fast-forward SPSC queue type definitions and function declarations
 */
#ifndef FF_QUEUE_H_
#define FF_QUEUE_H_

#define FF_WOULDBLOCK 2

/**
 * Single-Producer-Single-Consumer array-based bounded queue
 */ 
typedef struct ff_queue_st {
    //! head index
    unsigned int head __attribute__ ((aligned (64)));

    //! tail index
    unsigned int tail __attribute__ ((aligned (64)));
    
    //! queue size
    unsigned int size __attribute__ ((aligned (64))); 

    //! the actual queue implemented as an array
    //! each entry holds the address to the "payload" 
    unsigned long *buffer; // __attribute__ ((aligned (64)));

} ff_queue_t;

extern void ff_init(ff_queue_t *q, int size);
extern int ff_enqueue(ff_queue_t *q, void *data);
extern int ff_dequeue(ff_queue_t *q, void **data);
extern void ff_destroy(ff_queue_t *q);
extern void ff_print(ff_queue_t *q);

#endif
