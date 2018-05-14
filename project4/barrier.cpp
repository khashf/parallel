#include <stdio.h>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <omp.h>
#include <time.h>

using std::cerr;
using std::endl;
using std::cout;

const int NUM_THREADS = 3;
int main() {
    omp_set_num_threads(NUM_THREADS);
    #pragma omp parallel sections 
    {
        #pragma omp section 
        {
            std::cout << "Section [Grain] Thread [" << omp_get_thread_num() << "] - computing grain" << endl;
            //float tmpGrainHeight = ComputeGrain();
            #pragma omp barrier
            std::cout << "Section [Grain] Thread [" << omp_get_thread_num() << "] - updating grain" << endl;
            //UpdateGrain(tmpGrainHeight);
            #pragma omp barrier
            std::cout << "Section [Grain] Thread [" << omp_get_thread_num() << "] - ends" << endl;
            #pragma omp barrier
        }
        #pragma omp section 
        {
            std::cout << "Section [Deers] Thread [" << omp_get_thread_num() << "] - computing deers" << endl;
            //float tmpDeers = ComputeDeers();
            #pragma omp barrier
            std::cout << "Section [Deers] Thread [" << omp_get_thread_num() << "] - updating deers" << endl;
            //UpdateDeer(tmpDeers);
            #pragma omp barrier
            std::cout << "Section [Deers] Thread [" << omp_get_thread_num() << "] - ends" << endl;
            #pragma omp barrier
        }
        #pragma omp section 
        {
            #pragma omp barrier
            std::cout << "Section [Watcher] Thread [" << omp_get_thread_num() << "] - printing and updating states" << endl;
            PrintState();
            UpdateTime();
            UpdateFactors();
            #pragma omp barrier
            std::cout << "Section [Watcher] Thread [" << omp_get_thread_num() << "] - ends" << endl;
            #pragma omp barrier
        }
    } // omp parallel sections
    return 0;
}