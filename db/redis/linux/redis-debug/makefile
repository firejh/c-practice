#makefile for those who have many excutable files

.SUFFIXES: .sh .h .c .cc .cpp

CC          = gcc
CX          = g++
#CCFLAGS    = -ggdb3 -Wall -Werror
CCFLAGS     = -std=c99 -c -fPIC -Wstrict-prototypes -Wwrite-strings -g -ggdb -D_POSIX_C_SOURCE #-Wall -W 
CXFLAGS     = -c -fPIC -Wwrite-strings -g -ggdb #-Wall -W 
CCFLAG      = -fPIC  -Wall -W -Wstrict-prototypes -Wwrite-strings -g -ggdb 

LIB         = -lm -lpthread

REDIS_DIR   = src
HIREDIS_DIR = ./deps/hiredis
OBJ_DIR     = obj
BIN_DIR     = bin
BIN         = t_conf

INC         = -I./deps/lua/src/ -I./deps/hiredis/ -I$(REDIS_DIR)/

REDIS_SERVER_OBJ=adlist.o ae.o anet.o dict.o redis.o sds.o zmalloc.o lzf_c.o lzf_d.o pqsort.o zipmap.o sha1.o ziplist.o release.o networking.o util.o object.o db.o replication.o rdb.o t_string.o t_list.o t_set.o t_zset.o t_hash.o config.o aof.o pubsub.o multi.o debug.o sort.o intset.o syncio.o cluster.o crc16.o endianconv.o slowlog.o scripting.o bio.o rio.o rand.o memtest.o crc64.o bitops.o sentinel.o notify.o setproctitle.o blocked.o hyperloglog.o latency.o sparkline.o

all: debug 

dep:
	cd deps && make distclean && make jemalloc && make hiredis && make lua

depclean:
	cd deps && $(MAKE) distclean

debug : dir $(addprefix $(BIN_DIR)/,$(BIN))

#@ do not outoupt this command - ingnore its error and continue to make
dir:
	@-mkdir -p $(OBJ_DIR)
	@-mkdir -p $(BIN_DIR)

$(OBJ_DIR)/%.o:$(REDIS_DIR)/%.c
	$(CC) $(CCFLAGS) -o $@ $(INC) -c $<

$(BIN_DIR)/t_conf:$(addprefix $(OBJ_DIR)/, conf.o t_conf.o $(REDIS_SERVER_OBJ))
	$(CC) $(CCFLAG) -o $@ $^  ./deps/hiredis/libhiredis.a ./deps/lua/src/liblua.a $(LIB)

strip:
	strip $(addprefix $(BIN_DIR)/,$(BIN))

clean:
	rm -f $(addprefix $(BIN_DIR)/,$(BIN)) $(OBJ_DIR)/*.o
