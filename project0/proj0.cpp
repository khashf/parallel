#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// #define NUMT             4
#define ARRAYSIZE       1000
#define NUMTRIES        10  

float A[ARRAYSIZE];
float B[ARRAYSIZE];
float C[ARRAYSIZE];

void PrintArray(float* a) {
    for (int i = 0; i < ARRAYSIZE; ++i) {
        fprintf(stdout, " %8.2f ", a[i]);
        fprintf(stdout, "\n");
    }
}

void PrintAllArrays() {
    PrintArray(A);
    PrintArray(B);
    PrintArray(C);
}

int main(int argc, char** argv) {
#ifndef _OPENMP
    fprintf(stderr, "OpenMP is not supported here -- sorry.\n");
    return 1;
#endif
    if (argc != 2) {
        fprintf(stderr, "Number of arguments must be 1\n");
        fprintf(stderr, "Usage:\n%s <number of threads>\n", argv[0]);
        return 1;
    }

    double max_mega_mults = 0.;
    double sum_mega_mults = 0.;
    double sum_exec_time = 0.;
    int num_threads = atoi(argv[1]);

    omp_set_num_threads(num_threads);
    fprintf(stdout, "Using %d threads\n", num_threads);

    for(int t = 0; t < NUMTRIES; t++) {
        //printf("Trial %d\n", t+1);
        double time0 = omp_get_wtime(); // in seconds
        //printf("time0 = %8.2lf seconds\n", time0);

#pragma omp parallel for
        for(int i = 0; i < ARRAYSIZE; i++) {
            C[i] = A[i] * B[i];
        }

        double time1 = omp_get_wtime(  ); // in seconds
        //printf("time1 = %8.2lf seconds\n", time1);
        double megaMults = (double)ARRAYSIZE / (time1 - time0) / pow(10.0, 6.0); // in mega mults
        double exec_time = (double)(time1 - time0) * pow(10.0, 9.0); // in nano seconds
        //printf("exec_time = %8.2lf nano seconds\n", exec_time);

        sum_mega_mults += megaMults;
        sum_exec_time += exec_time;
        if(megaMults > max_mega_mults)
            max_mega_mults = megaMults;
    }
    double avg_mega_mults = sum_mega_mults/(double)NUMTRIES;
    double avg_exec_time = sum_exec_time/(double)NUMTRIES;
    fprintf(stdout, "   Peak Performance = %8.2lf MegaMults/Sec\n", max_mega_mults);
    fprintf(stdout, "Average Performance = %8.2lf MegaMults/Sec\n", avg_mega_mults);
    fprintf(stdout, "Average Execution Time = %8.2lf nano seconds\n", avg_exec_time);
}
