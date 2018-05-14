TARGET=grainville
CC=g++
CXXFLAGS= -std=c++11 -Wall -Wextra
CFLAGS=-g -O0
LIB=-lm -fopenmp
FLAGS= ${CFLAGS} ${CXXFLAGS} ${LIB}
SRC=grainville.cpp


all:
	g++ -o ${TARGET} ${SRC} ${FLAGS}

clean:
	rm -f ${TARGET}

.PHONY: clean, all
