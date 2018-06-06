BIN_PATH=.
TARGET_MUL=$(BIN_PATH)/multiply
TARGET_MUL_ADD=$(BIN_PATH)/multiply-add
TARGET_MUL_REDUCE=$(BIN_PATH)/multiply-reduce
CC=g++
CXXFLAGS= -std=c++11
CFLAGS=-g 
LIB=-lm -fopenmp -lOpenCL
FLAGS=$(CFLAGS) $(CXXFLAGS) $(LIB)
SRC_PATH=.
SRC=$(SRC_PATH)/multiply.cpp

all: multiply multiply_add multiply_reduce

multiply: $(SRC)
	$(CC) -DMODE=1 -o $(TARGET_MUL) $(SRC) $(FLAGS)

multiply_add: $(SRC)
	$(CC) -DMODE=2 -o $(TARGET_MUL_ADD) $(SRC) $(FLAGS)

multiply_reduce: $(SRC)
	$(CC) -DMODE=3 -o $(TARGET_MUL_REDUCE) $(SRC) $(FLAGS)

clean:
	rm -f $(TARGET_MUL) $(TARGET_MUL_ADD) $(TARGET_MUL_REDUCE)

.PHONY: clean, all
