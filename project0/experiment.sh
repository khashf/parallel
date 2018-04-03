#!/bin/bash -e

TIME_1_THRD=
TIME_4_THRD=

echo 'Running with 1 thread'
./proj0 1 > ./tmp1
#TIME_1_THRD= $( cat ./tmp1 | tail -n 1 | cut -d' ' -f4 )
echo "Running time with 1 thread = $TIME_1_THRD"

rm -f ./tmp1

