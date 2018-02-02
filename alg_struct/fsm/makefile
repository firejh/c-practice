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
BIN			= fsm_test

INC			= -I$(INC_DIR)
LIB			=

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

$(BIN_DIR)/$(BIN):$(addprefix $(OBJ_DIR)/, fsm_test.o lisk_fsm.o)
	$(CXX) $(CXXFLAGS) $(CXX_MACRO) -o $@ $^ $(LIB)

strip:
	strip $(addprefix $(BIN_DIR)/,$(BIN))

clean:
	rm -f $(addprefix $(BIN_DIR)/,$(BIN)) $(OBJ_DIR)/*.o

sync:
	rsync -azvc $(BIN_DIR)/*  rsync://10.133.2.206:49000/client_cgi/cgi-bin/

install:
	cp $(./bin/$(BIN)) ../qun.qq.com/cgi-bin/ 
	cp qun.ini ../qun.qq.com/cgi-bin/ 


