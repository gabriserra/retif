#---------------------------------------------------
# Target file
#---------------------------------------------------
RTSLIB = librts
LIBNAM = retif_lib

#---------------------------------------------------
# Compiler
#---------------------------------------------------
CC = gcc

#---------------------------------------------------
# Install paths
#---------------------------------------------------
SYS_LIB_PATH = /usr/lib
SYS_INC_PATH = /usr/include

#---------------------------------------------------
# Project paths
#---------------------------------------------------
SRC_PATH =  ../daemon/src
LIB_PATH =  ../daemon/lib
BIN_PATH =  ../bin
BLI_PATH =	../bin/lib
BDR_PATH =  ../bin/daemon
BDS_PATH =  ../bin/daemon/src
BDL_PATH =  ../bin/daemon/lib
OUT = $(BLI_PATH)/$(RTSLIB)

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
LDFLAGS = -pthread

#---------------------------------------------------
# Dependencies
#---------------------------------------------------
LIB_USL = usocket

LIBS = $(LIB_USL)
LIBS_O = $(foreach LIB, $(LIBS), $(BDL_PATH)/$(LIB).o)

SRC_CHN = retif_channel

SRCS = $(SRC_CHN)
SRCS_O = $(foreach SRC, $(SRCS), $(BDS_PATH)/$(SRC).o)

#---------------------------------------------------
# Target-specific variables
#---------------------------------------------------
debug: CFLAGS += $(DBGFLAGS)

#---------------------------------------------------
# Compile and create objects
#---------------------------------------------------

.PHONY: all paths debug install uninstall clean cleanall

all: paths $(OUT).so

# TODO: Use mkdir -p (?)
paths:
	@echo Creating build directories
	@if [ ! -d "$(BIN_PATH)" ]; then mkdir $(BIN_PATH); fi
	@if [ ! -d "$(BLI_PATH)" ]; then mkdir $(BLI_PATH); fi
	@if [ ! -d "$(BDR_PATH)" ]; then mkdir $(BDR_PATH); fi
	@if [ ! -d "$(BDS_PATH)" ]; then mkdir $(BDS_PATH); fi
	@if [ ! -d "$(BDL_PATH)" ]; then mkdir $(BDL_PATH); fi

$(OUT).so: $(OUT).o $(LIBS_O) $(SRCS_O)
	$(CC) $(CFLAGS) -shared $(LDFLAGS) $^ -o $(OUT).so

$(OUT).o: $(LIBNAM).c
	$(CC) $(INCDIR) -c $(CFLAGS) $< -o $@

$(LIBS_O): $(BDL_PATH)/%.o: $(LIB_PATH)/%.c
	$(CC) $(INCDIR) -c $(CFLAGS) $< -o $@

$(SRCS_O): $(BDS_PATH)/%.o: $(SRC_PATH)/%.c
	$(CC) $(INCDIR) -c $(CFLAGS) $< -o $@

debug: all

install: all
	@echo Install the library requires root privileges!
	@sudo cp $(OUT).so $(SYS_LIB_PATH)/$(RTSLIB).so
	@sudo chmod 0755 $(SYS_LIB_PATH)/$(RTSLIB).so
	@sudo ldconfig $(SYS_LIB_PATH)
	@sudo cp $(LIBNAM).h $(SYS_INC_PATH)/$(RTSLIB).h
	@sudo chmod 0444 $(SYS_INC_PATH)/$(RTSLIB).h
	@sudo cp $(LIBNAM).h $(SYS_INC_PATH)/$(RTSLIB).h
	@sudo chmod 0444 $(SYS_INC_PATH)/$(RTSLIB).h
	@echo Done!

uninstall:
	@echo Uninstall requires root privileges.
	@sudo rm -f $(SYS_LIB_PATH)/$(RTSLIB).so
	@sudo ldconfig
	@sudo rm -f $(SYS_INC_PATH)/$(RTSLIB).h
	@echo Done!

clean:
	@rm -rf $(BLI_PATH)

cleanall:
	@rm -rf $(BIN_PATH)
