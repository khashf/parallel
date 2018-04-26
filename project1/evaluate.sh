#!/bin/bash -e

CHOICE=
OUTFILE=
NUM_THREADS=
NUM_SUBDIVISIONS=

read -p 'Path to output file: ' OUTFILE
read -p "Observe performance by:\n[1] Number of Threads\n[2] Number of subdivisions\nYour choice: " CHOICE

if [ "$CHOICE" == "1" ]; then
    echo 'Outputing observation data on [Performance] as the [Number of Threads] changes, keeping fixed Number of Subdivision=10'
    echo 'Number of Threads, Execution Time' > $OUTFILE
    NUM_SUBDIVISIONS=10
    for i in {1..20..1}; do
        echo -n "$i,"
        ./project1 $i $NUM_SUBDIVISIONS | tail -n2 | head -n 1 | awk '{print $5}'
    done >> "$OUTFILE"
else
    echo 'Outputing observation data on [Performance] as the [Number of Subdivisions] changes, keeping fixed Number of Threads=10'
    echo 'Number of Subdivisions, Execution Time' > $OUTFILE
    NUM_THREADS=10
    i=10
    while [ $i -lt 10000 ]; do
        echo -n "$i,"
        ./project1 $NUM_THREADS $i | tail -n2 | head -n 1 | awk '{print $5}' 
        if [ $i -lt 30 ]; then
            i=$(( $i + 2 ))
        elif [ $i -lt 100 ]; then
            i=$(( $i + 10 ))
        elif [ $i -lt 1000 ]; then
            i=$(( $i * 2 ))
        else
            i=$(( $i * 5 ))
        fi
    done >> "$OUTFILE"
fi 
echo 'Evaluation Completed. Result is in '$OUTFILE
