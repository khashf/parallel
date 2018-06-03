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
OUT_MULT="$DATA_PATH/$1-mult.csv"
OUT_MULT_ADD="$DATA_PATH/$1-multadd.csv"
OUT_MULT_REDUCE="$DATA_PATH/$1-reduce.csv"

BIN_MULT=multiply
BIN_MULT_ADD=multiply-add
BIN_MULT_REDUCE=multiply-reduce

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
LOCAL_SIZE_CONST=32

rm -f $OUT_MULT && touch $OUT_MULT
rm -f $OUT_MULT_ADD && touch $OUT_MULT_ADD
rm -f $OUT_MULT_REDUCE && touch $OUT_MULT_REDUCE

i_MIN=$LOCAL_SIZE_1
i_MAX=$LOCAL_SIZE_11
i=$i_MIN

echo "LOCAL_SIZE,$GLOBAL_SIZE_1,$GLOBAL_SIZE_2,$GLOBAL_SIZE_3,$GLOBAL_SIZE_4,$GLOBAL_SIZE_5,$GLOBAL_SIZE_6,$GLOBAL_SIZE_7" | tee $OUT_MULT $OUT_MULT_ADD $OUT_MULT_REDUCE
while [ $i -le $i_MAX ]; do
        echo -n "$i," | tee -a $OUT_MULT $OUT_MULT_ADD
        # Program arguments: <program name> <global size> <local size>

        $BIN_MULT_ADD $GLOBAL_SIZE_1 $i | tail -n1 | awk '{print $3}' | tr '\n' ',' | tee -a $OUT_MULT
        $BIN_MULT $GLOBAL_SIZE_1 $i | tail -n1 | awk '{print $3}' | tr '\n' ',' | tee -a $OUT_MULT_ADD
        
        $BIN_MULT_ADD $GLOBAL_SIZE_2 $i | tail -n1 | awk '{print $3}' | tr '\n' ',' | tee -a $OUT_MULT
        $BIN_MULT $GLOBAL_SIZE_2 $i | tail -n1 | awk '{print $3}' | tr '\n' ',' | tee -a $OUT_MULT_ADD
        
        $BIN_MULT_ADD $GLOBAL_SIZE_3 $i | tail -n1 | awk '{print $3}' | tr '\n' ',' | tee -a $OUT_MULT
        $BIN_MULT $GLOBAL_SIZE_3 $i | tail -n1 | awk '{print $3}' | tr '\n' ',' | tee -a $OUT_MULT_ADD
        
        $BIN_MULT_ADD $GLOBAL_SIZE_4 $i | tail -n1 | awk '{print $3}' | tr '\n' ',' | tee -a $OUT_MULT
        $BIN_MULT $GLOBAL_SIZE_4 $i | tail -n1 | awk '{print $3}' | tr '\n' ',' | tee -a $OUT_MULT_ADD
        
        $BIN_MULT_ADD $GLOBAL_SIZE_5 $i | tail -n1 | awk '{print $3}' | tr '\n' ',' | tee -a $OUT_MULT
        $BIN_MULT $GLOBAL_SIZE_5 $i | tail -n1 | awk '{print $3}' | tr '\n' ',' | tee -a $OUT_MULT_ADD
        
        $BIN_MULT_ADD $GLOBAL_SIZE_6 $i | tail -n1 | awk '{print $3}' | tr '\n' ',' | tee -a $OUT_MULT
        $BIN_MULT $GLOBAL_SIZE_6 $i | tail -n1 | awk '{print $3}' | tr '\n' ',' | tee -a $OUT_MULT_ADD
        
        # Last column, `tr` is different
        $BIN_MULT_ADD $GLOBAL_SIZE_7 $i | tail -n1 | awk '{print $3}' | tr -d '\n' | tee -a $OUT_MULT
        $BIN_MULT $GLOBAL_SIZE_7 $i | tail -n1 | awk '{print $3}' | tr -d '\n' | tee -a $OUT_MULT_ADD

        echo | tee -a $OUT_MULT $OUT_MULT_ADD
        i=$(( $i * 2 ))
done

$BIN_MULT_REDUCE $GLOBAL_SIZE_1 $LOCAL_SIZE_CONST | tail -n1 | awk '{print $3}' | tr '\n' ',' | tee -a $OUT_MULT_REDUCE
$BIN_MULT_REDUCE $GLOBAL_SIZE_2 $LOCAL_SIZE_CONST | tail -n1 | awk '{print $3}' | tr '\n' ',' | tee -a $OUT_MULT_REDUCE
$BIN_MULT_REDUCE $GLOBAL_SIZE_3 $LOCAL_SIZE_CONST | tail -n1 | awk '{print $3}' | tr '\n' ',' | tee -a $OUT_MULT_REDUCE
$BIN_MULT_REDUCE $GLOBAL_SIZE_4 $LOCAL_SIZE_CONST | tail -n1 | awk '{print $3}' | tr '\n' ',' | tee -a $OUT_MULT_REDUCE
$BIN_MULT_REDUCE $GLOBAL_SIZE_5 $LOCAL_SIZE_CONST | tail -n1 | awk '{print $3}' | tr '\n' ',' | tee -a $OUT_MULT_REDUCE
$BIN_MULT_REDUCE $GLOBAL_SIZE_6 $LOCAL_SIZE_CONST | tail -n1 | awk '{print $3}' | tr '\n' ',' | tee -a $OUT_MULT_REDUCE
$BIN_MULT_REDUCE $GLOBAL_SIZE_7 $LOCAL_SIZE_CONST | tail -n1 | awk '{print $3}' | tr -d '\n' | tee -a $OUT_MULT_REDUCE

echo "Benchmarking Completed. Results are in $OUT_MULT and $OUT_MULT_ADD"

