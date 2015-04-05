/**
 * @file
 * Lamport's CLF SPSC queue function definitions
 */

#include "lam_queue.h"

#include <stdio.h>
#include <stdlib.h>

/**
 * Allocates queue structure and buffer
 * @param q queue handler
 * @param size queue size
 */  
void lam_init(lam_queue_t *q, int size)
{
    int i;

    q->buffer = (unsigned long*)malloc(sizeof(unsigned long)*size);
    if ( !q->buffer ) {
        fprintf(stderr, "%s: Allocation error\n", __FUNCTION__);
        exit(EXIT_FAILURE);
    }
    // 0 represents an empty slot (e.g. NULL pointer)
    for ( i = 0; i < size; i++ ) q->buffer[i] = 0;

    q->size = size;
    q->head = q->tail = 0;
}

/**
 * Enqueues an element 
 * @param q queue handler
 * @param data address of data to be enqueued 
 * @return 0 if successful, LAM_WOULDBLOCK if queue is full
 */ 
inline int lam_enqueue(lam_queue_t *q, void *data)
{
    int next_head;

    next_head = ( (q->head+1) < q->size ) ? q->head+1 : 0;
    if ( next_head == q->tail )
       return LAM_WOULDBLOCK;
    
    q->buffer[q->head] = (unsigned long)data;
    q->head = next_head;

    return 0;
}

/**
 * Dequeues an element
 * @param q queue handler
 * @param data address of placeholder for dequeued data
 * @return 0 if successful, LAM_WOULDBLOCK if queue is empty
 */ 
inline int lam_dequeue(lam_queue_t *q, void **data)
{
    if ( q->head == q->tail )
       return LAM_WOULDBLOCK;

    *data = (void*)q->buffer[q->tail];

    // tail = NEXT(tail)
    q->tail++;
    if ( q->tail == q->size ) q->tail = 0;
        
    return 0;
}

/**
 * Prints queue contents
 * @param q queue handler
 */ 
void lam_print(lam_queue_t *q)
{
    int i;

    fprintf(stderr, "[");
    for ( i = 0; i < q->size; i++ ) {
        if ( i == q->tail ) fprintf(stderr,"t>");
        if ( i == q->head ) fprintf(stderr, "h>");
        fprintf(stderr, "%lu ", q->buffer[i]);
    }
    fprintf(stderr, "]\n");
}
