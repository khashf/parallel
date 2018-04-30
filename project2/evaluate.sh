#!/bin/bash -e

if [ "$#" -ne 1 ]; then
    echo 'Usage:'
    echo "Must provide 1 arguments as path to output file"
    echo "For example:"
    echo "  ./$0 ./evaluation_data.csv"
    exit 1
fi

OUTFILE=$1

STATIC_COARSE=nbody-static-coarse
STATIC_FINE=nbody-static-fine
DYNAMIC_COARSE=nbody-dynamic-coarse
DYNAMIC_FINE=nbody-dynamic-fine

echo 'Outputing evaluation on [Mega Bodies Compared Per Seconds] as the [Number of Threads] changes with 4 different parallization methods'
echo 'Number of Threads, Static Coarse, Static Fine, Dynamic Coarse, Dynamic Fine' > $OUTFILE
i=1
while [ $i -lt 128 ]; do
    echo -n "$i,"
    echo -n $( ./$STATIC_COARSE $i | tail -n1 | awk '{print $7}' )','
    echo -n $( ./$STATIC_FINE $i | tail -n1 | awk '{print $7}' )','
    echo -n $( ./$DYNAMIC_COARSE $i | tail -n1 | awk '{print $7}' )','
    echo -n $( ./$DYNAMIC_FINE $i | tail -n1 | awk '{print $7}' )','
    echo
    i=$(( $i * 2 ))
done >> "$OUTFILE"

echo 'Evaluation Completed. Result is in '$OUTFILE
