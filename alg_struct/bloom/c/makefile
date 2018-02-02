#makefile for those who have many excutable files

.PHONY: all strip clean sync install mem_check
.SUFFIXES: .sh .h .hpp .c .cc .cpp

CC			= gcc
CX			= g++
CFLAGS	=  -g -Wall -Werror #-std=c++11 #-ggdb3
#CFLAGS		= -O3 -Werror #-std=c++11
CX_MACRO	=

INC_DIR		= ./inc
SRC_DIR		= ./src
EXAM_DIR	= ./exam
OBJ_DIR		= ./obj
BIN_DIR		= ./bin
BIN			= converter anti_adver

INC			= -I$(INC_DIR)
LIB			=

all: debug release
#strip

debug release: dir $(addprefix $(BIN_DIR)/,$(BIN))

#@ do not outoupt this command - ingnore its error and continue to make
dir:
	@-mkdir -p $(OBJ_DIR)
	@-mkdir -p $(BIN_DIR)

$(OBJ_DIR)/%.o:$(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(INC) -o $@ -c $<

$(OBJ_DIR)/%.o:$(EXAM_DIR)/%.c
	$(CC) $(CFLAGS) $(INC) -o $@ -c $<

$(OBJ_DIR)/%.o:$(EXAM_DIR)/%.cc
	$(CX) $(CFLAGS) $(CX_MACRO) $(INC) -o $@ -c $<

$(BIN_DIR)/converter:$(addprefix $(OBJ_DIR)/, bloom.o converter.o)
	$(CX) $(CFLAGS) $(CX_MACRO) -o $@ $^ $(LIB)

$(BIN_DIR)/anti_adver:$(addprefix $(OBJ_DIR)/, bloom.o anti_adver.o)
	$(CX) $(CFLAGS) $(CX_MACRO) -o $@ $^ $(LIB)

strip:
	strip $(addprefix $(BIN_DIR)/,$(BIN))

clean:
	@rm -f $(addprefix $(BIN_DIR)/,$(BIN)) $(OBJ_DIR)/*.o ./valgrind_info
	@rm -rf $(BIN_DIR) $(OBJ_DIR)

mem_check:
	valgrind --log-file=valgrind_info --track-origins=yes --leak-check=full --show-reachable=yes -v bin/tellist_fb

