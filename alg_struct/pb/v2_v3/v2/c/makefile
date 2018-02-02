#makefile for those who have many excutable files

.SUFFIXES: .sh .h .c .cc .cpp

CXX			= g++
CXXFLAGS	= -ggdb3 -Wall -Werror
CXX_MACRO	=

INC_DIR		= inc
SRC_DIR		= src
OBJ_DIR		= obj
LIB_DIR		= lib
BIN_DIR		= bin
BIN			= encoder decoder

BASE_PATH	=

GPB_PROTO   = ../proto/msg.proto ../proto/msg_array.proto
GPB_CC_EXT  = .pb.cc
GPB_OBJ_EXT = .pb.o
GPB_INC     = -I~/bin/protobuf/include
GPB_LIB     = -L~/bin/protobuf/lib -lprotobuf

INC			= -I../../protobuf/include/ -I$(INC_DIR)

LIB			= -L../../protobuf/lib -lprotobuf

all: clean debug release # strip

debug release gprof: dir gpb $(addprefix $(BIN_DIR)/,$(BIN))

#@ do not outoupt this command - ingnore its error and continue to make
dir:
	@-mkdir -p $(INC_DIR)
	@-mkdir -p $(SRC_DIR)
	@-mkdir -p $(OBJ_DIR)
	@-mkdir -p $(LIB_DIR)
	@-mkdir -p $(BIN_DIR)

gpb:
	-protoc -I=../proto --cpp_out=./$(SRC_DIR) $(GPB_PROTO)
	-mv $(SRC_DIR)/*.pb.h $(INC_DIR)

$(OBJ_DIR)/%$(GPB_OBJ_EXT):$(SRC_DIR)/%$(GPB_CCSRC_EXT)
	$(CXX) $(CXXFLAGS) $(CXX_MACRO) -o $@ $(INC) -c $<

$(OBJ_DIR)/%.o:$(SRC_DIR)/%.cc
	$(CXX) $(CXXFLAGS) $(CXX_MACRO) -o $@ $(INC) -c $<

$(BIN_DIR)/encoder:$(addprefix $(OBJ_DIR)/, encoder.o msg.pb.o msg_array.pb.o)
	$(CXX) $(CXXFLAGS) $(CXX_MACRO) -o $@ $^ $(LIB)

$(BIN_DIR)/decoder:$(addprefix $(OBJ_DIR)/, decoder.o msg.pb.o msg_array.pb.o)
	$(CXX) $(CXXFLAGS) $(CXX_MACRO) -o $@ $^ $(LIB)

strip:
	strip $(addprefix $(BIN_DIR)/,$(BIN))

clean:
	rm -f $(addprefix $(BIN_DIR)/,$(BIN)) $(OBJ_DIR)/*.o

