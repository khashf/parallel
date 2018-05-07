BIN_PATH=./bin
TARGET_FIX_1=${BIN_PATH}/cache-false-sharing-fix-1
TARGET_FIX_2=${BIN_PATH}/cache-false-sharing-fix-2

CC=g++
CXXFLAGS=-std=c++11 -Wall -Wextra
CFLAGS=-g
LIB=-lm -fopenmp
FLAGS=${CFLAGs} ${CXXFLAGS} ${LIB}
SRC=cache-false-sharing.cpp

all: fix1 fix2
fix1:
	g++ -DFIXMODE=1 -DNUM=1 -o ${TARGET_FIX_1}-1 ${SRC} ${FLAGS}
	g++ -DFIXMODE=1 -DNUM=2 -o ${TARGET_FIX_1}-2 ${SRC} ${FLAGS}
	g++ -DFIXMODE=1 -DNUM=3 -o ${TARGET_FIX_1}-3 ${SRC} ${FLAGS}
	g++ -DFIXMODE=1 -DNUM=4 -o ${TARGET_FIX_1}-4 ${SRC} ${FLAGS}
	g++ -DFIXMODE=1 -DNUM=5 -o ${TARGET_FIX_1}-5 ${SRC} ${FLAGS}
	g++ -DFIXMODE=1 -DNUM=6 -o ${TARGET_FIX_1}-6 ${SRC} ${FLAGS}
	g++ -DFIXMODE=1 -DNUM=7 -o ${TARGET_FIX_1}-7 ${SRC} ${FLAGS}
	g++ -DFIXMODE=1 -DNUM=8 -o ${TARGET_FIX_1}-8 ${SRC} ${FLAGS}
	g++ -DFIXMODE=1 -DNUM=9 -o ${TARGET_FIX_1}-9 ${SRC} ${FLAGS}
	g++ -DFIXMODE=1 -DNUM=10 -o ${TARGET_FIX_1}-10 ${SRC} ${FLAGS}
	g++ -DFIXMODE=1 -DNUM=11 -o ${TARGET_FIX_1}-11 ${SRC} ${FLAGS}
	g++ -DFIXMODE=1 -DNUM=12 -o ${TARGET_FIX_1}-12 ${SRC} ${FLAGS}
	g++ -DFIXMODE=1 -DNUM=13 -o ${TARGET_FIX_1}-13 ${SRC} ${FLAGS}
	g++ -DFIXMODE=1 -DNUM=14 -o ${TARGET_FIX_1}-14 ${SRC} ${FLAGS}
	g++ -DFIXMODE=1 -DNUM=15 -o ${TARGET_FIX_1}-15 ${SRC} ${FLAGS}
	g++ -DFIXMODE=1 -DNUM=16 -o ${TARGET_FIX_1}-16 ${SRC} ${FLAGS}
	g++ -DFIXMODE=1 -DNUM=17 -o ${TARGET_FIX_1}-17 ${SRC} ${FLAGS}
	g++ -DFIXMODE=1 -DNUM=18 -o ${TARGET_FIX_1}-18 ${SRC} ${FLAGS}
fix2:
	g++ -DFIXMODE=2 -o ${TARGET_FIX_2} ${SRC} ${FLAGS}

clean_fix1:
	rm -f ${TARGET_FIX_1}-*
clean_fix2:
	rm -f ${TARGET_FIX_2}
clean: clean_fix1 clean_fix2

.PHONY: clean, all
