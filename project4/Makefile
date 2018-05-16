BIN_PATH=./bin
TARGET=${BIN_PATH}/grainville
CC=g++
CXXFLAGS= -std=c++11 -Wall -Wextra
CFLAGS=-g -O0
LIB=-lm -fopenmp
FLAGS=${CFLAGS} ${CXXFLAGS} ${LIB}
SRC=grainville.cpp


all: compile run

compile:
	${CC} -o ${TARGET} ${SRC} ${FLAGS}

run:
	${TARGET}

clean:
	rm -f ${TARGET}

.PHONY: clean, all
