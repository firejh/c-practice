#makefile for those who have many excutable files

.PHONY: all strip clean sync install mem_check
.SUFFIXES: .sh .h .hpp .c .cc .cpp

CC			= gcc
CFLAGS		= -O0 -ggdb3
CX			= g++
CXFLAGS	    = -O0 -g -ggdb3
#CFLAGS		= -O3 -Werror -std=c++11

INC_DIR		= ./inc
SRC_DIR		= ./src
EXAM_DIR	= ./exam
OBJ_DIR		= ./obj
BIN_DIR		= ./bin
BIN			= ngx_test

INC			= -I$(INC_DIR)
LIB			= -lpthread

all: debug release
#strip

debug release: dir $(addprefix $(BIN_DIR)/,$(BIN))

#@ do not outoupt this command - ingnore its error and continue to make
dir:
	@-mkdir -p $(OBJ_DIR)
	@-mkdir -p $(BIN_DIR)

$(OBJ_DIR)/%.o:$(EXAM_DIR)/%.cc
	$(CX) $(CXFLAGS) $(INC) -o $@ -c $<

$(OBJ_DIR)/%.o:$(SRC_DIR)/%.cc
	$(CX) $(CXFLAGS) $(INC) -o $@ -c $<

$(OBJ_DIR)/%.o:$(EXAM_DIR)/%.c
	$(CC) $(CFLAGS) $(INC) -o $@ -c $<

$(OBJ_DIR)/%.o:$(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(INC) -o $@ -c $<

$(BIN_DIR)/ini_test:$(addprefix $(OBJ_DIR)/, ini.o ini_test.o)
	$(CC) $(CFLAGS) -o $@ $^ $(LIB)

$(BIN_DIR)/ngx_test:$(addprefix $(OBJ_DIR)/, ngx.o)
	$(CC) $(CFLAGS) -o $@ $^ $(LIB)

strip:
	strip $(addprefix $(BIN_DIR)/,$(BIN))

clean:
	@rm -f $(addprefix $(BIN_DIR)/,$(BIN)) $(OBJ_DIR)/*.o ./valgrind_info
	@rm -rf $(BIN_DIR) $(OBJ_DIR)

mem_check:
	valgrind --log-file=valgrind_info --track-origins=yes --leak-check=full --show-reachable=yes -v bin/tellist_fb
