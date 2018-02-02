#makefile for those who have many excutable files

.PHONY: all strip clean sync install mem_check
.SUFFIXES: .sh .h .hpp .c .cc .cpp

CC			= gcc
CX			= g++
CXFLAGS	=  -g -Wall -Werror #-ggdb3
#CXFLAGS		= -O3 -Werror
CX_MACRO	=

INC_DIR		= ./inc
SRC_DIR		= ./src
OBJ_DIR		= ./obj
LIB_DIR		= ../lib
BIN_DIR		= ./bin
EXAM_DIR	= ./exam
BIN			= lock_ring lock_ring2 unlock_ring1 unlock_ring2 unlock_ring3 unlock_ring4

INC			= -I$(INC_DIR)
LIB			= -L./lib -lpthread

all: clean debug release
#strip

debug release: dir $(addprefix $(BIN_DIR)/,$(BIN))

#@ do not outoupt this command - ingnore its error and continue to make
dir:
	@-mkdir -p $(INC_DIR)
	@-mkdir -p $(SRC_DIR)
	@-mkdir -p $(OBJ_DIR)
	@-mkdir -p $(LIB_DIR)
	@-mkdir -p $(BIN_DIR)

$(OBJ_DIR)/%.o:$(SRC_DIR)/%.c
	$(CC) $(CXFLAGS) $(CX_MACRO) $(INC) -o $@ -c $<

$(OBJ_DIR)/%.o:$(SRC_DIR)/%.cc
	$(CX) $(CXFLAGS) $(CX_MACRO) $(INC) -o $@ -c $<

$(OBJ_DIR)/%.o:$(EXAM_DIR)/%.c
	$(CC) $(CXFLAGS) $(CX_MACRO) $(INC) -o $@ -c $<

$(OBJ_DIR)/%.o:$(EXAM_DIR)/%.cc
	$(CX) $(CXFLAGS) $(CX_MACRO) $(INC) -o $@ -c $<

$(BIN_DIR)/lock_ring:$(addprefix $(OBJ_DIR)/, lock_ring.o)
	$(CX) $(CXFLAGS) $(CX_MACRO) -o $@ $^ $(LIB)

$(BIN_DIR)/lock_ring2:$(addprefix $(OBJ_DIR)/, lock_ring2.o lisk_ring.o lisk_deque.o)
	$(CX) $(CXFLAGS) $(CX_MACRO) -o $@ $^ $(LIB)

$(BIN_DIR)/unlock_ring1:$(addprefix $(OBJ_DIR)/, unlock_ring1.o lisk_ring.o lisk_sys.o)
	$(CX) $(CXFLAGS) $(CX_MACRO) -o $@ $^ $(LIB)

$(BIN_DIR)/unlock_ring2:$(addprefix $(OBJ_DIR)/, unlock_ring2.o lisk_ring.o lisk_sys.o)
	$(CX) $(CXFLAGS) $(CX_MACRO) -o $@ $^ $(LIB)

$(BIN_DIR)/unlock_ring3:$(addprefix $(OBJ_DIR)/, unlock_ring3.o lisk_ring.o lisk_sys.o)
	$(CX) $(CXFLAGS) $(CX_MACRO) -o $@ $^ $(LIB)

$(BIN_DIR)/unlock_ring4:$(addprefix $(OBJ_DIR)/, unlock_ring4.o lisk_ring.o lisk_sys.o)
	$(CX) $(CXFLAGS) $(CX_MACRO) -o $@ $^ $(LIB)

strip:
	strip $(addprefix $(BIN_DIR)/,$(BIN))

clean:
	@rm -f $(addprefix $(BIN_DIR)/,$(BIN)) $(OBJ_DIR)/*.o

mem_check:
	valgrind --log-file=unlock_info --track-origins=yes --leak-check=full --show-reachable=yes -v bin/unlock_ring2
	valgrind --log-file=lock_info --track-origins=yes --leak-check=full --show-reachable=yes -v bin/lock_ring2

