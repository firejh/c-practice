#makefile for those who have many excutable files

.PHONY: all strip clean sync install mem_check
.SUFFIXES: .sh .h .hpp .c .cc .cpp

CC			= gcc
CX			= g++
CFLAGS	    = -g -ggdb3 -Wall -Werror -std=c11 -lpthread -D_GLIBCXX_USE_NANOSL -D_GLIBCXX_USE_NANOSLEEP -D_GLIBCXX_USE_SCHED_YIELD
CXFLAGS	    = -fno-rtti -g -ggdb3 -Wall -Werror -std=c++11 -lpthread -D_GLIBCXX_USE_NANOSL -D_GLIBCXX_USE_NANOSLEEP -D_GLIBCXX_USE_SCHED_YIELD  -DROCKSDB_PLATFORM_POSIX -DROCKSDB_LIB_IO_POSIX  -DOS_LINUX -fno-builtin-memcmp -DROCKSDB_FALLOCATE_PRESENT -DSNAPPY -DZLIB -DBZIP2 -DLZ4 -DZSTD -DROCKSDB_MALLOC_USABLE_SIZE -DROCKSDB_PTHREAD_ADAPTIVE_MUTEX -DROCKSDB_BACKTRACE -DROCKSDB_RANGESYNC_PRESENT -DROCKSDB_SCHED_GETCPU_PRESENT -march=native  -DHAVE_SSE42 -DROCKSDB_SUPPORT_THREAD_LOCAL -D_GLIBCXX_USE_NANOSLEEP
#CFLAGS		= -O3 -Werror -std=c++11
CX_MACRO	=

INC_DIR		= ./inc
SRC_DIR		= ./src
EXAM_DIR	= ./exam
LIB_DIR     = ./lib
OBJ_DIR		= ./obj
BIN_DIR		= ./bin
BIN			= simple

INC			= -I$(INC_DIR)
LIB			= -lpthread -lrt -rdynamic -lz -llz4 -lbz2 -lsnappy -L$(LIB_DIR) -lrocksdb -lzstd -lsnappy

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

$(OBJ_DIR)/%.o:$(SRC_DIR)/%.cc
	$(CX) $(CXFLAGS) $(CX_MACRO) $(INC) -o $@ -c $<

$(BIN_DIR)/simple:$(addprefix $(OBJ_DIR)/, simple.o)
	$(CX) $(CXFLAGS) $(CX_MACRO) -o $@ $^ $(LIB)

strip:
	strip $(addprefix $(BIN_DIR)/,$(BIN))

clean:
	@rm -f $(addprefix $(BIN_DIR)/,$(BIN)) $(OBJ_DIR)/*.o ./valgrind_info
	@rm -rf $(BIN_DIR) $(OBJ_DIR)

mem_check:
	valgrind --log-file=valgrind_info --track-origins=yes --leak-check=full --show-reachable=yes -v bin/tellist_fb

