#!/bin/bash -e

if [ "$#" -ne 1 ]; then
    echo 'Usage:'
    echo "Must provide 1 arguments as path to output file"
    echo "For example:"
    echo "  ./$0 ./evaluation_data.csv"
    exit 1
fi

OUTFILE=$1

BIN_PATH='./bin'
FIX_1=$BIN_PATH/cache-false-sharing-fix-1
FIX_2=$BIN_PATH/cache-false-sharing-fix-2

echo 'Running Fix #2'
FIX_2_RESULT_1=$( ./$FIX_2 1 | tail -n1 | awk '{print $10}' )
FIX_2_RESULT_2=$( ./$FIX_2 2 | tail -n1 | awk '{print $10}' )
FIX_2_RESULT_4=$( ./$FIX_2 4 | tail -n1 | awk '{print $10}' )

echo "Running Fix #1 and outputing both fixes' results to $OUTFILE"
echo -n 'Padding Size,' > $OUTFILE
echo -n 'Fix #1 with 1 thread,Fix #1 with 2 threads,Fix #1 with 4 threads,' >> $OUTFILE
echo -n 'Fix #2 with 1 threads,Fix #2 with 2 threads,Fix #2 with 4 threads' >> $OUTFILE
echo >> $OUTFILE

i=0
while [ $i -lt 19 ]; do
    echo -n "$i,"
    echo -n $( ./${FIX_1}-${i} 1 | tail -n1 | awk '{print $10}' )','
    echo -n $( ./${FIX_1}-${i} 2 | tail -n1 | awk '{print $10}' )','
    echo -n $( ./${FIX_1}-${i} 4 | tail -n1 | awk '{print $10}' )','
    echo -n $FIX_2_RESULT_1','
    echo -n $FIX_2_RESULT_2','
    echo -n $FIX_2_RESULT_4
    echo
    i=$(( $i + 1 ))
done >> "$OUTFILE"

echo 'Evaluation Completed. Result is in '$OUTFILE