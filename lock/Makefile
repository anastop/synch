INCLUDE_DIR = ../ 
LIBRARY_DIR = ./
UTIL_PARENT = ../../

CC = gcc
CFLAGS = -O3 -Wall  
LDGLAGS = 
LIBS = -lpthread

CFLAGS += -I$(INCLUDE_DIR) -I$(UTIL_PARENT)

PROGRAMS = locks_scalability 

all : $(PROGRAMS)

locks_scalability : processor_map.o util.o locks_scalability.o 
	$(CC) $(LDFLAGS) processor_map.o util.o locks_scalability.o -o locks_scalability -L$(LIBRARY_DIR) $(LIBS)   

util.o : $(UTIL_PARENT)/util/util.c
	$(CC) $(CFLAGS) -c $(UTIL_PARENT)/util/util.c

processor_map.o : $(UTIL_PARENT)/util/processor_map.c
	$(CC) $(CFLAGS) -c $(UTIL_PARENT)/util/processor_map.c

%.o : %.c
	$(CC) $(CFLAGS) -c $<

clean :
	rm -f $(PROGRAMS) *.o 
