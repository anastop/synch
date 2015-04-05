#include <stdio.h>

#include "ff_queue.h"
 
int main(int argc, char **argv)
{
    char input[10] = "abcdefghij";
    char* out;
    int ret, next = 0;

    ff_queue_t q;
    ff_init(&q, 5);

    ff_print(&q);

    for (;;) {
        fprintf(stderr, "\nEnqueing %c...", input[next]); 
        ret = ff_enqueue(&q, (void*)&input[next]);
        if ( ret == FF_WOULDBLOCK ) {
            fprintf(stderr, "Queue is full\n");
            break;
        }
        fprintf(stderr, "OK\n");
        ff_print(&q);
        next++;
    } 
    ff_print(&q);
       
    for (;;) {
        fprintf(stderr, "\nDequeing...");
        ret = ff_dequeue(&q, (void*)&out);
        if ( ret == FF_WOULDBLOCK ) {
            fprintf(stderr, "Queue is empty\n");
            break;
        }
        fprintf(stderr, "OK, val=%c\n", *out);
        ff_print(&q);
    } 
    ff_print(&q);

    return 0;
}
