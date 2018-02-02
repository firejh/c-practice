#makefile for those who have many excutable files

.SUFFIXES: .sh .h .c .cc .cpp

CXX			= g++
CXXFLAGS	= -ggdb3 -Wall -Werror
CXX_MACRO	=

GPB_PROTO   = proto/tellist.proto

INC_DIR		= inc
SRC_DIR		= src
OBJ_DIR		= obj
LIB_DIR		= lib
BIN_DIR		= bin
BIN			= store destore

BASE_PATH	=

INC			= -I/Users/alex/bin/protobuf/include -I$(INC_DIR)

LIB			= -L/Users/alex/bin/protobuf/lib -lprotobuf

all: clean debug release # strip

debug release gprof: dir gpb $(addprefix $(BIN_DIR)/,$(BIN))

gpb:
	protoc -I=./proto -I=/Users/alex/bin/protobuf/include --cpp_out=./src $(GPB_PROTO)

#@ do not outoupt this command - ingnore its error and continue to make
dir:
	@-mkdir -p $(INC_DIR)
	@-mkdir -p $(SRC_DIR)
	@-mkdir -p $(OBJ_DIR)
	@-mkdir -p $(LIB_DIR)
	@-mkdir -p $(BIN_DIR)

$(OBJ_DIR)/%.pb.o:$(SRC_DIR)/%.pb.cc
	$(CXX) $(CXXFLAGS) $(CXX_MACRO) -o $@ $(INC) -c $<

$(OBJ_DIR)/%.o:$(SRC_DIR)/%.cc
	$(CXX) $(CXXFLAGS) $(CXX_MACRO) -o $@ $(INC) -c $<

$(BIN_DIR)/store:$(addprefix $(OBJ_DIR)/, store.o tellist.pb.o)
	$(CXX) $(CXXFLAGS) $(CXX_MACRO) -o $@ $^ $(LIB)

$(BIN_DIR)/destore:$(addprefix $(OBJ_DIR)/, destore.o tellist.pb.o)
	$(CXX) $(CXXFLAGS) $(CXX_MACRO) -o $@ $^ $(LIB)

strip:
	strip $(addprefix $(BIN_DIR)/,$(BIN))

clean:
	rm -f $(addprefix $(BIN_DIR)/,$(BIN)) $(OBJ_DIR)/*.o

