/**
 * @file
 * Lamport's CLF SPSC queue type definitions and function declarations
 */
#ifndef LAM_QUEUE_H_
#define LAM_QUEUE_H_

#define LAM_WOULDBLOCK 2

/**
 * Single-Producer-Single-Consumer array-based bounded queue
 */ 
typedef struct lam_queue_st {
    //! head index
    unsigned int head __attribute__ ((aligned (128)));

    //! tail index
    unsigned int tail __attribute__ ((aligned (128)));
    
    //! queue size
    unsigned int size __attribute__ ((aligned (128))); 

    //! the actual queue implemented as an array
    //! each entry holds the address to the "payload" 
    unsigned long *buffer;

} lam_queue_t;

extern void lam_init(lam_queue_t *q, int size);
extern int lam_enqueue(lam_queue_t *q, void *data);
extern int lam_dequeue(lam_queue_t *q, void **data);
extern void lam_destroy(lam_queue_t *q);
extern void lam_print(lam_queue_t *q);

#endif
