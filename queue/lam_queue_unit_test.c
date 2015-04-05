#include <stdio.h>

#include "lam_queue.h"
 
int main(int argc, char **argv)
{
    char input[10] = "abcdefghij";
    char* out;
    int ret, next = 0;

    lam_queue_t q;
    lam_init(&q, 5);

    lam_print(&q);

    for (;;) {
        fprintf(stderr, "\nEnqueing %c...", input[next]); 
        ret = lam_enqueue(&q, (void*)&input[next]);
        if ( ret == LAM_WOULDBLOCK ) {
            fprintf(stderr, "Queue is full\n");
            break;
        }
        fprintf(stderr, "OK\n");
        lam_print(&q);
        next++;
    } 
    lam_print(&q);
       
    for (;;) {
        fprintf(stderr, "\nDequeing...");
        ret = lam_dequeue(&q, (void*)&out);
        if ( ret == LAM_WOULDBLOCK ) {
            fprintf(stderr, "Queue is empty\n");
            break;
        }
        fprintf(stderr, "OK, val=%c\n", *out);
        lam_print(&q);
    } 
    lam_print(&q);

    return 0;
}
