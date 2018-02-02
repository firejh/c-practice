
.PHONY: all

CC      = g++
CC_FLAG = -Wall -lpthread -lrt -rdynamic -O2
INC_DIR = inc
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

INC=-I./$(INC_DIR)
LIB=

all:dir clean $(BIN_DIR)/test

dir:
	@-mkdir -p $(OBJ_DIR)
	@-mkdir -p $(BIN_DIR)

$(OBJ_DIR)/%.o:$(SRC_DIR)/%.cc
	$(CC) $(CC_FLAG) $(INC) -c $< -o $@

$(BIN_DIR)/test:$(OBJ_DIR)/test.o
	$(CC) $(CC_FLAG) $^ -o $@ $(LIB)

clean:
	rm -rf $(OBJ_DIR)/*.o $(BIN_DIR)/test


