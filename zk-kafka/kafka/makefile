#makefile for those who have many excutable files

.SUFFIXES: .sh .h .c .cc .cpp

CXX	        = gcc
CXXFLAGS	= -ggdb3 -Wall -Werror -fPIC
CXX_MACRO	=

INC_DIR		= ./inc
SRC_DIR		= ./src
OBJ_DIR		= ./obj
LIB_DIR		= ./lib
BIN_DIR		= ./bin
EXAM_DIR	= ./exam
BIN			= producer consumer

INC		= -I$(INC_DIR) -I$(INC_DIR)/zookeeper/ -I$(INC_DIR)/librdkafka/
# LIB		= -L$(LIB_DIR) -lrdkafka -lpthread -lm
LIB		= -L$(LIB_DIR) -lrdkafka  -rdynamic -lpthread -lcrypto -ldl -lz -lssl

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

$(BIN_DIR)/producer:$(addprefix $(OBJ_DIR)/, producer.o)
	$(CXX) $(CXXFLAGS) $(CXX_MACRO) -o $@ $^ $(LIB)

$(BIN_DIR)/consumer:$(addprefix $(OBJ_DIR)/, consumer.o)
	$(CXX) $(CXXFLAGS) $(CXX_MACRO) -o $@ $^ $(LIB)

strip:
	strip $(addprefix $(BIN_DIR)/,$(BIN))

clean:
	rm -f $(addprefix $(BIN_DIR)/,$(BIN)) $(OBJ_DIR)/*.o

