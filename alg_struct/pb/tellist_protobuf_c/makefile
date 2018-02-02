#makefile for those who have many excutable files

.SUFFIXES: .sh .h .c .cc .cpp

CXX			= gcc
CXXFLAGS	= -ggdb3 -Wall -Werror
CXX_MACRO	=

INC_DIR		= ./inc
SRC_DIR		= ./src
OBJ_DIR		= ./obj
LIB_DIR		= ./lib
BIN_DIR		= ./bin
BIN			= store destore

INC			= -I./protobuf_c/include -I$(INC_DIR)
LIB			= -L./protobuf_c/lib64 -lprotobuf-c

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
	$(CXX) $(CXXFLAGS) $(CXX_MACRO) -o $@ $(INC) -c $<

$(BIN_DIR)/store:$(addprefix $(OBJ_DIR)/, tellist.pc.o  store.o)
	$(CXX) $(CXXFLAGS) $(CXX_MACRO) -o $@ $^ $(LIB)

$(BIN_DIR)/destore:$(addprefix $(OBJ_DIR)/, tellist.pc.o  destore.o)
	$(CXX) $(CXXFLAGS) $(CXX_MACRO) -o $@ $^ $(LIB)

strip:
	strip $(addprefix $(BIN_DIR)/,$(BIN))

clean:
	rm -f $(addprefix $(BIN_DIR)/,$(BIN)) $(OBJ_DIR)/*.o
	rm -f out

