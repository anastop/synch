/**
 * @file
 * Performance comparison of SPSC queues in multithredaded mode
 *
 * A 3-stage looped pipeline is implemented. Each stage executes on 
 * a different processor, and queues are used to interconnect stages. 
 */ 

#define _GNU_SOURCE

#include <assert.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "ff_queue.h"
#include "lam_queue.h"
#include "util/tsc_x86_64.h"
#include "util/processor_map.h"
#include "util/util.h"

pthread_barrier_t bar;
tsctimer_t tim;

// number of pipeline stages
int nstages = 3;

// number of queue entries
int queue_size;

// number of iterations (dequeue / spin / enqueue)
unsigned long niters;

// local work nanoseconds
unsigned long delay_nanosecs;
// local work in cycles
unsigned long delay_cycles;

// queue handlers
ff_queue_t ffq[3];
lam_queue_t lamq[3];

// pointer to queue contents
char *data;

typedef struct {
    int id;
} targs_t;

void* stage_ff(void *args)
{
    unsigned long i = 0;
    int ret, in_q, out_q;
    char* item;
    targs_t *ta = (targs_t*)args;

    in_q = ta->id;
    out_q = (ta->id + 1 < nstages ? ta->id + 1 : 0 );

    pthread_barrier_wait(&bar);
    if ( ta->id == 0 ) timer_start(&tim);

    while ( i++ < niters ) {
        while ( (ret = ff_dequeue(&ffq[in_q], (void*)&item)) ) ;
        spin_for_cycles(delay_cycles);
        while ( (ret = ff_enqueue(&ffq[out_q], (void*)&item)) ) ;
    }

    pthread_barrier_wait(&bar);
    if ( ta->id == 0 ) timer_stop(&tim);
    
    pthread_exit(NULL);
}

void* stage_lam(void *args)
{
    unsigned long i = 0;
    int ret, in_q, out_q;
    char* item;
    targs_t *ta = (targs_t*)args;

    in_q = ta->id;
    out_q = (ta->id + 1 < nstages ? ta->id + 1 : 0 );

    pthread_barrier_wait(&bar);
    if ( ta->id == 0 ) timer_start(&tim);

    while ( i++ < niters ) {
        while ( (ret = lam_dequeue(&lamq[in_q], (void*)&item)) ) ;
        spin_for_cycles(delay_cycles);
        while ( (ret = lam_enqueue(&lamq[out_q], (void*)&item)) ) ;
    }

    pthread_barrier_wait(&bar);
    if ( ta->id == 0 ) timer_stop(&tim);
    
    pthread_exit(NULL);
}

typedef struct {
    void* (*func)(void*);
    char *name;
} tfunc_t;

#define INIT_FUNC(f) {.func = f, .name = #f}

tfunc_t impl[] = {
    INIT_FUNC(stage_ff),
    INIT_FUNC(stage_lam)
};

int main(int argc, char **argv)
{
    targs_t *targs;
    pthread_t *tids;
    pthread_attr_t *attr;
    procmap_t *pi;
    int p, c, t, i, f, population;
  
    if ( argc < 4 ) {
        printf("Usage: ./prog <queue_size> <iters> <nanosecs_to_spin>\n");
        exit(EXIT_FAILURE);
    }

    queue_size = atoi(argv[1]);
    niters = atoi(argv[2]);
    delay_nanosecs = atoi(argv[3]);
    delay_cycles = (unsigned long)((double)delay_nanosecs* timer_read_hz() 
                                  / 1000000000.0);
    
    // Initialize queues
    assert (queue_size > 16);
    population = queue_size - 16;
    for ( i = 0; i < nstages; i++ ) {
        ff_init(&ffq[i], queue_size);
        lam_init(&lamq[i], queue_size);
    }
    
    data = (char*)malloc_safe(population * sizeof(char));
    
    // The 1st queue is populated with pointers to all
    // characters of 'data' array. Pointers are copied to
    // queue entries.
    for ( i = 0; i < population; i++ ) data[i] = i;

    for ( i = 0; i < population; i++ ) {
        int ret = ff_enqueue(&ffq[0], (void*)&data[i]);
        if ( ret == FF_WOULDBLOCK ) {
            fprintf(stderr, "Queue is full. Exiting\n");
            exit(EXIT_FAILURE);
        }
    }
    for ( i = 0; i < population; i++ ) {
        int ret = lam_enqueue(&lamq[0], (void*)&data[i]);
        if ( ret == LAM_WOULDBLOCK ) {
            fprintf(stderr, "Queue is full. Exiting\n");
            exit(EXIT_FAILURE);
        }
    }

    // Configure thread affinity: first fill cores, then packages, 
    // and last peer threads
    pi = procmap_init();
    cpu_set_t cpusets[pi->num_cpus];

    i = 0;
    fprintf(stdout, "Thread mapping:\n");
    for ( t = 0; t < pi->num_threads_per_core; t++ ) {
        for ( c = 0; c < pi->num_cores_per_package; c++ ) {
            for ( p = 0; p < pi->num_packages; p++ ) {
                int cpu_id = pi->package[p].core[c].thread[t]->cpu_id;
                CPU_ZERO(&cpusets[i]);
                CPU_SET(cpu_id, &cpusets[i]);

                fprintf(stdout, "Thread %d @ package %d, core %d, "
                                "hw thread %d (cpuid: %d)\n",
                                i, p, c, t, cpu_id);
                i++;
            }
        }    
    }
    fprintf(stdout, "\n");

    // Allocate thread structures 
    tids = (pthread_t*)malloc_safe( nstages * sizeof(pthread_t) );
    targs = (targs_t*)malloc_safe( nstages * sizeof(targs_t)); 
    attr = (pthread_attr_t*)malloc_safe( nstages * sizeof(pthread_attr_t)); 
    pthread_barrier_init(&bar, NULL, nstages);

    for ( f = 0; f < 2; f++ ) {
        timer_clear(&tim);

        // Create threads
        for ( i = 0; i < nstages; i++ ) {
            targs[i].id = i;
            pthread_attr_init(&attr[i]);
            pthread_attr_setaffinity_np(&attr[i], 
                                        sizeof(cpusets[i]), 
                                        &cpusets[i]);
            pthread_create(&tids[i], 
                           &attr[i], 
                           impl[f].func, 
                           (void*)&targs[i]);
        }
        for ( i = 0; i < nstages; i++ ) {
            pthread_join(tids[i], NULL);
            pthread_attr_destroy(&attr[i]);
        }
        
        fprintf(stdout, "Queue:%s queue_size:%d iters:%lu" 
                        " nsecs_to_spin:%lu cycles_to_spin:%lu" 
                        " cycles_per_iter:%lf cycles_per_iter_wo_delay:%lf\n", 
                        impl[f].name, queue_size, niters,
                        delay_nanosecs, delay_cycles,
                        timer_total(&tim)/niters, 
                        timer_total(&tim)/niters - delay_cycles );
    }
            
    // Clean-up things
    pthread_barrier_destroy(&bar);
    free(tids);
    free(targs);
    free(attr);
    procmap_destroy(pi); 

    return 0;
}
