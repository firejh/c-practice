#makefile for those who have many excutable files

.SUFFIXES: .sh .h .c .cc .cpp

CC          = gcc
CX          = g++
#CCFLAGS    = -ggdb3 -Wall -Werror
CCFLAGS     = -std=c99 -c -fPIC -Wstrict-prototypes -Wwrite-strings -g -ggdb -D_POSIX_C_SOURCE #-Wall -W 
CXFLAGS     = -c -fPIC -Wwrite-strings -g -ggdb #-Wall -W 
CCFLAG      = -fPIC  -Wall -W -Wstrict-prototypes -Wwrite-strings -g -ggdb 

INC_DIR     = inc
SRC_DIR     = src
REDIS_DIR   = redis
TEST_DIR    = test
OBJ_DIR     = obj
LIB_DIR     = lib
BIN_DIR     = bin
BIN         = async-test
# redis-replay-aof redis-dump redis-dump-key redis-migrate redis-max-kv async-test

BASE_PATH   =

INC         = -I$(INC_DIR) -I$(INC_DIR)/hiredis -I$(INC_DIR)/redis

# rpath把lib的绝对路径放在elf文件中  -Wl,-rpath=./$(LIB_DIR) 
LIB         =  -L./$(LIB_DIR) -lhiredis

all: clean debug release # strip

debug release gprof: dir $(addprefix $(BIN_DIR)/,$(BIN))

#@ do not outoupt this command - ingnore its error and continue to make
dir:
	@-mkdir -p $(INC_DIR)
	@-mkdir -p $(SRC_DIR)
	@-mkdir -p $(OBJ_DIR)
	@-mkdir -p $(LIB_DIR)
	@-mkdir -p $(BIN_DIR)

$(OBJ_DIR)/%.o:$(SRC_DIR)/%.c
	$(CC) $(CCFLAGS) $(CC_MACRO) -o $@ $(INC) -c $<

$(OBJ_DIR)/%.o:$(REDIS_DIR)/%.c
	$(CC) $(CCFLAGS) $(CC_MACRO) -o $@ $(INC) -c $<

$(OBJ_DIR)/%.o:$(TEST_DIR)/%.c
	$(CC) $(CCFLAGS) $(CC_MACRO) -o $@ $(INC) -c $<

$(OBJ_DIR)/%.o:$(TEST_DIR)/%.cc
	$(CX) $(CXFLAGS) $(CC_MACRO) -o $@ $(INC) -c $<

$(BIN_DIR)/redis-dump:$(addprefix $(OBJ_DIR)/, tool.o redis-dump.o)
	$(CC) $(CCFLAG) $(CC_MACRO) -o $@ $^ $(LIB)

$(BIN_DIR)/redis-dump-key:$(addprefix $(OBJ_DIR)/, tool.o redis-dump-key.o)
	$(CC) $(CCFLAG) $(CC_MACRO) -o $@ $^ $(LIB)

$(BIN_DIR)/redis-migrate:$(addprefix $(OBJ_DIR)/, tool.o redis-migrate.o)
	$(CC) $(CCFLAG) $(CC_MACRO) -o $@ $^ $(LIB)

$(BIN_DIR)/redis-max-kv:$(addprefix $(OBJ_DIR)/, tool.o redis-max-kv.o)
	$(CX) $(CCFLAG) $(CC_MACRO) -o $@ $^ $(LIB)

$(BIN_DIR)/redis-replay-aof:$(addprefix $(OBJ_DIR)/, redis-replay-aof.o)
	$(CC) $(CCFLAG) $(CC_MACRO) -o $@ $^ $(LIB)

$(BIN_DIR)/async-test:$(addprefix $(OBJ_DIR)/, async-test.o ae.o anet.o)
	$(CC) $(CCFLAG) $(CC_MACRO) -o $@ $^ $(LIB)

strip:
	strip $(addprefix $(BIN_DIR)/,$(BIN))

clean:
	rm -f $(addprefix $(BIN_DIR)/,$(BIN)) $(OBJ_DIR)/*.o

