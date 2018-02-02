#makefile for those who have many excutable files

.PHONY: all strip clean sync install mem_check
.SUFFIXES: .sh .h .hpp .c .cc .cpp

CC			= gcc
CX			= g++
CXFLAGS	=  -g -Wall -Werror #-ggdb3
#CXFLAGS		= -O3 -Werror
CX_MACRO	=

AR			= ar
AR_FLAGS	= -crus

INC_DIR		= ./inc
SRC_DIR		= ./src
OBJ_DIR		= ./obj
LIB_DIR		= ./lib
BIN_DIR		= ./bin
EXAM_DIR	= ./exam
LIB_BIN		= liblisk_network.a
BIN			= event_test

INC			= -I$(INC_DIR)
LIB			= -L./lib -llisk_network

all: clean debug release
#strip

debug release: dir $(addprefix $(LIB_DIR)/,$(LIB_BIN)) $(addprefix $(BIN_DIR)/,$(BIN))

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

$(LIB_DIR)/liblisk_network.a:$(addprefix $(OBJ_DIR)/, lisk_log.o lisk_list.o lisk_network.o)
	$(AR) $(AR_FLAGS) -o $@ $^

$(BIN_DIR)/event_test:$(addprefix $(OBJ_DIR)/, event_test.o)
	$(CX) $(CXFLAGS) $(CX_MACRO) -o $@ $^ $(LIB)

strip:
	strip $(addprefix $(LIB_DIR)/,$(LIB))
	strip $(addprefix $(BIN_DIR)/,$(BIN))

clean:
	@rm -f $(addprefix $(LIB_DIR)/,$(LIB_BIN))
	@rm -f $(addprefix $(BIN_DIR)/,$(BIN)) $(OBJ_DIR)/*.o

mem_check:
	valgrind --log-file=unlock_info --track-origins=yes --leak-check=full --show-reachable=yes -v bin/unlock_ring2
	valgrind --log-file=lock_info --track-origins=yes --leak-check=full --show-reachable=yes -v bin/lock_ring2

