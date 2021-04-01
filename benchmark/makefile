#--------------------------------------------------- 
# Target file
#---------------------------------------------------
BENCHMARK = benchmark

#---------------------------------------------------
# Compiler
#---------------------------------------------------
CC = gcc

#---------------------------------------------------
# Options passed to the compiler
#---------------------------------------------------
CFLAGS = -Wall -std=gnu99
DBGFLAGS = -g -O0

#---------------------------------------------------
# Modules loaded
#---------------------------------------------------
LIB_RTS = -lrts
LIB_PTH = -lpthread
LIB_RTE = -lrdtsc

#--------------------------------------------------- 
# Target-specific variables
#---------------------------------------------------
debug: CFLAGS += $(DBGFLAGS)

#--------------------------------------------------- 
# Compile and create objects
#---------------------------------------------------

.PHONY: all debug clean

all: $(BENCHMARK)
	
$(BENCHMARK):
	$(CC) $(CFLAGS) $(BENCHMARK).c $(LIB_RTE) $(LIB_RTS) $(LIB_PTH) -o $@

debug: all

clean:
	@rm -rf $(BENCHMARK)