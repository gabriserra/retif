#--------------------------------------------------- 
# Target file
#---------------------------------------------------
DAEMON = rtsd

#---------------------------------------------------
# Compiler
#---------------------------------------------------
CC = gcc

#--------------------------------------------------- 
# Install paths
#---------------------------------------------------
SYS_BIN_PATH = /usr/bin
SYS_SHARE_PATH = /usr/share
RTSD_DIR = $(SYS_SHARE_PATH)/$(DAEMON)

#--------------------------------------------------- 
# Project paths
#---------------------------------------------------
SRC_PATH =  src
LIB_PATH =  lib
BIN_PATH =  ../bin
BDR_PATH =  ../bin/daemon
BDS_PATH =  ../bin/daemon/src
BDL_PATH =  ../bin/daemon/lib
OUT = $(BDR_PATH)/$(DAEMON)

#---------------------------------------------------
# Options passed to the compiler
#---------------------------------------------------
CFLAGS = -Wall -std=gnu99 -O3
DBGFLAGS = -g -O0 -D LOG_OUTPUT=LOG_STDOUT -D LOG_LEVEL=40

#---------------------------------------------------
# Load library path
#---------------------------------------------------
INCDIR = -I$(LIB_PATH) -I$(SRC_PATH)

#---------------------------------------------------
# Modules loaded
#---------------------------------------------------
LDFLAGS = -ldl -lrt -lpthread

#--------------------------------------------------- 
# Dependencies
#---------------------------------------------------

LIB_LSP = list
LIB_USK = usocket

LIBS = $(LIB_LSP) $(LIB_USK)
LIBS_C = $(foreach LIB, $(LIBS), $(LIB_PATH)/$(LIB).c)
LIBS_O = $(foreach LIB, $(LIBS), $(BDL_PATH)/$(LIB).o)

SRC_CHN = rts_channel
SRC_DAE = rts_daemon
SRC_PLG = rts_plugin
SRC_SCH = rts_scheduler
SRC_TSK = rts_task
SRC_TSS = rts_taskset
SRC_TYP = rts_types
SRC_UTS = rts_utils
SRC_CNF = rts_config

SRCS = $(SRC_CHN) $(SRC_DAE) $(SRC_PLG) $(SRC_SCH) $(SRC_TSK) $(SRC_TSS) $(SRC_UTS) $(SRC_CNF)
SRCS_C = $(foreach SRC, $(SRCS), $(SRC_PATH)/$(SRC).c)
SRCS_O = $(foreach SRC, $(SRCS), $(BDS_PATH)/$(SRC).o)

#--------------------------------------------------- 
# Target-specific variables
#---------------------------------------------------

debug: CFLAGS += $(DBGFLAGS)

#--------------------------------------------------- 
# Compile and create objects
#---------------------------------------------------

.PHONY: all paths debug install uninstall clean

all: paths $(OUT)

paths:
	if [ ! -d "$(BIN_PATH)" ]; then mkdir $(BIN_PATH); fi
	if [ ! -d "$(BDR_PATH)" ]; then mkdir $(BDR_PATH); fi
	if [ ! -d "$(BDL_PATH)" ]; then mkdir $(BDL_PATH); fi
	if [ ! -d "$(BDS_PATH)" ]; then mkdir $(BDS_PATH); fi

$(OUT): $(OUT).o $(LIBS_O) $(SRCS_O) 
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

$(OUT).o:
	$(CC) $(INCDIR) -c $(CFLAGS) daemon.c -o $@
	
$(LIBS_O): $(BDL_PATH)/%.o: $(LIB_PATH)/%.c
	$(CC) -c $(CFLAGS) $< -o $@

$(SRCS_O): $(BDS_PATH)/%.o: $(SRC_PATH)/%.c
	$(CC) -I$(LIB_PATH) -c $(CFLAGS) $< -o $@

debug: all

install: all
	@echo Installing rts daemon.
	@echo $(DAEMON) needs root privileges to run. You will be asked for root password.
	@sudo chown root $(OUT)
	@if [ ! -d "$(RTSD_DIR)" ]; then sudo mkdir $(RTSD_DIR); fi
	@sudo cp $(OUT) $(SYS_BIN_PATH)
	@sudo chmod 4755 $(SYS_BIN_PATH)/rtsd
	@echo Done!

uninstall:
	@echo Uninstalling rts daemon.
	@sudo rm -rf $(RTSD_DIR)
	@sudo rm -f $(SYS_BIN_PATH)/$(DAEMON)
	@echo Done!

clean:
	@rm -rf $(BIN_PATH)
	

