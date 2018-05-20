#include <stdio.h>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <omp.h>

using std::cerr;
using std::endl;
using std::cout;

const int g_actual_desired_argc = 2;

double* A = nullptr;
double* C = nullptr;

// float Ranf(float low, float high) {
//     float r = (float)rand(); // 0 - RAND_MAX
//     return (low + r * (high - low) / (float)RAND_MAX);
// }

void InitData(const int& size) {
    //srand (time(NULL));
    unsigned int seed = time(NULL);
    A = new double[size];
    for (int i = 0; i < size; ++i) {
        A[i] = (static_cast<double>(rand_r(&seed))); // 0 - RAND_MAX
    }
    cout << "A[2] = " << A[2] << endl;
    cout << "A[100] = " << A[100] << endl;
    C = new double[size];
}

int main(int argc, char** argv) {
    #ifndef _OPENMP
        fprintf(stderr, "OpenMP is not supported here -- sorry.\n");
        return 1;
    #endif
    if (argc != g_actual_desired_argc+1) {
        fprintf(stderr, "Number of arguments must be %d\n", g_actual_desired_argc);
        fprintf(stderr, "Usage:\n%s <number of threads> <array size>\n", argv[0]);
        return 1;
    }
    int num_threads = atoi(argv[1]);
    int array_size = atoi(argv[2]);
    omp_set_num_threads(num_threads);
    cout << "Using " << num_threads << " threads" << endl;
    cout << "Using " << array_size << " elements for array size" << endl;

    InitData(array_size);
    double time0 = omp_get_wtime();
    #ifdef SIMD
        #pragma omp parallel for simd default(none) shared(array_size, A, C)
    #else
        #pragma omp parallel for default(none) shared(array_size, A, C)
    #endif
    //#pragma omp parallel for simd default(none) shared(array_size, A, C)
    for(int i = 0; i < array_size; i++) {
        C[i] = sqrt(A[i]);
    }
    double time1 = omp_get_wtime();
    double megaMults = (double)array_size / (time1 - time0) / pow(10.0, 6.0); // in mega mults
    double exec_time = (double)(time1 - time0) * pow(10.0, 9.0); // in nano seconds
    fprintf(stdout, "Performance = %10.2lf MegaSquareRoot Per Second\n", megaMults);
    fprintf(stdout, "Execution Time = %10.2lf nano seconds\n", exec_time);
    return 0;
}