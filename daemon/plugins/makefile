#---------------------------------------------------
# Compiler
#---------------------------------------------------
CC = gcc

#--------------------------------------------------- 
# Install paths
#---------------------------------------------------
SYS_SHARE_PATH = /usr/share
RTSD_DIR = $(SYS_SHARE_PATH)/rtsd

#--------------------------------------------------- 
# Project paths
#---------------------------------------------------
SRC_PATH =  ../src
LIB_PATH =  ../lib
BIN_PATH =  ../../bin
BDR_PATH =  ../../bin/daemon
BDS_PATH =  ../../bin/daemon/src
BDL_PATH =  ../../bin/daemon/lib
BDP_PATH =  ../../bin/daemon/plugins

#---------------------------------------------------
# Options passed to the compiler
#---------------------------------------------------
CFLAGS = -Wall -O -fPIC -std=gnu99
DBGFLAGS = -g -O0

#---------------------------------------------------
# Load library path
#---------------------------------------------------
INCDIR = -I$(LIB_PATH) -I$(SRC_PATH)

#---------------------------------------------------
# Modules loaded
#---------------------------------------------------
LDFLAGS = -lm

#--------------------------------------------------- 
# Dependencies
#---------------------------------------------------
LIB_LSP = list

LIBS = $(LIB_LSP)
LIBS_C = $(foreach LIB, $(LIBS), $(LIB_PATH)/$(LIB).c)
LIBS_O = $(foreach LIB, $(LIBS), $(BDL_PATH)/$(LIB).o)

SRC_PLG = rts_plugin
SRC_TSK = rts_task
SRC_TSS = rts_taskset
SRC_UTS = rts_utils

SRCS = $(SRC_PLG) $(SRC_TSK) $(SRC_TSS) $(SRC_UTS)
SRCS_O = $(foreach SRC, $(SRCS), $(BDS_PATH)/$(SRC).o)

PLG_EDF = sched_EDF
PLG_RM = sched_RM
PLG_FP = sched_FP
PLG_RR = sched_RR

PLG_EDF_B = $(BDP_PATH)/$(PLG_EDF)
PLG_RM_B = $(BDP_PATH)/$(PLG_RM)
PLG_FP_B = $(BDP_PATH)/$(PLG_FP)
PLG_RR_B = $(BDP_PATH)/$(PLG_RR)

PLGS_SO = $(PLG_EDF_B).so $(PLG_RM_B).so $(PLG_FP_B).so $(PLG_RR_B).so

#--------------------------------------------------- 
# Target-specific variables
#---------------------------------------------------
debug: CFLAGS += $(DBGFLAGS)

#--------------------------------------------------- 
# Compile and create objects
#---------------------------------------------------

.PHONY: all debug install uninstall clean cleanall

all: paths $(PLGS_SO)

paths:
	@echo Creating build directories
	@if [ ! -d "$(BIN_PATH)" ]; then mkdir $(BIN_PATH); fi
	@if [ ! -d "$(BDR_PATH)" ]; then mkdir $(BDR_PATH); fi
	@if [ ! -d "$(BDP_PATH)" ]; then mkdir $(BDP_PATH); fi
	@if [ ! -d "$(BDL_PATH)" ]; then mkdir $(BDL_PATH); fi
	@if [ ! -d "$(BDS_PATH)" ]; then mkdir $(BDS_PATH); fi
	
$(PLG_EDF_B).so: $(PLG_EDF_B).o $(LIBS_O) $(SRCS_O)
	$(CC) $(CFLAGS) -shared $^ -o $@

$(PLG_RM_B).so: $(PLG_RM_B).o $(LIBS_O) $(SRCS_O)
	$(CC) $(CFLAGS) -shared $^ -lm -o $@

$(PLG_FP_B).so: $(PLG_FP_B).o $(LIBS_O) $(SRCS_O)
	$(CC) $(CFLAGS) -shared $^ -o $@

$(PLG_RR_B).so: $(PLG_RR_B).o $(LIBS_O) $(SRCS_O)
	$(CC) $(CFLAGS) -shared $^ -o $@

$(PLG_EDF_B).o: $(BDP_PATH)/%.o: %.c
	$(CC) -c $(INCDIR) $(CFLAGS) $< -o $@

$(PLG_RM_B).o: $(BDP_PATH)/%.o: %.c
	$(CC) -c $(INCDIR) $(CFLAGS) $< -o $@

$(PLG_FP_B).o: $(BDP_PATH)/%.o: %.c
	$(CC) -c $(INCDIR) $(CFLAGS) $< -o $@

$(PLG_RR_B).o: $(BDP_PATH)/%.o: %.c
	$(CC) -c $(INCDIR) $(CFLAGS) $< -o $@

$(LIBS_O): $(BDL_PATH)/%.o: $(LIB_PATH)/%.c
	$(CC) -c $(CFLAGS) $< -o $@

$(SRCS_O): $(BDS_PATH)/%.o: $(SRC_PATH)/%.c
	$(CC) -c $(INCDIR) $(CFLAGS) $< -o $@

debug: all

install: all
	@echo Installing plugins.
	@if [ ! -d "$(RTSD_DIR)" ]; then echo "RTS-Daemon not installed. Exiting"; exit 1; fi
	@if [ ! -d "$(RTSD_DIR)/plugins" ]; then sudo mkdir $(RTSD_DIR)/plugins; fi
	@sudo cp $(BDP_PATH)/*.so $(RTSD_DIR)/plugins 
	@sudo cp schedconfig.cfg $(RTSD_DIR)/schedconfig.cfg
	@echo Done!

uninstall:
	@echo Uninstalling plugins.
	@sudo rm -rf $(RTSD_DIR)/plugins
	@echo Done!

clean:
	@rm -rf $(BDP_PATH)

cleanall:
	@rm -rf $(BIN_PATH)
