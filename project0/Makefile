TARGET=proj0
CC=g++
CXXFLAGS=-std=c++11 -Wall -Wextra
CFLAGS=-g -O3
FLAGS=${CFLAGs} ${CXXFLAGS} ${LIB}
SRC=proj0.cpp
LIB=-lm -fopenmp

all:
	g++ -o ${TARGET} ${SRC} ${FLAGS}

clean:
	rm -f ${TARGET}

.PHONY: clean, all
