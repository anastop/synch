INCLUDE_DIR = ../ 
LIBRARY_DIR = ./
UTIL_PARENT = ../../

CC = gcc
CFLAGS = -O3 -Wall 
LDGLAGS = 
LIBS = -lpthread

CFLAGS += -I$(INCLUDE_DIR) -I$(UTIL_PARENT)

PROGRAMS = ffq_test lamq_test mt_test

all : $(PROGRAMS)

ffq_test : ff_queue.o ff_queue_unit_test.o 
	$(CC) $(LDFLAGS) ff_queue.o ff_queue_unit_test.o -o ffq_test -L$(LIBRARY_DIR) $(LIBS)

lamq_test : lam_queue.o lam_queue_unit_test.o 
	$(CC) $(LDFLAGS) lam_queue.o lam_queue_unit_test.o -o lamq_test -L$(LIBRARY_DIR) $(LIBS)

mt_test : ff_queue.o lam_queue.o mt_queue_test.o util.o processor_map.o 
	$(CC) $(LDFLAGS) ff_queue.o lam_queue.o  mt_queue_test.o util.o processor_map.o -o mt_test -L$(LIBRARY_DIR) $(LIBS)

util.o : $(UTIL_PARENT)/util/util.c
	$(CC) $(CFLAGS) -c $(UTIL_PARENT)/util/util.c

processor_map.o : $(UTIL_PARENT)/util/processor_map.c
	$(CC) $(CFLAGS) -c $(UTIL_PARENT)/util/processor_map.c

%.o : %.c %.h 
	$(CC) $(CFLAGS) -c $<

clean :
	rm -f $(PROGRAMS) *.o
