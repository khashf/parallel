#!/bin/bash

usage() {
    echo 'Usage:'
    echo "$0 <filename>"
    echo 'You must provide 1 arguments as the name output file.'
    echo 'There will be 2 csv files generated in root directory of project as `mul-reduce` and `mul-noreduce`'
}

if [ "$#" -ne 1 ]; then
    usage
    exit 1
fi

DATA_PATH='.'
OUT_REDUCE="$DATA_PATH/$1-reduce.csv"
OUT_NOREDUCE="$DATA_PATH/$1-noreduce.csv"
BIN_REDUCE=mul-reduce
BIN_NOREDUCE=mul-noreduce

# at least from 1K to 8M
GLOBAL_SIZE_1=1000
GLOBAL_SIZE_2=10000
GLOBAL_SIZE_3=100000
GLOBAL_SIZE_4=1000000
GLOBAL_SIZE_5=16000000
GLOBAL_SIZE_6=32000000
GLOBAL_SIZE_7=64000000

# from 1 to 1024
LOCAL_SIZE_1=1
LOCAL_SIZE_2=2
LOCAL_SIZE_3=4
LOCAL_SIZE_4=8
LOCAL_SIZE_5=16
LOCAL_SIZE_6=32
LOCAL_SIZE_7=64
LOCAL_SIZE_8=128
LOCAL_SIZE_9=256
LOCAL_SIZE_10=512
LOCAL_SIZE_11=1024

touch $OUT_REDUCE
touch $OUT_NOREDUCE

i_MIN=$LOCAL_SIZE_1
i_MAX=$LOCAL_SIZE_11
i=$i_MIN

echo "LOCAL_SIZE,$GLOBAL_SIZE_1,$GLOBAL_SIZE_2,$GLOBAL_SIZE_3,$GLOBAL_SIZE_4,$GLOBAL_SIZE_5,$GLOBAL_SIZE_6,$GLOBAL_SIZE_7" | tee $OUT_NOREDUCE $OUT_REDUCE
while [ $i -le $i_MAX ]; do
        echo -n "$i," | tee -a $OUT_NOREDUCE $OUT_REDUCE
        # Program arguments: <program name> <global size> <local size>

        $BIN_NOREDUCE $GLOBAL_SIZE_1 $i | tail -n2 | head -n 1 | awk '{print $6}' | tr '\n' ',' | tee -a $OUT_NOREDUCE
        $BIN_REDUCE $GLOBAL_SIZE_1 $i | tail -n2 | head -n 1 | awk '{print $6}' | tr '\n' ',' | tee -a $OUT_REDUCE
        
        $BIN_NOREDUCE $GLOBAL_SIZE_2 $i | tail -n2 | head -n 1 | awk '{print $6}' | tr '\n' ',' | tee -a $OUT_NOREDUCE
        $BIN_REDUCE $GLOBAL_SIZE_2 $i | tail -n2 | head -n 1 | awk '{print $6}' | tr '\n' ',' | tee -a $OUT_REDUCE
        
        $BIN_NOREDUCE $GLOBAL_SIZE_3 $i | tail -n2 | head -n 1 | awk '{print $6}' | tr '\n' ',' | tee -a $OUT_NOREDUCE
        $BIN_REDUCE $GLOBAL_SIZE_3 $i | tail -n2 | head -n 1 | awk '{print $6}' | tr '\n' ',' | tee -a $OUT_REDUCE
        
        $BIN_NOREDUCE $GLOBAL_SIZE_4 $i | tail -n2 | head -n 1 | awk '{print $6}' | tr '\n' ',' | tee -a $OUT_NOREDUCE
        $BIN_REDUCE $GLOBAL_SIZE_4 $i | tail -n2 | head -n 1 | awk '{print $6}' | tr '\n' ',' | tee -a $OUT_REDUCE
        
        $BIN_NOREDUCE $GLOBAL_SIZE_5 $i | tail -n2 | head -n 1 | awk '{print $6}' | tr '\n' ',' | tee -a $OUT_NOREDUCE
        $BIN_REDUCE $GLOBAL_SIZE_5 $i | tail -n2 | head -n 1 | awk '{print $6}' | tr '\n' ',' | tee -a $OUT_REDUCE
        
        $BIN_NOREDUCE $GLOBAL_SIZE_6 $i | tail -n2 | head -n 1 | awk '{print $6}' | tr '\n' ',' | tee -a $OUT_NOREDUCE
        $BIN_REDUCE $GLOBAL_SIZE_6 $i | tail -n2 | head -n 1 | awk '{print $6}' | tr '\n' ',' | tee -a $OUT_REDUCE
        
        # Last column, `tr` is different
        $BIN_NOREDUCE $GLOBAL_SIZE_7 $i | tail -n2 | head -n 1 | awk '{print $6}' | tr -d '\n' | tee -a $OUT_NOREDUCE
        $BIN_REDUCE $GLOBAL_SIZE_7 $i | tail -n2 | head -n 1 | awk '{print $6}' | tr -d '\n' | tee -a $OUT_REDUCE

        echo | tee -a $OUT_NOREDUCE $OUT_REDUCE
        i=$(( $i * 2 ))
done

echo "Benchmarking Completed. Results are in $OUT_NOREDUCE and $OUT_REDUCE"

