BIN_PATH=.
TARGET=$(BIN_PATH)/first
CC=g++
CXXFLAGS= -std=c++11
CFLAGS=-g 
LIB=-lm -fopenmp -lOpenCL
FLAGS=$(CFLAGS) $(CXXFLAGS) $(LIB)
SRC_PATH=.
SRC=$(SRC_PATH)/first.cpp

all: first

first:
	$(CC) -o $(TARGET) $(SRC) $(FLAGS)

clean:
	rm -f $(TARGET)

.PHONY: clean, all
