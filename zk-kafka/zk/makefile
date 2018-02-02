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
EXAM_DIR	= ./exam
BIN			= async_client sync_client

INC			= -I$(INC_DIR) -I$(INC_DIR)/zookeeper/
LIB			= -L$(LIB_DIR) -lzookeeper_mt -lpthread -lm

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
	$(CXX) $(CXXFLAGS) $(CXX_MACRO) -o $@ $(INC) -c $<

$(OBJ_DIR)/%.o:$(EXAM_DIR)/%.c
	$(CXX) $(CXXFLAGS) $(CXX_MACRO) -o $@ $(INC) -c $<

$(BIN_DIR)/async_client:$(addprefix $(OBJ_DIR)/, async_client.o)
	$(CXX) $(CXXFLAGS) $(CXX_MACRO) -o $@ $^ $(LIB)

$(BIN_DIR)/sync_client:$(addprefix $(OBJ_DIR)/, sync_client.o)
	$(CXX) $(CXXFLAGS) $(CXX_MACRO) -o $@ $^ $(LIB)

strip:
	strip $(addprefix $(BIN_DIR)/,$(BIN))

clean:
	rm -f $(addprefix $(BIN_DIR)/,$(BIN)) $(OBJ_DIR)/*.o



