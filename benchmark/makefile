#--------------------------------------------------- 
# Target file
#---------------------------------------------------
BENCHMARK_CRE = benchmark_create
BENCHMARK_ATT = benchmark_attach

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

#--------------------------------------------------- 
# Dependencies
#---------------------------------------------------
LIB_RTE = rte_cycles

#--------------------------------------------------- 
# Target-specific variables
#---------------------------------------------------
debug: CFLAGS += $(DBGFLAGS)

#--------------------------------------------------- 
# Compile and create objects
#---------------------------------------------------

.PHONY: all debug clean

all: $(BENCHMARK_CRE) $(BENCHMARK_ATT)
	
$(BENCHMARK_CRE): 
	$(CC) $(CFLAGS) $(LIB_RTE).c $(BENCHMARK_CRE).c $(LIB_RTS) -o $@

$(BENCHMARK_ATT):
	$(CC) $(CFLAGS) $(LIB_RTE).c $(BENCHMARK_ATT).c  $(LIB_RTS) $(LIB_PTH) -o $@

debug: all

clean:
	@rm -rf $(BENCHMARK_CRE) $(BENCHMARK_ATT)