#makefile for those who have many excutable files

.SUFFIXES: .sh .h .hpp .c .cc .cpp

CC			= gcc
CX			= g++
CXFLAGS		= -ggdb3 -Wall -Werror
CX_MACRO	=

INC_DIR		= ./inc
SRC_DIR		= ./src
OBJ_DIR		= ./obj
LIB_DIR		= ./lib
BIN_DIR		= ./bin
EXAM_DIR	= ./exam
BIN			= atom_test

INC			= -I$(INC_DIR)
LIB			= -lpthread

all: clean debug release # strip

debug release: dir $(addprefix $(BIN_DIR)/,$(BIN))

#@ do not outoupt this command - ingnore its error and continue to make
dir:
	@-mkdir -p $(INC_DIR)
	@-mkdir -p $(SRC_DIR)
	@-mkdir -p $(OBJ_DIR)
	@-mkdir -p $(LIB_DIR)
	@-mkdir -p $(BIN_DIR)

$(OBJ_DIR)/%.o:$(SRC_DIR)/%.c
	$(CX) $(CXFLAGS) $(CX_MACRO) -o $@ $(INC) -c $<

$(OBJ_DIR)/%.o:$(EXAM_DIR)/%.c
	$(CX) $(CXFLAGS) $(CX_MACRO) -o $@ $(INC) -c $<

$(BIN_DIR)/$(BIN):$(addprefix $(OBJ_DIR)/, lisk_atom.o atom_test.o)
	$(CX) $(CXFLAGS) $(CX_MACRO) -o $@ $^ $(LIB)

strip:
	strip $(addprefix $(BIN_DIR)/,$(BIN))

clean:
	rm -f $(addprefix $(BIN_DIR)/,$(BIN)) $(OBJ_DIR)/*.o

sync:
	rsync -azvc $(BIN_DIR)/*  rsync://

install:
	cp $(./bin/$(BIN)) ../


