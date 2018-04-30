TARGET_STATIC_COARSE=nbody-static-coarse
TARGET_STATIC_FINE=nbody-static-fine
TARGET_DYNAMIC_COARSE=nbody-dynamic-coarse
TARGET_DYNAMIC_FINE=nbody-dynamic-fine

CC=g++
CXXFLAGS=-std=c++11 -Wall -Wextra
CFLAGS=-g -O3
LIB=-lm -fopenmp
FLAGS=${CFLAGs} ${CXXFLAGS} ${LIB}
SRC=nbody.cpp

all: static_coarse static_fine dynamic_coarse dynamic_fine

static_coarse:
	g++ -DSMODE=0 -DPMODE=0 -o ${TARGET_STATIC_COARSE} ${SRC} ${FLAGS}
static_fine:
	g++ -DSMODE=0 -DPMODE=1 -o ${TARGET_STATIC_FINE} ${SRC} ${FLAGS}
dynamic_coarse:
	g++ -DSMODE=1 -DPMODE=0 -o ${TARGET_DYNAMIC_COARSE} ${SRC} ${FLAGS}
dynamic_fine:
	g++ -DSMODE=1 -DPMODE=1 -o ${TARGET_DYNAMIC_FINE} ${SRC} ${FLAGS}

clean_static_coarse:
	rm -f ${TARGET_STATIC_COARSE}
clean_static_fine:
	rm -f ${TARGET_STATIC_FINE}
clean_dynamic_coarse:
	rm -f ${TARGET_DYNAMIC_COARSE}
clean_dynamic_fine:
	rm -f ${TARGET_DYNAMIC_FINE}
clean: clean_static_coarse clean_static_fine clean_dynamic_coarse clean_dynamic_fine

.PHONY: clean, all
