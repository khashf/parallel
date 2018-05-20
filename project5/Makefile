BIN_PATH=./bin
SRC_PATH=./src

TARGET_VEC=$(BIN_PATH)/simd-vec
TARGET_NOVEC=$(BIN_PATH)/simd-novec
CC=icpc
CXXFLAGS= -std=c++11 -Wall -align
CFLAGS= -qopt-report=3 -qopt-report-phase=vec
LIB=-lm -qopenmp
FLAGS=$(CFLAGS) $(CXXFLAGS) $(LIB)
SRC=$(SRC_PATH)/simd.cpp


all: compile

compile: compile-vec compile-novec
compile-vec:
	$(CC) -DSIMD -o $(TARGET_VEC) $(SRC) $(FLAGS)
compile-novec:
	$(CC) -o $(TARGET_NOVEC) $(SRC) $(FLAGS) -no-vec

clean: clean-vec clean-novec
clean-vec:
	rm -f $(TARGET_VEC)
clean-novec:
	rm -f $(TARGET_NOVEC)

.PHONY: clean, all
