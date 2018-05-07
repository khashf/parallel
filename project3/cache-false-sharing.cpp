#include <stdio.h>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <omp.h>

using std::cerr;
using std::endl;
using std::cout;

enum ErrorType {
    NO_ERROR,
    NO_NUMT,
    INVALID_FIXMODE
};

int NUMT;
const int NUM_ARGS = 2;

#ifndef NUM
    #if FIXMODE == 1
        #define NUM 0 // dummy define
        #define ERROR 1
        int gErrorType = NO_NUMT;
    #endif
    // if FIXMODE == 2, we don't need NUM
    #define NUM 0 // define a dummy value anyway
#endif

#if FIXMODE == 1
    struct s {
        float value;
        int pad[NUM];
    } Array[4];
#elif FIXMODE == 2
    struct s {
        float value;
    } Array[4];
#else
    #define ERROR 1
    int gErrorType = INVALID_FIXMODE;
    // default struct
    struct s {
        float value;
        int pad[NUM];
    } Array[4];
#endif


int main(int argc, char** argv) {
    #ifdef ERROR
        if (gErrorType == 1)
            fprintf(stderr, "Number of threads is not defined at compilation time\n");
        else if (gErrorType == INVALID_FIXMODE)
            fprintf(stderr, "Invalid Fix Mode\n");
        else
            fprintf(stderr, "Unknown Error\n");
        return 1;
    #endif
    #ifndef _OPENMP
        fprintf(stderr, "OpenMP is not available\n");
        return 1;
    #endif
    if (argc != NUM_ARGS) {
        cerr << "Number of arguments must be " << NUM_ARGS-1 << endl;
        cerr << "Usage" << endl;
        cerr << argv[0] << " <number of threads>" << endl;
        return 1;
    }

    NUMT = atoi(argv[1]);
    omp_set_num_threads(NUMT);
    int numProcessors = omp_get_num_procs();
    cout << "Using " << NUMT << " threads" << endl;
    cout << "Using " << numProcessors << " processors" << endl;

    const int numTries = 4;
    const long numSteps = 1000000000;
    double time0 = omp_get_wtime();
    #if FIXMODE == 1
        #pragma omp parallel for
        for (int i = 0; i < numTries; i++) {
            for (int j = 0; j < numSteps; j++) {
                Array[i].value = Array[i].value + 2.;
            }
        }
    #elif FIXMODE == 2
        #pragma omp parallel for
        for (int i = 0; i < numTries; i++) {
            float tmp = Array[i].value;
            for (int j = 0; j < numSteps; j++) {
                tmp = tmp + 2.;
            }
            Array[i].value = tmp;
        }
    #endif
    
    double time1 = omp_get_wtime();
    double execTime = (double)(time1 - time0) * pow(10.0, 9.0); // in nano seconds
    double MegaReadWritePerSec = ((double)(numTries * numSteps) / (time1 - time0) / 1000000.);

    cout << "Execution Time = " << execTime << endl;
    cout << "Mega Read Write Compared Per Seconds = " << MegaReadWritePerSec << endl;

}


