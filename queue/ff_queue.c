/**
 * @file
 * Fast-forward SPSC queue function definitions
 * See Giacomoni et. al., PPoPP08
 */

#include "ff_queue.h"

#include <stdio.h>
#include <stdlib.h>

/**
 * Allocates queue structure and buffer
 * @param q queue handler
 * @param size queue size
 */  
void ff_init(ff_queue_t *q, int size)
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
 * @return 0 if successful, FF_WOULDBLOCK if queue is full
 */ 
inline int ff_enqueue(ff_queue_t *q, void *data)
{
    if ( q->buffer[q->head] != 0 ) 
       return FF_WOULDBLOCK;
    
    q->buffer[q->head] = (unsigned long)data;

    // head = NEXT(head)
    q->head++;
    if ( q->head == q->size ) q->head = 0;

    return 0;
}

/**
 * Dequeues an element
 * @param q queue handler
 * @param data address of placeholder for dequeued data
 * @return 0 if successful, FF_WOULDBLOCK if queue is empty
 */ 
inline int ff_dequeue(ff_queue_t *q, void **data)
{
    *data = (void*)q->buffer[q->tail];
    if ( *data == 0 )
        return FF_WOULDBLOCK;

    q->buffer[q->tail] = 0;
    
    // tail = NEXT(tail)
    q->tail++;
    if ( q->tail == q->size ) q->tail = 0;
        
    return 0;
}

/**
 * Prints queue contents
 * @param q queue handler
 */ 
void ff_print(ff_queue_t *q)
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
